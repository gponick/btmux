#ifndef XCODE_H
#define XCODE_H

//#include <list>

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "externs.h"

#include "btech.h"

class xcode 
{
public:
	int m_dbref;
	static int count;

	xcode(dbref object);
	~xcode();

	virtual void load();
	virtual void save();
	virtual void update();
	virtual int size();

	xcode *Find(dbref object);

	char *getattr(char *attrname);
	bool setattr(char *attrname, char *attrtext);
};

#endif