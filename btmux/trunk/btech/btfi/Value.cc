/*
 * Dynamically-castable Value class implementation.
 */
#include "autoconf.h"

#include <cstddef>
#include <cstring>
#include <cassert>

#include <memory>

#include "values.h"

#include "Codec.hh"

#include "Value.hh"
#include "Serializable.hh"

// Use auto_ptr to avoid memory leak warnings from valgrind & friends.
using std::auto_ptr;


namespace BTech {
namespace FI {

/*
 * Dynamic value table.  Holds pre-decoded values from parsing. (We don't
 * support writing values by index, at least at this time.)
 *
 * ATTRIBUTE VALUE
 * CONTENT CHARACTER CHUNK
 * OTHER STRING
 *
 * In the standard, this is just a dynamic string table (8.4).
 */

DV_VocabTable::TypedVocabTable *
DV_VocabTable::builtin_table()
{
	static DV_VocabTable builtins (true, FI_ONE_MEG);
	return &builtins;
}

DV_VocabTable::DV_VocabTable(bool read_only, FI_VocabIndex max_idx)
: DynamicTypedVocabTable<value_type> (true, max_idx)
{
	// Index 0 is the empty value.
	base_idx = 0;
	last_idx = -1;

	// Add built-in values.
	static auto_ptr<Entry>
	entry_0 (addStaticEntry (FI_VOCAB_INDEX_NULL, Value ()));
}

DV_VocabTable::DV_VocabTable()
: DynamicTypedVocabTable<value_type> (false, builtin_table())
{
}


/*
 * Value implementation.
 */

namespace {

// Replace value of existing type.
template<typename T>
void
set_value_buf(char *value_buf, size_t count, const void *buf)
{
	const T *const src_buf = static_cast<const T *>(buf);
	T *const dst_buf = reinterpret_cast<T *>(value_buf);

	for (size_t ii = 0; ii < count; ii++) {
		dst_buf[ii] = src_buf[ii];
	}
}

// Create value of different type.
template<typename T>
char *
new_value_buf(size_t count, const void *buf)
{
	char *new_buf;

	if (count > ((size_t)-1) / sizeof(T)) {
		// Overflow.
		return 0;
	}

	try {
		new_buf = new char[count * sizeof(T)];
	} catch (const std::bad_alloc& e) {
		return 0;
	}

	set_value_buf<T>(new_buf, count, buf);

	return new_buf;
}

// Test whether one value of a given type is less than another of the same
// type.  Strings of values are compared lexicographically.
//
// The type involved must naturally be LessThanComparable.
template<typename T>
bool
less_value_buf(size_t count, const void *lhs, const void *rhs)
{
	const T *const lhs_buf = static_cast<const T *>(lhs);
	const T *const rhs_buf = static_cast<const T *>(rhs);

	for (size_t ii = 0; ii < count; ii++) {
		if (lhs_buf[ii] < rhs_buf[ii]) {
			// lhs < rhs
			return true;
		} else if (rhs_buf[ii] < lhs_buf[ii]) {
			// lhs > rhs
			return false;
		}
	}

	// lhs == rhs
	return false;
}

} // anonymous namespace

Value::~Value()
{
	delete[] value_buf;
}

bool
Value::setValue(FI_ValueType type, size_t count, const void *buf)
{
	// Empty values are always of type FI_VALUE_AS_NULL.
	if (count == 0) {
		delete[] value_buf;

		value_type = FI_VALUE_AS_NULL;
		value_count = 0;
		value_buf = 0;
		return true;
	}

	// Create/assign the new value.
	const bool in_place = (value_type == type && value_count == count);

	char *new_buf;

	switch (type) {
	case FI_VALUE_AS_SHORT:
		if (in_place) {
			set_value_buf<FI_Int16>(value_buf, count, buf);
		} else {
			new_buf = new_value_buf<FI_Int16>(count, buf);
		}
		break;

	case FI_VALUE_AS_INT:
		if (in_place) {
			set_value_buf<FI_Int32>(value_buf, count, buf);
		} else {
			new_buf = new_value_buf<FI_Int32>(count, buf);
		}
		break;

	case FI_VALUE_AS_LONG:
		if (in_place) {
			set_value_buf<FI_Int64>(value_buf, count, buf);
		} else {
			new_buf = new_value_buf<FI_Int64>(count, buf);
		}
		break;

	case FI_VALUE_AS_BOOLEAN:
		if (in_place) {
			set_value_buf<FI_Boolean>(value_buf, count, buf);
		} else {
			new_buf = new_value_buf<FI_Boolean>(count, buf);
		}
		break;

	case FI_VALUE_AS_FLOAT:
		if (in_place) {
			set_value_buf<FI_Float32>(value_buf, count, buf);
		} else {
			new_buf = new_value_buf<FI_Float32>(count, buf);
		}
		break;

	case FI_VALUE_AS_DOUBLE:
		if (in_place) {
			set_value_buf<FI_Float64>(value_buf, count, buf);
		} else {
			new_buf = new_value_buf<FI_Float64>(count, buf);
		}
		break;

	case FI_VALUE_AS_UUID:
		if (in_place) {
			set_value_buf<FI_UUID>(value_buf, count, buf);
		} else {
			new_buf = new_value_buf<FI_UUID>(count, buf);
		}
		break;

	case FI_VALUE_AS_OCTETS:
		if (in_place) {
			set_value_buf<FI_Octet>(value_buf, count, buf);
		} else {
			new_buf = new_value_buf<FI_Octet>(count, buf);
		}
		break;

	default:
		// Unknown type requested.
		return false;
	}

	// Perform the swap.
	if (!in_place) {
		if (!new_buf) {
			return false;
		}

		delete[] value_buf;

		value_type = type;
		value_count = count;
		value_buf = new_buf;
	}

	return true;
}

// Imposes an absolute ordering on all possible values.
bool
Value::operator < (const Value& rhs) const
{
	// Values are directly comparable if they have the same type.  If they
	// have different types, then they are ordered by increasing value of
	// the FI_ValueType enumeration.
	if (value_type < rhs.value_type) {
		return true;
	} else if (rhs.value_type > value_type) {
		return false;
	}

	// All FI_VALUE_AS_NULL values compare as equal (not-less-than).
	if (value_type == FI_VALUE_AS_NULL) {
		assert(rhs.value_type == FI_VALUE_AS_NULL);
		return false;
	}

	assert(value_count > 0);

	// Values are directly comparable if they have the same count.  If they
	// have different counts, then they are ordered by increasing count.
	if (value_count < rhs.value_count) {
		return true;
	} else if (rhs.value_count < value_count) {
		return false;
	}

	// Compare the values.
	switch (value_type) {
	case FI_VALUE_AS_SHORT:
		return less_value_buf<FI_Int16>(value_count,
		                                value_buf, rhs.value_buf);

	case FI_VALUE_AS_INT:
		return less_value_buf<FI_Int32>(value_count,
		                                value_buf, rhs.value_buf);

	case FI_VALUE_AS_LONG:
		return less_value_buf<FI_Int64>(value_count,
		                                value_buf, rhs.value_buf);

	case FI_VALUE_AS_BOOLEAN:
		return less_value_buf<FI_Boolean>(value_count,
		                                  value_buf, rhs.value_buf);

	case FI_VALUE_AS_FLOAT:
		return less_value_buf<FI_Float32>(value_count,
		                                  value_buf, rhs.value_buf);

	case FI_VALUE_AS_DOUBLE:
		return less_value_buf<FI_Float64>(value_count,
		                                  value_buf, rhs.value_buf);

	case FI_VALUE_AS_UUID:
		return less_value_buf<FI_UUID>(value_count,
		                               value_buf, rhs.value_buf);

	case FI_VALUE_AS_OCTETS:
		return less_value_buf<FI_Octet>(value_count,
		                                value_buf, rhs.value_buf);

	default:
		// Should never happen.
		assert(false);
		return false;
	}
}


/*
 * Fast Infoset serialization support.
 */

void
Value::setVocabTable(DV_VocabTable& new_vocab_table)
{
	vocab_table = &new_vocab_table;
	super_step = 0;
}

void
Value::write(Encoder& encoder) const
{
	switch (encoder.getBitOffset()) {
	case 0:
		// Write value starting on first bit (C.14).
		write_bit1(encoder);
		break;

	case 2:
#if 0
		// Write value starting on third bit (C.15).
		write_bit3(encoder);
#endif // FIXME
		break;

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}
}

bool
Value::read(Decoder& decoder)
{
reparse:
	switch (super_step) {
	case 0:
		sub_step = 0;

		switch (decoder.getBitOffset()) {
		case 0:
			// On first bit.
			break;

		case 2:
			// On third bit.
			super_step = 2;
			goto reparse;

		default:
			// Should never happen.
			throw AssertionFailureException ();
		}

		super_step = 1;
		// FALLTHROUGH

	case 1:
		// Read value header starting on first bit (C.14).
		if (!read_bit1(decoder)) {
			return false;
		}
		break;

	case 2:
#if 0
		// Read value header starting on third bit (C.15).
		if (!read_bit3(decoder)) {
			return false;
		}
#endif // FIXME
		break;

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}

	super_step = 0;
	return true;
}

// C.14/C.19
// Note that we only choose to encode by literal, not index, except for the
// empty string/value.  Also, we never request values be added to the table,
// although we do support adding during parsing.
void
Value::write_bit1(Encoder& encoder) const
{
	assert(encoder.getBitOffset() == 0); // C.14.2

	if (value_type == FI_VALUE_AS_NULL) {
		// Use index rules (C.14.4).

		// Write string-index discriminant (C.14.4: 1).
		encoder.writeBits(1, FI_BITS(1,,,,,,,));

		// Write string-index using C.26 (C.14.4).
		encoder.writeUInt21_bit2(FI_VOCAB_INDEX_NULL);
		return;
	}

	// Use literal rules (C.14.3).

	// Write literal-character-string discriminant (C.14.3: 0).
	// Write add-to-table (C.14.3.1: 0). (Adding currently not supported.)
	encoder.writeBits(2, FI_BITS(0, 0,,,,,,));

	// Write character-string using C.19 (C.14.3.2).
	assert(value_count > 0); // non-empty

	// Selecting the encoding based on value type.
	// FIXME: Support other value types (encoding algorithms).
	assert(value_type == FI_VALUE_AS_OCTETS);
	EncodingFormat format = ENCODE_AS_UTF8;

	// Write discriminant (C.19.3).
	encoder.writeBits(2, format);

	switch (format) {
	case ENCODE_AS_UTF8:
		break;

	case ENCODE_AS_UTF16:
		// TODO: We don't use the utf-16 alternative.
		throw UnsupportedOperationException ();

	case ENCODE_WITH_ALPHABET:
	case ENCODE_WITH_ALGORITHM:
		// TODO: We don't use the restricted-alphabet alternative.
		// TODO: We don't support encoding algorithms yet.
		// Write restricted-alphabet index using C.29 (C.19.3.3).
		// Write encoding-algorithm index using C.29 (C.19.3.4).
		throw UnsupportedOperationException ();
	}

	// Encode octets.
	// TODO: Move this into a separate method, for modularity.
	// TODO: Assert FI_UINT_TO_PINT(len) <= FI_PINT32_MAX?
	FI_Length len = value_count;

	// Write encoded string length using C.23.3 (C.19.4).
	encoder.writeNonEmptyOctets_len_bit5(FI_UINT_TO_PINT(len));

	FI_Octet *w_buf = encoder.getWriteBuffer(len);

	memcpy(w_buf, value_buf, len);
}

// FIXME: Value is left in an inconsistent state during deserialization.
bool
Value::read_bit1(Decoder& decoder)
{
	const FI_Octet *r_buf;

	FI_UInt21 idx;

	FI_PInt8 encoding_idx;
	FI_PInt32 octets_len;

reparse:
	switch (sub_step) {
	case 0:
		assert(decoder.getBitOffset() == 0); // C.14.2

		// Examine discriminator bits.
		if (!decoder.readBits(1)) {
			return false;
		}

		if (!(decoder.getBits() & FI_BIT_1)) {
			// Use literal rules (C.14.3).

			// Read add-to-table (C.14.3.1).
			decoder.readBits(1);

			add_value_to_table = decoder.getBits() & FI_BIT_2;

			sub_step = 2;
			goto reparse;
		}

		sub_step = 1;
		// FALLTHROUGH

	case 1:
		// Read string-index using C.26 (C.14.4).
		if (!decoder.readUInt21_bit2(idx)) {
			return false;
		}

		// XXX: Throws IndexOutOfBoundsException on bogus index.
		*this = *(*vocab_table)[idx];
		break;

	case 2:
		// Read character-string using C.19 (C.14.3.2).

		// Examine discriminator bits.
		decoder.readBits(2);

		switch (decoder.getBits() & FI_BITS(,,1,1,,,,)) {
		case ENCODE_AS_UTF8:
			// Use UTF-8 decoding rules.
			// TODO: Need a UTF-8-specific value type.
			next_value_type = FI_VALUE_AS_OCTETS;
			break;

		case ENCODE_AS_UTF16:
			// TODO: We don't use the utf-16 alternative.
			throw UnsupportedOperationException ();

		case ENCODE_WITH_ALPHABET:
			sub_step = 5;
			goto reparse;

		case ENCODE_WITH_ALGORITHM:
			sub_step = 6;
			goto reparse;
		}

		sub_step = 3;
		// FALLTHROUGH

	case 3:
		// Read encoded string length using C.23.3 (C.19.4).
		if (!decoder.readNonEmptyOctets_len_bit5(octets_len)) {
			return false;
		}

		// XXX: FI_PINT_TO_UINT() can't overflow, because we already
		// checked in readNonEmptyOctets_len_bit5().
		saved_len = FI_PINT_TO_UINT(octets_len);

		sub_step = 4;
		// FALLTHROUGH

	case 4:
		// Read encoded string octets.
		r_buf = decoder.getReadBuffer(saved_len);
		if (!r_buf) {
			return false;
		}

		// Decode octets.
		// TODO: Move this into a separate method, for modularity.
		// FIXME: Support other value types (encoding algorithms).
		assert(next_value_type == FI_VALUE_AS_OCTETS);
		if (!setValue(FI_VALUE_AS_OCTETS, saved_len, r_buf)) {
			// Couldn't set the attribute value.
			// FIXME: This exception makes no sense.
			throw UnsupportedOperationException ();
		}
		break;

	case 5:
		// Read restricted-alphabet index using C.29 (C.19.3.3).
		if (!decoder.readPInt8(encoding_idx)) {
			return false;
		}

		// Determine future value type.
		// TODO: We don't support the restricted-alphabet alternative.
		throw UnsupportedOperationException ();

	case 6:
		// Read encoding-algorithm index using C.29 (C.19.3.4).
		if (!decoder.readPInt8(encoding_idx)) {
			return false;
		}

		// Determine future value type.
		// FIXME: We don't support the encoding-algorithm alternative.
		throw UnsupportedOperationException ();

	default:
		// Should never happen.
		throw AssertionFailureException ();
	}

	sub_step = 0;
	return true;
}

} // namespace FI
} // namespace BTech


/*
 * C interface.
 */

using namespace BTech::FI;

FI_Value *
fi_create_value(void)
{
	try {
		return new FI_Value ();
	} catch (const std::bad_alloc& e) {
		return 0;
	}
}

void
fi_destroy_value(FI_Value *obj)
{
	delete obj;
}

FI_ValueType
fi_get_value_type(const FI_Value *obj)
{
	return obj->getType();
}

size_t
fi_get_value_count(const FI_Value *obj)
{
	return obj->getCount();
}

const void *
fi_get_value(const FI_Value *obj)
{
	return obj->getValue();
}

int
fi_set_value(FI_Value *obj, FI_ValueType type, size_t count, const void *buf)
{
	return obj->setValue(type, count, buf);
}
