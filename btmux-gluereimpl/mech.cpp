#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "externs.h"
#include "timeutil.h"
#include "svdhash.h"

#include "btech.h"
#include "mech.h"


MECH::MECH(dbref object) {
	m_dbref = object;
}

MECH::MECH(void) {

}

MECH::~MECH(void) {

}

void MECH::load() {

}

void MECH::save() {

}

void MECH::update() {

}

int MECH::size() {
    return sizeof(this);
}

int MECH::HandledCommand(dbref executor, char *pCommand) {
	if(!string_match(pCommand,"status")) {
		Status(executor);
		return 1;
	} else
		return 0;
}

void MECH::Status(dbref executor) {
	notify(executor, "Here's the status.");
}