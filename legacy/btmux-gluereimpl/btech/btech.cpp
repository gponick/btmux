#include <string>

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "externs.h"

#include "btech.h"

BTech::BTech(void) {

}

BTech::~BTech(void) {

}

void BTech::load() {

}

void BTech::save() {

}

void BTech::update() {
    // for item in specialobjectlist { item.update() }
}

int BTech::size() {
    return sizeof(this);
}

void BTech::Init(dbref object) {
	m_dbref = object;
}

void BTech::LoadSpecialObjects() {

}

void BTech::LoadEconDatabase() {

}

void BTech::InitTimer() {

}

void BTech::SaveSpecialObjects() {

}

void BTech::Shutdown() {

}

void BTech::CheckSpecialObjects() {

}

void BTech::CheckEconDatabase() {

}

int BTech::Is_SpecialObject(dbref object) {
	xcodemap::iterator iter;

	iter = m_mSpecialObjects.find(object);
	if(iter != m_mSpecialObjects.end())
		return 1;

	return 0;
}

int BTech::HandledCommand(dbref executor, char *pCommand) {
	if(!string_compare(pCommand,"HELP")) {
		notify(executor, "here is the requested help.");
		xcodemap::iterator iter;
		notify(executor, tprintf("SpecialObjectMap.size(): %d", m_mSpecialObjects.size()));
		for(iter=m_mSpecialObjects.begin();iter!=m_mSpecialObjects.end();iter++) {
			MAP *xc = static_cast <MAP *> (iter->second);
			notify(executor, tprintf("dbref: #%d", xc->m_dbref));
		}
		return 1;
	} else {
		xcodemap::iterator iter;
		MAP *map;

		iter = m_mSpecialObjects.find(Location(executor));
		if(iter != m_mSpecialObjects.end()) {
			map = static_cast <MAP *>(iter->second);
			if(map->HandledCommand(executor, pCommand))
				return 1;
			else {
				iter = m_mSpecialObjects.find(executor);
				if(iter != m_mSpecialObjects.end()) {
					map = static_cast <MAP *>(iter->second);
					if(map->HandledCommand(executor, pCommand))
						return 1;
				}
			}
		} 
	}
	return 0;
}

void BTech::NewSpecialObject(dbref object) {
	MAP *xc = new MAP(object, 10,10);
	m_mSpecialObjects.insert(std::pair<dbref, MAP *>(object, xc));
	notify(1, "caught marker9 set");
}

void BTech::DisposeSpecialObject(dbref object) {
	xcodemap::iterator iter;

	iter = m_mSpecialObjects.find(object);
	if(iter != m_mSpecialObjects.end())
		m_mSpecialObjects.erase(iter);

	notify(1, "caught marker9 unset");
}
