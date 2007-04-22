#include <string>

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "externs.h"
#include "timeutil.h"
#include "svdhash.h"

#include "btech.h"

int XCode::instancecount = 0;

extern class BTech btech;

XCode::XCode(int object)
{
	CLinearTimeAbsolute ltaNow;
	ltaNow.GetLocal();

	m_dbref = object;

	dbref aowner;
	int aflags, atr = mkattr(m_dbref, "xcStarted");
	atr_pget_info(m_dbref, atr, &aowner, &aflags);
	atr_add(m_dbref, atr, ltaNow.ReturnDateString(7), GOD, aflags);

	// then add this item to the global list
	instancecount++;
}

XCode::XCode(void)
{
	instancecount++;
}

XCode::~XCode()
{
	instancecount--;
}

int XCode::size()
{
	return sizeof(this);
}

bool XCode::setattr(char *attrname, std::string attrtext)
{
	int atr = mkattr(m_dbref, attrname);
	if(!atr) 
		return false;

	dbref aowner;
	int aflags;
	atr_pget_info(m_dbref, atr, &aowner, &aflags);
	atr_add(m_dbref, atr, const_cast <char *> (attrtext.c_str()), GOD, aflags);

	return true;
}

bool XCode::setattr(char *attrname, int attrnum)
{
	int atr = mkattr(m_dbref, attrname);
	if(!atr) 
		return false;

	dbref aowner;
	int aflags;
	atr_pget_info(m_dbref, atr, &aowner, &aflags);
	atr_add(m_dbref, atr, tprintf("%d", attrnum), GOD, aflags);

	return true;
}

bool XCode::setattr(char *attrname, float attrnum)
{
	int atr = mkattr(m_dbref, attrname);
	if(!atr) 
		return false;

	dbref aowner;
	int aflags;
	atr_pget_info(m_dbref, atr, &aowner, &aflags);
	atr_add(m_dbref, atr, tprintf("%0.2f", attrnum), GOD, aflags);

	return true;
}

std::string XCode::getattr(char *attrname)
{
	dbref aowner;
	int aflags;
	char *buf;
	ATTR *pattr;

	if((pattr = atr_str(attrname)) && pattr->number)
		buf = atr_get(m_dbref, pattr->number, &aowner, &aflags);
	else
		return NULL;

	if(!buf) 
		return NULL;
	else
		return buf;
}

int XCode::getattrn(char *attrname)
{
	dbref aowner;
	int aflags;
	char *buf;
	ATTR *pattr;
	int result;

	if((pattr = atr_str(attrname)) && pattr->number)
		buf = atr_get(m_dbref, pattr->number, &aowner, &aflags);
	else
		return NULL;

	if(!buf) 
		return NULL;
	else {
		sscanf(buf, "%d", &result);
		return result;
	}
}

void XCode::Emit(char *message)
{
	notify_all_from_inside(m_dbref, GOD, message);
}
