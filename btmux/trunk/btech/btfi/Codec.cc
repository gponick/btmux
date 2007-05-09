/*
 * Fast Infoset Encoder/Decoder classes.
 *
 * The Decoder implementation in particular is a bit convoluted.  It has been
 * designed to allow incremental parsing, with I/O performed in the outer loop.
 * This allows, for example, simultaneously parsing several streams without
 * multithreading or other such tricks.
 */

#include "autoconf.h"

#include <cstring>
#include <cassert>

#include "stream.h"

#include "Name.hh"
#include "Value.hh"
#include "Vocabulary.hh"

#include "Codec.hh"

namespace BTech {
namespace FI {


/*
 * General Encoder routines.
 */

Encoder::Encoder()
: vocabulary (0)
{
	clear();
}

// Reset Encoder state.  This will clear the associated vocabulary and internal
// decoder state, but not the associated output stream.
void
Encoder::clear()
{
	if (vocabulary) {
		vocabulary->clear();
	}

	num_unwritten_bits = 0;
	partial_octet = 0;
}


/*
 * General Decoder routines.
 */

Decoder::Decoder()
: vocabulary (0)
{
	clear();
}

// Reset Decoder state.  This will clear the associated vocabulary and internal
// decoder state, but not the associated input stream.
void
Decoder::clear()
{
	if (vocabulary) {
		vocabulary->clear();
	}

	next_child_type = NEXT_UNKNOWN;

	num_read_bits = 0;

	super_step = 0;
	sub_step = 0;
	sub_sub_step = 0;
}


/*
 * Byte-level I/O.
 */
FI_Octet *
Encoder::getWriteBuffer(FI_Length length)
{
	FI_Octet *w_buf = fi_get_stream_write_buffer(stream, length);
	if (!w_buf) {
		throw IOException ();
	}

	return w_buf;
}

const FI_Octet *
Decoder::getReadBuffer(FI_Length length)
{
	const FI_Octet *r_buf;

	if (fi_try_read_stream(stream, &r_buf, length, length) < length) {
		return 0;
	}

	return r_buf;
}


/*
 * Bit-level I/O.
 */

void
Encoder::writeBits(unsigned int num_bits, FI_Octet octet_mask)
{
	assert(num_bits <= 8 - num_unwritten_bits);

	partial_octet |= octet_mask;

	if (num_bits < 8 - num_unwritten_bits) {
		num_unwritten_bits += num_bits;
	} else {
		num_unwritten_bits = 0;

		// Write out accumulated bits.
		FI_Octet *w_buf = fi_get_stream_write_buffer(stream, 1);
		if (!w_buf) {
			throw IOException ();
		}

		w_buf[0] = partial_octet;
		partial_octet = 0;
	}
}

bool
Decoder::readBits(unsigned int num_bits)
{
	assert(num_bits <= 8 - num_read_bits);

	if (num_read_bits == 0) {
		// Load next octet.
		const FI_Octet *r_buf;

		if (num_bits > 0) {
			// Read.
			if (fi_try_read_stream(stream, &r_buf, 1, 1) < 1) {
				return false;
			}
		} else {
			// Just peek.
			if (fi_try_read_stream(stream, &r_buf, 0, 1) < 1) {
				return false;
			}
		}

		partial_octet = r_buf[0];
	}

	if (num_bits < 8 - num_read_bits) {
		num_read_bits += num_bits;
	} else {
		num_read_bits = 0;
	}

	return true;
}


/*
 * High-level encoding/decoding routines.
 */

// Determine next child of document/element/DTD (possibly none for terminator).
// Consumes identification/terminator bits.  Some child types are not valid in
// all contexts; it is the responsibility of the caller to verify this.
bool
Decoder::readNext()
{
	// Children always start on the first bit of an octet, but the most
	// recent bit may either be the fourth or eight bit (C.2.11.1,
	// C.3.7.1), possibly requiring padding.
	switch (getBitOffset()) {
	case 0:
		// Child may start on this octet.
		break;

	case 4:
		// Terminator or padding.
		// TODO: Is this exhaustive?
		switch (getBits() & FI_BITS(,,,,1,1,1,1)) {
		case FI_BITS(,,,,1,1,1,1):
			// Terminator.
			readBits(4);
			next_child_type = END_CHILD;
			return true;

		case FI_BITS(,,,,0,0,0,0):
			// Padding.
			readBits(4);

			// Child may start on next octet.
			break;

		default:
			// Not a valid Fast Infoset.
			throw IllegalStateException ();
		}
		break;

	default:
		// Shouldn't happen.
		throw IllegalStateException ();
	}

	// Identify the child type.
	if (!readBits(0)) {
		return false;
	}

	if (!(getBits() & FI_BIT_1)) {
		// Start of element (C.2.11.2, C.3.7.2: 0).
		readBits(1);
		next_child_type = START_ELEMENT;
	} else if (!(getBits() & FI_BIT_2)) {
		// Character chunk (C.3.7.5: 10).
		readBits(2);
		next_child_type = CHARACTERS;
	} else if (!(getBits() & FI_BIT_3)) {
		// Unexpanded entity reference or DTD.
		readBits(6);
		switch (getBits() & FI_BITS(/*1*/,/*1*/,/*0*/,1,1,1,,)) {
		case FI_BITS(/*1*/,/*1*/,/*0*/,0,1,0,,):
			// Unexpanded entity reference (C.3.7.4: 110010).
			next_child_type = ENTITY_REFERENCE;
			break;

		case FI_BITS(/*1*/,/*1*/,/*0*/,0,0,1,,):
			// DTD (C.2.11.5: 110001).
			next_child_type = DTD;
			break;

		default:
			// Not a valid Fast Infoset.
			throw IllegalStateException ();
		}
	} else if (!(getBits() & FI_BIT_4)) {
		// Processing instruction or comment.
		readBits(8);
		switch (getBits() &  FI_BITS(/*1*/,/*1*/,/*1*/,/*0*/,1,1,1,1)) {
		case FI_BITS(/*1*/,/*1*/,/*1*/,/*0*/,0,0,0,1):
			// Processing instruction
			// (C.2.11.3, C.3.7.3, C.9.6: 11100001).
			next_child_type = PROCESSING_INSTRUCTION;
			break;

		case FI_BITS(/*1*/,/*1*/,/*1*/,/*0*/,0,0,1,0):
			// Comment (C.2.11.4, C.3.7.6: 11100010).
			next_child_type = COMMENT;
			break;

		default:
			// Not a valid Fast Infoset.
			throw IllegalStateException ();
		}
	} else {
		// Terminator (C.2.12, C.3.8, C.9.7: 1111).
		readBits(4);
		next_child_type = END_CHILD;
	}

	return true;
}

// 12.3: XML declaration pseudo-parser.
namespace {

const char *const xml_decl[] = {
	"<?xml encoding='finf'?>",
	"<?xml encoding='finf' standalone='no'?>",
	"<?xml encoding='finf' standalone='yes'?>",
	"<?xml version='1.0' encoding='finf'?>",
	"<?xml version='1.0' encoding='finf' standalone='no'?>",
	"<?xml version='1.0' encoding='finf' standalone='yes'?>",
	"<?xml version='1.1' encoding='finf'?>",
	"<?xml version='1.1' encoding='finf' standalone='no'?>",
	"<?xml version='1.1' encoding='finf' standalone='yes'?>"
}; // xml_decl[]

} // anonymous namespace

void
Encoder::writeXMLDecl(XMLVersion version, FI_Ternary standalone)
{
	int ii;

	// Select XML version.
	switch (version) {
	case XML_VERSION_NONE:
		ii = 0;
		break;

	case XML_VERSION_1_0:
		ii = 3;
		break;

	case XML_VERSION_1_1:
		ii = 6;
		break;

	default:
		// Not a supported XML version number.
		throw InvalidArgumentException ();
	}

	// Select standalone state.
	switch (standalone) {
	case FI_TERNARY_FALSE:
		ii += 1;
		break;

	case FI_TERNARY_UNKNOWN:
		ii += 0;
		break;

	case FI_TERNARY_TRUE:
		ii += 2;
		break;

	default:
		// Not a ternary state.
		throw AssertionFailureException ();
	}

	// Write declaration.
	const size_t xml_decl_len = strlen(xml_decl[ii]);

	FI_Octet *w_buf = getWriteBuffer(xml_decl_len);

	memcpy(w_buf, xml_decl[ii], xml_decl_len);
}

bool
Decoder::readXMLDecl()
{
	return true;
}

// C.4
// FIXME: This only encodes values as literals, not by index.  We need to at
// least support the empty string value index.
void
Encoder::writeAttribute(const DN_VocabTable::TypedEntryRef& name,
                        const Value& value)
{
	// Write qualified-name using C.17 (C.4.3).
	writeName_bit2(name);

	// Write normalized-value using C.14 (C.4.4).
	writeValue_bit1(value);
}

bool
Decoder::readAttribute(DN_VocabTable::TypedEntryRef& name,
                       Value& value)
{
	switch (super_step) {
	case 0:
		// Read qualified-name using C.17 (C.4.3).
		if (!readName_bit2(vocabulary->attribute_names, saved_name)) {
			return false;
		}

		super_step = 1;
		// FALLTHROUGH

	case 1:
		// Read normalized-value using C.14 (C.4.4).
		if (!readValue_bit1(vocabulary->attribute_values, value)) {
			return false;
		}

		name = saved_name;
		break;

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}

	super_step = 0;
	return true;
}

// C.12
// TODO: Fix this to work with arbitrary namespace attributes, not just the
// default BT_NAMESPACE.
void
Encoder::writeNSAttribute(const NSN_DS_VocabTable::TypedEntryRef& ns_name)
{
	assert(getBitOffset() == 6); // C.12.2 (there's a typo here)

	// No prefix (C.12.3: 0).
	// namespace-name (C.12.4: 1).
	writeBits(2, FI_BITS(,,,,,,0, 1));

	// No prefix to encode using C.13 (C.12.5).
	// namespace-name encoded using C.13 (C.12.6).
	writeIdentifier(ns_name);
}

bool
Decoder::readNSAttribute(NSN_DS_VocabTable::TypedEntryRef& ns_name)
{
	// This is currently hard-coded to check that the namespace attribute
	// has no prefix, and has a namespace-name.
	switch (super_step) {
	case 0:
		assert(getBitOffset() == 6); // C.12.2 (there's a typo here)

		// Examine discriminator bits (C.12.3).
		if (getBits() & FI_BIT_7) {
			// prefix present (C.12.3).
			// FIXME: Implementation restriction.
			throw UnsupportedOperationException ();
		}

		if (!(getBits() & FI_BIT_8)) {
			// namespace-name not present (C.12.4).
			// FIXME: Implementation restriction.
			throw UnsupportedOperationException ();
		}

		readBits(2);

		super_step = 1;
		// FALLTHROUGH

	case 1:
		// Don't parse prefix using C.13 (C.12.5).
		// Parse namespace-name using C.13 (C.12.6).
		// TODO: Being able to cast ns_name down like this, and assign
		// any DS_VocabTable value to it, somewhat breaks type safety.
		// Add some checks in the TypedEntryRef assignment/copy
		// operators to make this type safe, or don't derive from
		// DS_VocabTable.  Or just roll with it. :-)
		if (!readIdentifier(vocabulary->namespace_names, ns_name)) {
			return false;
		}
		break;
	}

	super_step = 0;
	return true;
}


/*
 * Primitive encoding/decoding routines.
 */

// C.13
void
Encoder::writeIdentifier(const DS_VocabTable::TypedEntryRef& id)
{
	assert(getBitOffset() == 0); // C.13.2

	if (id.hasIndex()) {
		const FI_VocabIndex idx = id.getIndex();

		if (idx != FI_VOCAB_INDEX_NULL) {
			// Use index rules (C.13.4).

			// Write index presence bit.
			writeBits(1, FI_BITS(1,,,,,,,));

			// Write index using C.25.
			writePInt20_bit2(FI_UINT_TO_PINT(idx));
			return;
		}
	}

	// Use literal rules (C.13.3).

	// Write index absence bit.
	writeBits(1, FI_BITS(0,,,,,,,));

	// Write literal using C.22.
	const FI_Length id_len = id->size();
	assert(id_len > 0); // identifiers are always non-empty

	writeNonEmptyOctets_len_bit2(FI_UINT_TO_PINT(id_len));

	FI_Octet *w_buf = getWriteBuffer(id_len);
	if (!w_buf) {
		throw IOException ();
	}

	memcpy(w_buf, id->data(), id_len);

	// Enter literal into vocabulary table (7.13.7).
	id.getIndex();
}

// TODO: Document how different routines share the same sub_step, so that you
// must repeatedly calling a parsing routine to completion, or abort parsing.
bool
Decoder::readIdentifier(DS_VocabTable& string_table,
                        DS_VocabTable::TypedEntryRef& id)
{
	const FI_Octet *r_buf;

	FI_PInt20 idx;
	FI_PInt32 id_len;

reparse:
	switch (sub_sub_step) {
	case 0:
		assert(getBitOffset() == 0); // C.13.2

		// Examine discriminator bits.
		if (!readBits(1)) {
			return false;
		}

		if (!(getBits() & FI_BIT_1)) {
			// Use literal rules.
			sub_sub_step = 2;
			goto reparse;
		}

		sub_sub_step = 1;
		// FALLTHROUGH

	case 1:
		// Read string-index using C.25 (C.13.4).
		if (!readPInt20_bit2(idx)) {
			return false;
		}

		// XXX: Throws IndexOutOfBoundsException on bogus index.
		id = string_table[FI_PINT_TO_UINT(idx)];
		break;

	case 2:
		// Read literal-character-string using C.22 (C.13.3).
		if (!readNonEmptyOctets_len_bit2(id_len)) {
			return false;
		}

		// XXX: FI_PINT_TO_UINT() can't overflow, because we already
		// checked in readNonEmptyOctets_len_bit2().
		saved_len = FI_PINT_TO_UINT(id_len);

		sub_sub_step = 3;
		// FALLTHROUGH

	case 3:
		r_buf = getReadBuffer(saved_len);
		if (!r_buf) {
			return false;
		}

		// Enter literal into vocabulary table (7.13.7).
		id = string_table.getEntry(CharString (reinterpret_cast<const char *>(r_buf), saved_len));
		id.getIndex();
		break;

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}

	sub_sub_step = 0;
	return true;
}

// C.14
// Note that we only choose to encode by literal, not index, except for the
// empty string/value.  Also, we never request values be added to the table.
// FIXME: We do need to support adding values to tables during parsing, though.
void
Encoder::writeValue_bit1(const Value& value)
{
	assert(getBitOffset() == 0); // C.14.2

	if (value.getType() == FI_VALUE_AS_NULL) {
		// Use index rules (C.14.4).

		// Write string-index discriminant (C.14.4: 1).
		writeBits(1, FI_BITS(1,,,,,,,));

		// Write string-index using C.26 (C.14.4).
		writeUInt21_bit2(FI_VOCAB_INDEX_NULL);
		return;
	}

	// Use literal rules (C.14.3).

	// Write literal-character-string discriminant (C.14.3: 0).
	// Write add-to-table (C.14.3.1: 0). (Adding currently not supported.)
	writeBits(2, FI_BITS(0, 0,,,,,,));

	// Write character-string using C.19 (C.14.3.2).
	writeEncoded_bit3(value);
}

bool
Decoder::readValue_bit1(DV_VocabTable& value_table, Value& value)
{
	FI_UInt21 idx;

reparse:
	switch (sub_step) {
	case 0:
		assert(getBitOffset() == 0); // C.14.2

		// Examine discriminator bits.
		if (!readBits(1)) {
			return false;
		}

		if (!(getBits() & FI_BIT_1)) {
			// Use literal rules (C.14.3).
			sub_step = 2;
			goto reparse;
		}

		sub_step = 1;
		// FALLTHROUGH

	case 1:
		// Read string-index using C.26 (C.14.4).
		if (!readUInt21_bit2(idx)) {
			return false;
		}

		// XXX: Throws IndexOutOfBoundsException on bogus index.
		value = *value_table[idx];
		break;

	case 2:
		// Read add-to-table (C.14.3.1).
		readBits(1);

		if (getBits() & FI_BIT_2) {
			// FIXME: Implementation restriction.
			throw UnsupportedOperationException ();
		}

		sub_step = 3;
		// FALLTHROUGH

	case 3:
		// Read character-string using C.19 (C.14.3.2).
		if (!readEncoded_bit3(value)) {
			return false;
		}
		break;

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}

	sub_step = 0;
	return true;
}

// C.17
void
Encoder::writeName_bit2(const DN_VocabTable::TypedEntryRef& name)
{
	assert(getBitOffset() == 1); // C.17.2

	if (name.hasIndex()) {
		const FI_VocabIndex idx = name.getIndex();

		if (idx != FI_VOCAB_INDEX_NULL) {
			// Use index rules (C.17.4).

			// Write name-surrogate-index using C.25.
			writePInt20_bit2(FI_UINT_TO_PINT(idx));
			return;
		}
	}

	// Use literal rules (C.17.3).

	// Write identification (C.17.3: 1111 0).
	// Write prefix and namespace-name presence (C.17.3.1).
	const bool has_pfx = name->pfx_part;
	const bool has_nsn = name->nsn_part;

	writeBits(7, FI_BITS(,1,1,1,1, 0, has_pfx, has_nsn));

	// Write optional prefix using C.13 (C.17.3.2).
	if (has_pfx) {
		writeIdentifier(name->pfx_part);
	}

	// Write optional namespace-name using C.13 (C.17.3.3).
	if (has_nsn) {
		writeIdentifier(name->nsn_part);
	}

	// Write local-name using C.13 (C.17.3.4).
	writeIdentifier(name->local_part);

	// Enter name surrogate into vocabulary table (7.16.7.5).
	name.getIndex();
}

bool
Decoder::readName_bit2(DN_VocabTable& name_table,
                       DN_VocabTable::TypedEntryRef& name)
{
	FI_Octet bits;

	FI_PInt20 idx;
	bool has_pfx, has_nsn;

reparse:
	switch (sub_step) {
	case 0:
		assert(getBitOffset() == 1);

		// Examine discriminator bits.
		bits = getBits() & FI_BITS(,1,1,1,1,1,,);

		if (bits == FI_BITS(,1,1,1,1,0,,)) {
			// Use literal rules.

			// Read as literal-qualified-name (C.17.3).

			// Read presence bits (C.17.3.1).
			readBits(7);

			has_pfx = getBits() & FI_BIT_7;
			has_nsn = getBits() & FI_BIT_8;

			if (has_pfx) {
				// Go to prefix part.
				if (!has_nsn) {
					// Not a valid Fast Infoset.
					throw IllegalStateException ();
				}

				sub_step = 2;
			} else {
				if (has_nsn) {
					// Go to namespace name part.
					sub_step = 3;
				} else {
					// Go directly to local name part.
					sub_step = 4;
				}
			}

			goto reparse;
		}

		sub_step = 1;
		// FALLTHROUGH

	case 1:
		// Read name-surrogate-index using C.25 (C.17.4).
		if (!readPInt20_bit2(idx)) {
			return false;
		}

		// XXX: Throws IndexOutOfBoundsException on bogus index.
		name = name_table[FI_PINT_TO_UINT(idx)];
		break;

		// TODO: Don't allow use of non-declared prefixes/namespaces.
		// Might be best to do this with a separate data structure that
		// handles namespace scoping properly.
	case 2:
		// Read prefix using C.13 (C.17.3.2).
		if (!readIdentifier(vocabulary->prefixes,
		                    saved_pfx_part)) {
			return false;
		}

		sub_step = 3;
		// FALLTHROUGH

	case 3:
		// Read namespace-name using C.13 (C.17.3.3).
		if (!readIdentifier(vocabulary->namespace_names,
		                    saved_nsn_part)) {
			return false;
		}

		sub_step = 4;
		// FALLTHROUGH

	case 4:
		// Read local name using C.13 (C.17.3.4).
		if (!readIdentifier(vocabulary->local_names,
		                    saved_local_part)) {
			return false;
		}

		// Enter literal into vocabulary table (7.13.7).
		name = name_table.getEntry(Name (saved_local_part,
		                                 saved_nsn_part,
		                                 saved_pfx_part));
		name.getIndex();
		break;
	}

	sub_step = 0;
	return true;
}

// C.18
void
Encoder::writeName_bit3(const DN_VocabTable::TypedEntryRef& name)
{
	assert(getBitOffset() == 2); // C.18.2

	if (name.hasIndex()) {
		const FI_VocabIndex idx = name.getIndex();

		if (idx != FI_VOCAB_INDEX_NULL) {
			// Use index rules (C.18.4).

			// Write name-surrogate-index using C.27.
			writePInt20_bit3(FI_UINT_TO_PINT(idx));
			return;
		}
	}

	// Use literal rules (C.18.3).

	// Write identification (C.18.3: 1111).
	// Write prefix and namespace-name presence (C.18.3.1).
	const bool has_pfx = name->pfx_part;
	const bool has_nsn = name->nsn_part;

	writeBits(6, FI_BITS(,,1,1,1,1, has_pfx, has_nsn));

	// Write optional prefix using C.13 (C.18.3.2).
	if (has_pfx) {
		writeIdentifier(name->pfx_part);
	}

	// Write optional namespace-name using C.13 (C.18.3.3).
	if (has_nsn) {
		writeIdentifier(name->nsn_part);
	}

	// Write local-name using C.13 (C.18.3.4).
	writeIdentifier(name->local_part);

	// Enter name surrogate into vocabulary table (7.16.7.5).
	name.getIndex();
}

bool
Decoder::readName_bit3(DN_VocabTable& name_table,
                       DN_VocabTable::TypedEntryRef& name)
{
	FI_Octet bits;

	FI_PInt20 idx;
	bool has_pfx, has_nsn;

reparse:
	switch (sub_step) {
	case 0:
		assert(getBitOffset() == 2);

		// Examine discriminator bits.
		bits = getBits() & FI_BITS(,,1,1,1,1,,);

		if (bits == FI_BITS(,,1,1,1,1,,)) {
			// Use literal rules.

			// Read as literal-qualified-name (C.18.3).

			// Read presence bits (C.18.3.1).
			readBits(6);

			has_pfx = getBits() & FI_BIT_7;
			has_nsn = getBits() & FI_BIT_8;

			if (has_pfx) {
				// Go to prefix part.
				if (!has_nsn) {
					// Not a valid Fast Infoset.
					throw IllegalStateException ();
				}

				sub_step = 2;
			} else {
				if (has_nsn) {
					// Go to namespace name part.
					sub_step = 3;
				} else {
					// Go directly to local name part.
					sub_step = 4;
				}
			}

			goto reparse;
		}

		sub_step = 1;
		// FALLTHROUGH

	case 1:
		// Read name-surrogate-index using C.27 (C.18.4).
		if (!readPInt20_bit3(idx)) {
			return false;
		}

		// XXX: Throws IndexOutOfBoundsException on bogus index.
		name = name_table[FI_PINT_TO_UINT(idx)];
		break;

		// TODO: Don't allow use of non-declared prefixes/namespaces.
		// Might be best to do this with a separate data structure that
		// handles namespace scoping properly.
	case 2:
		// Read prefix using C.13 (C.18.3.2).
		if (!readIdentifier(vocabulary->prefixes,
		                    saved_pfx_part)) {
			return false;
		}

		sub_step = 3;
		// FALLTHROUGH

	case 3:
		// Read namespace-name using C.13 (C.18.3.3).
		if (!readIdentifier(vocabulary->namespace_names,
		                    saved_nsn_part)) {
			return false;
		}

		sub_step = 4;
		// FALLTHROUGH

	case 4:
		// Read local name using C.13 (C.18.3.4).
		if (!readIdentifier(vocabulary->local_names,
		                    saved_local_part)) {
			return false;
		}

		// Enter literal into vocabulary table (7.13.7).
		name = name_table.getEntry(Name (saved_local_part,
		                                 saved_nsn_part,
		                                 saved_pfx_part));
		name.getIndex();
		break;
	}

	sub_step = 0;
	return true;
}

// C.19
enum EncodingMode {
	ENCODE_AS_UTF8        = FI_BITS(,,0,0,,,,), // C.19.3.1
	ENCODE_AS_UTF16       = FI_BITS(,,0,1,,,,), // C.19.3.2
	ENCODE_WITH_ALPHABET  = FI_BITS(,,1,0,,,,), // C.19.3.3
	ENCODE_WITH_ALGORITHM = FI_BITS(,,1,1,,,,)  // C.19.3.4
}; // enum EncodingMode

void
Encoder::writeEncoded_bit3(const Value& value)
{
	assert(getBitOffset() == 2); // C.19.2
	assert(value.getCount() > 0); // non-empty

	// Selecting the encoding based on value type.
	EncodingMode mode;

	switch (value.getType()) {
	case FI_VALUE_AS_OCTETS:
		// Encode as octets of UTF-8.
		// FIXME: This needs to be actual UTF-8, not just arbitrary
		// octets.  Arbitrary octets will require an encoding algorithm
		// of their own.
		mode = ENCODE_AS_UTF8;
		break;

	default:
		// Unsupported value type.
		throw UnsupportedOperationException ();
	}

	// Write discriminant (C.19.3).
	writeBits(2, mode);

	// Write the encoded octet string.
	FI_Octet *w_buf;
	FI_Length len;

	switch (mode) {
	case ENCODE_AS_UTF8:
		// Write octets using C.23 (C.19.4).
		len = value.getCount();

		writeNonEmptyOctets_len_bit5(FI_UINT_TO_PINT(len));

		w_buf = getWriteBuffer(len);

		memcpy(w_buf, value.getValue(), len);
		break;

	case ENCODE_AS_UTF16:
		// TODO: We don't use the utf-16 alternative.
		// Write octets using C.23 (C.19.4).
		throw UnsupportedOperationException ();

	case ENCODE_WITH_ALPHABET:
		// TODO: We don't use the restricted-alphabet alternative.
		// Write restricted-alphabet index using C.29 (C.19.3.3).
		// Write octets using C.23 (C.19.4).
		throw UnsupportedOperationException ();

	case ENCODE_WITH_ALGORITHM:
		// TODO: We don't support encoding algorithms yet.
		// Write encoding-algorithm index using C.29 (C.19.3.4).
		// Write octets using C.23 (C.19.4).
		throw UnsupportedOperationException ();

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}
}

bool
Decoder::readEncoded_bit3(Value& value)
{
	const FI_Octet *r_buf;

	FI_PInt32 octets_len;

reparse:
	switch (sub_sub_step) {
	case 0:
		assert(getBitOffset() == 2); // C.19.2

		// Examine discriminator bits.
		readBits(2);

		switch (getBits() & FI_BITS(,,1,1,,,,)) {
		case ENCODE_AS_UTF8:
			// Use UTF-8 decoding rules.
			sub_sub_step = 1;
			goto reparse;

		case ENCODE_AS_UTF16:
			// TODO: We don't use the utf-16 alternative.
			// Read octets using C.23 (C.19.4).
			throw UnsupportedOperationException ();

		case ENCODE_WITH_ALPHABET:
			// TODO: We don't use the restricted-alphabet
			// alternative.
			// FIXME: Implementation restriction.
			// Read restricted-alphabet index using C.29 (C.19.3.3).
			// Read octets using C.23 (C.19.4).
			throw UnsupportedOperationException ();

		case ENCODE_WITH_ALGORITHM:
			// TODO: We don't support encoding algorithms yet.
			// FIXME: Implementation restriction.
			// Read encoding-algorithm index using C.29 (C.19.3.4).
			// Read octets using C.23 (C.19.4).
			throw UnsupportedOperationException ();
		}
		break;

	case 1:
		// Decode UTF-8.

		// Read octets using C.23 (C.19.4).
		if (!readNonEmptyOctets_len_bit5(octets_len)) {
			return false;
		}

		// XXX: FI_PINT_TO_UINT() can't overflow, because we already
		// checked in readNonEmptyOctets_len_bit5().
		saved_len = FI_PINT_TO_UINT(octets_len);

		sub_sub_step = 2;
		// FALLTHROUGH

	case 2:
		r_buf = getReadBuffer(saved_len);
		if (!r_buf) {
			return false;
		}

		// Set Value from buffer contents.
		// TODO: Support other value types.
		if (!value.setValue(FI_VALUE_AS_OCTETS, saved_len, r_buf)) {
			// Couldn't set the attribute value.
			throw UnsupportedOperationException ();
		}
		break;

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}

	sub_sub_step = 0;
	return true;
}

// C.22.3
// Note that this only writes the length of the octet string.
void
Encoder::writeNonEmptyOctets_len_bit2(FI_PInt32 len)
{
	assert(getBitOffset() == 1); // C.22.2
	assert(len <= FI_PINT32_MAX);

	if (len > FI_UINT_TO_PINT(FI_LENGTH_MAX)) {
		// Will overflow.
		// FIXME: Implementation restriction.
		throw UnsupportedOperationException ();
	}

	FI_Octet *w_buf;

	if (len <= FI_UINT_TO_PINT(64)) {
		// [1,64] (C.22.3.1)
		len -= FI_UINT_TO_PINT(1);

		writeBits(7, FI_BITS(,0,,,,,,) | len);
	} else if (len <= FI_UINT_TO_PINT(320)) {
		// [65,320] (C.22.3.2)
		len -= FI_UINT_TO_PINT(65);

		writeBits(7, FI_BITS(,1,0, 0,0,0,0,0));

		w_buf = getWriteBuffer(1);

		w_buf[0] = len;
	} else {
		// [321,2^32] (C.22.3.3)
		len -= FI_UINT_TO_PINT(321);

		writeBits(7, FI_BITS(,1,1, 0,0,0,0,0));

		w_buf = getWriteBuffer(4);

		w_buf[0] = len >> 24;
		w_buf[1] = len >> 16;
		w_buf[2] = len >> 8;
		w_buf[3] = len;
	}
}

bool
Decoder::readNonEmptyOctets_len_bit2(FI_PInt32& len)
{
	assert(getBitOffset() == 1); // C.22.2

	const FI_Octet *r_buf;

	FI_PInt32 tmp_len;

	if (getBits() & FI_BIT_2) {
		switch (getBits() & FI_BITS(,/*1*/,1,1,1,1,1,1)) {
		case FI_BITS(,/*1*/,0, 0,0,0,0,0):
			// [65,320] (C.22.3.2)
			r_buf = getReadBuffer(1);
			if (!r_buf) {
				return false;
			}

			tmp_len = r_buf[0];

			tmp_len += FI_UINT_TO_PINT(65);
			break;

		case FI_BITS(,/*1*/,1, 0,0,0,0,0):
			// [321,2^32] (C.22.3.3)
			r_buf = getReadBuffer(4);
			if (!r_buf) {
				return false;
			}

			tmp_len = r_buf[0] << 24;
			tmp_len |= r_buf[1] << 16;
			tmp_len |= r_buf[2] << 8;
			tmp_len |= r_buf[3];

			if (tmp_len > FI_PINT32_MAX - FI_UINT_TO_PINT(321)) {
				// Not a valid Fast Infoset.
				throw IllegalStateException ();
			}

			tmp_len += FI_UINT_TO_PINT(321);
			break;

		default:
			// Not a valid Fast Infoset.
			throw IllegalStateException ();
		}
	} else {
		// [1,64] (C.22.3.1)
		tmp_len = getBits() & FI_BITS(,/*0*/,1,1,1,1,1,1);

		tmp_len += FI_UINT_TO_PINT(1);
	}

	if (tmp_len > FI_UINT_TO_PINT(FI_LENGTH_MAX)) {
		// Will overflow.
		// FIXME: Implementation restriction.
		throw UnsupportedOperationException ();
	}

	readBits(7);

	len = tmp_len;
	return true;
}

// C.23.3
// Note that this only writes the length of the octet string.
void
Encoder::writeNonEmptyOctets_len_bit5(FI_PInt32 len)
{
	assert(getBitOffset() == 4); // C.23.2
	assert(len <= FI_PINT32_MAX);

	if (len > FI_UINT_TO_PINT(FI_LENGTH_MAX)) {
		// Will overflow.
		// FIXME: Implementation restriction.
		throw UnsupportedOperationException ();
	}

	FI_Octet *w_buf;

	if (len <= FI_UINT_TO_PINT(8)) {
		// [1,8] (C.23.3.1)
		len -= FI_UINT_TO_PINT(1);

		writeBits(4, FI_BITS(,,,,0,,,) | len);
	} else if (len <= FI_UINT_TO_PINT(264)) {
		// [9,264] (C.23.3.2)
		len -= FI_UINT_TO_PINT(9);

		writeBits(4, FI_BITS(,,,,1,0, 0,0));

		w_buf = getWriteBuffer(1);

		w_buf[0] = len;
	} else {
		// [265,2^32] (C.23.3.3)
		len -= FI_UINT_TO_PINT(265);

		writeBits(4, FI_BITS(,,,,1,1, 0,0));

		w_buf = getWriteBuffer(4);

		w_buf[0] = len >> 24;
		w_buf[1] = len >> 16;
		w_buf[2] = len >> 8;
		w_buf[3] = len;
	}
}

bool
Decoder::readNonEmptyOctets_len_bit5(FI_PInt32& len)
{
	assert(getBitOffset() == 4); // C.23.2

	const FI_Octet *r_buf;

	FI_PInt32 tmp_len;

	if (getBits() & FI_BIT_5) {
		switch (getBits() & FI_BITS(,,,,/*1*/,1,1,1)) {
		case FI_BITS(,,,,/*1*/,0, 0,0):
			// [9,264] (C.23.3.2)
			r_buf = getReadBuffer(1);
			if (!r_buf) {
				return false;
			}

			tmp_len = r_buf[0];

			tmp_len += FI_UINT_TO_PINT(9);
			break;

		case FI_BITS(,,,,/*1*/,1, 0,0):
			// [265,2^32] (C.23.3.3)
			r_buf = getReadBuffer(4);
			if (!r_buf) {
				return false;
			}

			tmp_len = r_buf[0] << 24;
			tmp_len |= r_buf[1] << 16;
			tmp_len |= r_buf[2] << 8;
			tmp_len |= r_buf[3];

			if (tmp_len > FI_PINT32_MAX - FI_UINT_TO_PINT(265)) {
				// Not a valid Fast Infoset.
				throw IllegalStateException ();
			}

			tmp_len += FI_UINT_TO_PINT(265);
			break;

		default:
			// Not a valid Fast Infoset.
			throw IllegalStateException ();
		}
	} else {
		// [1,8] (C.23.3.1)
		tmp_len = getBits() & FI_BITS(,,,,/*0*/,1,1,1);

		tmp_len += FI_UINT_TO_PINT(1);
	}

	if (tmp_len > FI_UINT_TO_PINT(FI_LENGTH_MAX)) {
		// Will overflow.
		// FIXME: Implementation restriction.
		throw UnsupportedOperationException ();
	}

	readBits(4);

	len = tmp_len;
	return true;
}

// C.25
void
Encoder::writePInt20_bit2(FI_PInt20 val)
{
	assert(getBitOffset() == 1); // C.25.1
	assert(val <= FI_PINT20_MAX);

	FI_Octet *w_buf;

	if (val <= FI_UINT_TO_PINT(64)) {
		// [1,64] (C.25.2)
		val -= FI_UINT_TO_PINT(1);

		writeBits(7, FI_BITS(,0,,,,,,) | val);
	} else if (val <= FI_UINT_TO_PINT(8256)) {
		// [65,8256] (C.25.3)
		val -= FI_UINT_TO_PINT(65);

		writeBits(7, FI_BITS(,1,0,,,,,) | val >> 8);

		w_buf = getWriteBuffer(1);

		w_buf[0] = val;
	} else {
		// [8257,2^20] (C.25.4)
		val -= FI_UINT_TO_PINT(8257);

		writeBits(7, FI_BITS(,1,1,,,,,) | val >> 16);

		w_buf = getWriteBuffer(2);

		w_buf[0] = val >> 8;
		w_buf[1] = val;
	}
}

bool
Decoder::readPInt20_bit2(FI_PInt20& val)
{
	assert(getBitOffset() == 1); // C.25.1

	const FI_Octet *r_buf;

	FI_PInt20 tmp_val;

	if (getBits() & FI_BIT_2) {
		if (getBits() & FI_BIT_3) {
			if (getBits() & FI_BIT_4) {
				// Not a valid Fast Infoset.
				throw IllegalStateException ();
			}

			// [8257,2^20] (C.25.4)
			r_buf = getReadBuffer(2);
			if (!r_buf) {
				return false;
			}

			tmp_val = (getBits() & FI_BITS(,/*1*/,/*1*/,/*0*/,1,1,1,1)) << 16;
			tmp_val |= r_buf[0] << 8;
			tmp_val |= r_buf[1];

			if (tmp_val > FI_PINT20_MAX - FI_UINT_TO_PINT(8257)) {
				// Not a valid Fast Infoset.
				throw IllegalStateException ();
			}

			tmp_val += FI_UINT_TO_PINT(8257);
		} else {
			// [65,8256] (C.25.3)
			r_buf = getReadBuffer(1);
			if (!r_buf) {
				return false;
			}

			tmp_val = (getBits() & FI_BITS(,/*1*/,/*0*/,1,1,1,1,1)) << 8;
			tmp_val |= r_buf[0];

			tmp_val += FI_UINT_TO_PINT(65);
		}
	} else {
		// [1,64] (C.25.2)
		tmp_val = getBits() & FI_BITS(,/*0*/,1,1,1,1,1,1);

		tmp_val += FI_UINT_TO_PINT(1);
	}

	readBits(7);

	val = tmp_val;
	return true;
}

// C.26
void
Encoder::writeUInt21_bit2(FI_UInt21 val)
{
	assert(getBitOffset() == 1); // C.26.1
	assert(val <= FI_ONE_MEG);

	if (val == 0) {
		// Write zero value (C.26.2: 1111111).
		writeBits(7, FI_BITS(,1,1,1,1,1,1,1));
	} else {
		// Write value using C.25 (C.26.2).
		writePInt20_bit2(FI_UINT_TO_PINT(val));
	}
}

bool
Decoder::readUInt21_bit2(FI_UInt21& val)
{
	assert(getBitOffset() == 1); // C.26.1

	FI_PInt20 tmp_val;

	switch (getBits() & FI_BITS(,1,1,1,1,1,1,1)) {
	case FI_BITS(,1,1,1,1,1,1,1):
		// Read zero value (C.26.2: 1111111).
		readBits(7);
		val = 0;
		break;

	default:
		// Read value using C.25 (C.26.2).
		if (!readPInt20_bit2(tmp_val)) {
			return false;
		}

		// XXX: FI_PINT_TO_UINT() can't overflow FI_ONE_MEG, because
		// we already checked in readPInt20_bit2().
		val = FI_PINT_TO_UINT(tmp_val);
		break;
	}

	return true;
}

// C.27
void
Encoder::writePInt20_bit3(FI_PInt20 val)
{
	assert(getBitOffset() == 2); // C.27.1
	assert(val <= FI_PINT20_MAX);

	FI_Octet *w_buf;

	if (val <= FI_UINT_TO_PINT(32)) {
		// [1,32] (C.27.2)
		val -= FI_UINT_TO_PINT(1);

		writeBits(6, FI_BITS(,,0,,,,,) | val);
	} else if (val <= FI_UINT_TO_PINT(2080)) {
		// [33,2080] (C.27.3)
		val -= FI_UINT_TO_PINT(33);

		writeBits(6, FI_BITS(,,1,0,0,,,) | val >> 8);

		w_buf = getWriteBuffer(1);

		w_buf[0] = val;
	} else if (val <= FI_UINT_TO_PINT(526368)) {
		// [2081,526368] (C.27.4)
		val -= FI_UINT_TO_PINT(2081);

		writeBits(6, FI_BITS(,,1,0,1,,,) | val >> 16);

		w_buf = getWriteBuffer(2);

		w_buf[0] = val >> 8;
		w_buf[1] = val;
	} else {
		// [526369,2^20] (C.27.5)
		val -= FI_UINT_TO_PINT(526369);

		writeBits(6, FI_BITS(,,1,1,0,0,0,0));

		w_buf = getWriteBuffer(3);

		// Padding '0000'
		w_buf[0] = val >> 16;
		w_buf[1] = val >> 8;
		w_buf[2] = val;
	}
}

bool
Decoder::readPInt20_bit3(FI_PInt20& val)
{
	assert(getBitOffset() == 2);

	const FI_Octet *r_buf;

	FI_PInt20 tmp_val;

	if (getBits() & FI_BIT_3) {
		switch (getBits() & FI_BITS(,,/*1*/,1,1,,,)) {
		case FI_BITS(,,/*1*/,0,0,,,):
			// [33,2080] (C.27.3)
			r_buf = getReadBuffer(1);
			if (!r_buf) {
				return false;
			}

			tmp_val = (getBits() & FI_BITS(,,,,,1,1,1)) << 8;
			tmp_val |= r_buf[0];

			tmp_val += FI_UINT_TO_PINT(33);
			break;

		case FI_BITS(,,/*1*/,0,1,,,):
			// [2081,526368] (C.27.4)
			r_buf = getReadBuffer(2);
			if (!r_buf) {
				return false;
			}

			tmp_val = (getBits() & FI_BITS(,,,,,1,1,1)) << 16;
			tmp_val |= r_buf[0] << 8;
			tmp_val |= r_buf[1];

			tmp_val += FI_UINT_TO_PINT(2081);
			break;

		case FI_BITS(,,/*1*/,1,0,,,):
			// [526369,2^20] (C.27.5)
			r_buf = getReadBuffer(3);
			if (!r_buf) {
				return false;
			}

			// Padding: 000 0000.
			if ((getBits() & FI_BITS(,,,,,1,1,1))
			    || (r_buf[0] & FI_BITS(1,1,1,1,,,,))) {
				// Not a valid Fast Infoset.
				throw IllegalStateException ();
			}

			tmp_val = (r_buf[0] & FI_BITS(,,,,1,1,1,1)) << 16;
			tmp_val |= r_buf[1] << 8;
			tmp_val |= r_buf[2];

			if (tmp_val > FI_PINT20_MAX - FI_UINT_TO_PINT(526369)) {
				// Not a valid Fast Infoset.
				throw IllegalStateException ();
			}

			tmp_val += FI_UINT_TO_PINT(526369);
			break;

		default:
			// Not a valid Fast Inofset.
			throw IllegalStateException ();
		}
	} else {
		// [1,32] (C.27.2)
		tmp_val = getBits() & FI_BITS(,,/*0*/,1,1,1,1,1);

		tmp_val += FI_UINT_TO_PINT(1);
	}

	readBits(6);

	val = tmp_val;
	return true;
}

// C.29
void
Encoder::writePInt8(FI_PInt8 val)
{
	assert(val <= FI_PINT8_MAX);

	// [1,256] (C.29.2)
	val -= FI_UINT_TO_PINT(1);

	// Write octet + partial.
	switch (getBitOffset()) {
	case 4:
		// Starting on bit 5.
		writeBits(4, (val & FI_BITS(1,1,1,1,,,,)) >> 4);
		writeBits(4, (val & FI_BITS(,,,,1,1,1,1)) << 4);
		break;

	case 6:
		// Starting on bit 7.
		// TODO: We don't actually use this right now.
		writeBits(2, (val & FI_BITS(1,1,,,,,,)) >> 6);
		writeBits(6, (val & FI_BITS(,,1,1,1,1,1,1)) << 2);
		break;

	default:
		// Shouldn't happen (C.29.1).
		throw AssertionFailureException ();
	}
}

bool
Decoder::readPInt8(FI_PInt8& val)
{
	FI_PInt8 tmp_val;

	// Read partial + octet.
	switch (getBitOffset()) {
	case 4:
		// Starting on bit 5.
		tmp_val = (getBits() & FI_BITS(,,,,1,1,1,1)) << 4;

		if (!readBits(4)) {
			return false;
		}

		tmp_val |= (getBits() & FI_BITS(1,1,1,1,,,,)) >> 4;
		break;

	case 6:
		// Starting on bit 7.
		tmp_val = (getBits() & FI_BITS(,,,,,,1,1)) << 6;

		if (!readBits(6)) {
			return false;
		}

		tmp_val |= (getBits() & FI_BITS(1,1,1,1,1,1,,)) >> 2;
		break;

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}

	// [1,256] (C.29.2).
	val = tmp_val + FI_UINT_TO_PINT(1);
	return true;
}


/*
 * FIXME: Some broken code for initial vocabulary tables.
 */

#if 0 // defined(FI_USE_INITIAL_VOCABULARY)
// C.16
bool
write_name_surrogate(FI_OctetStream *stream, const VocabTable::EntryRef& name)
{
	assert(fi_get_stream_num_bits(stream) == 6); // C.16.2

	FI_Octet presence_flags = 0;

	// Set prefix-string-index presence flag (C.16.3).
	if (name.prefix_idx) {
		presence_flags |= FI_BIT_1;
	}

	// Set namespace-name-string-index presence flag (C.16.4).
	if (name.namespace_idx) {
		presence_flags |= FI_BIT_2;
	}

	if (!fi_write_stream_bits(stream, 2, presence_flags)) {
		return false;
	}

	// If prefix-string-index, '0' + C.25 (C.16.5).
	if (name.prefix_idx) {
		if (!fi_write_stream_bits(stream, 1, 0)) {
			return false;
		}

		if (!write_non_zero_uint20_bit_2(stream, name.prefix_idx)) {
			return false;
		}
	}

	// If namespace-name-string-index, '0' + C.25 (C.16.6).
	if (name.namespace_idx) {
		if (!fi_write_stream_bits(stream, 1, 0)) {
			return false;
		}

		if (!write_non_zero_uint20_bit_2(stream, name.namespace_idx)) {
			return false;
		}
	}

	// For local-name-string-index, '0' + C.25 (C.16.7).
	if (!fi_write_stream_bits(stream, 1, 0)) {
		return false;
	}

	if (!write_non_zero_uint20_bit_2(stream, name.local_idx)) {
		return false;
	}

	return true;
}

// C.21
bool
write_length_sequence_of(FI_OctetStream *stream, FI_PInt20 len)
{
	assert(len > 0 && len <= FI_ONE_MEG);
	assert(fi_get_stream_num_bits(stream) == 0); // C.21.1

	FI_Octet *w_buf;

	if (len <= 128) {
		// [1,128] (C.21.2)
		w_buf = fi_get_stream_write_buffer(stream, 1);
		if (!w_buf) {
			return false;
		}

		w_buf[0] = len - 1;
	} else {
		// [129,2^20] (C.21.3)
		w_buf = fi_get_stream_write_buffer(stream, 3);
		if (!w_buf) {
			return false;
		}

		len -= 129;

		w_buf[0] = FI_BITS(1,0,0,0,,,,) | (len >> 16);
		w_buf[1] = len >> 8;
		w_buf[2] = len;
	}

	return true;
}
#endif // FI_USE_INITIAL_VOCABULARY

} // namespace FI
} // namespace BTech
#if 0
bool
read_xml_decl(FI_OctetStream *stream, FI_Length& r_len_state)
{
	const FI_Octet *r_buf;
	FI_Length avail_len;

	// Get at least one more octet than last time.
	avail_len = fi_try_read_stream(stream, &r_buf, 0, r_len_state + 1);
	if (avail_len <= r_len_state) {
		return false;
	}

	// Search for "?>" from r_len_state - 2 to the current position.
	FI_Length ii;

	bool saw_question_mark = false;
	for (ii = r_len_state - 1; ii < avail_len; ii++) {
		if (saw_question_mark) {
			if (r_buf[ii] == '>') {
				// Probably just saw an XML declaration.
				// TODO: Implement the full grammar.
				break;
			} else if (r_buf[ii] != '?') {
				saw_question_mark = false;
			}
		} else if (r_buf[ii] == '?') {
			saw_question_mark = true;
		}
	}

	if (ii < avail_len) {
		ii++; // advance ii to input XML declaration length
	} else {
		// Didn't find a "?>" yet.
		r_len_state = ii;

		// Just as a sanity check, give up after 192 octets or so.
		if (r_len_state > 192) {
			throw UnsupportedOperationException ();
		}

		// Mark the stream as needing at least one more byte.
		if (saw_question_mark) {
			fi_set_stream_needed_length(stream, 1);
		} else {
			fi_set_stream_needed_length(stream, 2);
		}

		return false;
	}

	// Try to match against the 9 possible XML declarations for a valid
	// Fast Infoset document (12.3).
	const int XML_DECL_MAX = sizeof(xml_decl) / sizeof(xml_decl[0]);

	int jj;

	for (jj = 0; jj < XML_DECL_MAX; jj++) {
		if (strlen(xml_decl[jj]) != ii) {
			// Not even the same length.
			continue;
		}

		if (memcmp(r_buf, xml_decl[jj], ii) == 0) {
			// Match, yay.
			break;;
		}
	}

	if (jj == XML_DECL_MAX) {
		// TODO: We're being rather strict here about what we accept,
		// since we didn't implement a full XML declaration parser.
		// This behavior is allowed by the X.891 standard, but probably
		// isn't very friendly.
		throw IllegalStateException ();
	}

	// FIXME: We don't verify that the XML declaration matches the contents
	// of the Document section.

	// Success, advance cursor.
	fi_advance_stream_cursor(stream, ii);
	return true;
}

{
		// A Fast Infoset may optionally begin with a UTF-8 encoded XML
		// declaration (which begins with "<?xml") (12.3).
#if 0
		if (fi_try_read_stream(stream, &r_buf, 0, 4) < 4) {
			return false;
		}
#endif // FIXME

		if (r_buf[0] != '<' || r_buf[1] != '?'
		    || r_buf[2] != 'x' || r_buf[3] != 'm') {
			// Try again as a Fast Infoset identification string.
			r_header_state = MAIN_HEADER_STATE;
			// XXX: Gotos are not evil, but if you really feel this
			// one is so egregious, you can replace it with a
			// continue and a loop, or some other construct.  But I
			// think that's even worse.
			goto redispatch;
		}

		r_header_state = XML_DECL_HEADER_STATE;
		r_len_state = 4;
		// FALLTHROUGH

	case XML_DECL_HEADER_STATE:
		// Try to read XML declaration.
#if 0
		if (!read_xml_decl(stream, r_len_state)) {
			return false;
		}
#endif // FIXME
}

#endif // 0