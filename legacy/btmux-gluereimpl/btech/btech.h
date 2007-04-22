#ifndef BTECH_H
#define BTECH_H

#include <map>

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "externs.h"

#include "svdhash.h"

class XCode 
{
public:
	int m_dbref;
	static int instancecount;

	XCode(dbref object);
	XCode(void);
	~XCode();

	virtual void load() = 0;
	virtual void save() = 0;
	virtual void update() = 0;
	virtual int size();

	std::string getattr(char *attrname);
	int getattrn(char *attrname);
	bool setattr(char *attrname, std::string attrtext);
	bool setattr(char *attrname, int attrnum);
	bool setattr(char *attrname, float attrnum);

	void Emit(char *message);
};

typedef std::map<dbref, XCode *> xcodemap;
typedef std::map<std::string, FUN *> funcmap;

class BTech: public XCode {
public:
	xcodemap m_mSpecialObjects;
	funcmap m_mFunc;

	BTech(void);
	~BTech(void);

	void load();
	void save();
	void update();
	int size();

	void Init(dbref);
	void LoadSpecialObjects();
	void LoadEconDatabase();
	void InitTimer();
	void SaveSpecialObjects();
	void Shutdown();
	void CheckSpecialObjects();
	void CheckEconDatabase();

	int Is_SpecialObject(dbref object);
	int HandledCommand(dbref executor, char *pCommand);
	void NewSpecialObject(dbref object);
	void DisposeSpecialObject(dbref object);
};

class MECH;		// defined below

class MAP: public XCode {
public:
	dbref mechsOnMap[100];				// temporary

	MAP(dbref object, int x, int y);
	MAP(void);
	~MAP(void); 

	void load();
	void save();
	void update();
	int size();

	int HandledCommand(dbref executor, char *pCommand); 

	void View(dbref executor, int x, int y);

	// Map rendering methods
	inline int Is_OddCol(int col)  { return (unsigned) col & 1; }
	inline int TACDispCols(int hexcols)  { return hexcols * 3 + 1; }
	inline int TACHexOffset(int x, int y, int dispcols, int oddcol1)  
			{  return (y * 2 + 1 - Is_OddCol(x + oddcol1)) * dispcols + x * 3 + 1; }
	inline void TACSketchRow(char *pos, int left_offset, char const *src, int len);
	void TACSketchMap(char *buf, MECH * mech, int sx, int sy, int wx, int wy, int dispcols, 
			int top_offset, int left_offset, int docolour, int dohexlos);
	void TACSketchDS(char *base, int dispcols, char terr);
	void TACSketchOwnMech(char *buf, MECH * mech, int sx, int sy, int wx, int wy, int dispcols, 
			int top_offset, int left_offset);
	void TACSketchMechs(char *buf, MECH * player_mech, int sx, int sy, int wx, int wy, int dispcols, 
			int top_offset, int left_offset, int docolor, int labels);
	void TACSketchCliffs(char *buf, int sx, int sy, int wx, int wy, int dispcols, 
			int top_offset, int left_offset, int cliff_size);
	void TACSketchDSLZ(char *buf, MECH * mech, int sx, int sy, int wx, int wy, int dispcols, 
			int top_offset, int left_offset, int cliff_size, int docolor);
	char **TACColorizeMap(char const *sketch, int dispcols, int disprows);
	char **MakeMapText(dbref player, MECH * mech, int cx, int cy, int labels, int dohexlos);

	// map lookup functions
	int GetIndex(char terrain, char elevation);
	char GetElevation(int index);
	char GetElevation(int x, int y);
	char GetTerrain(int index);
	char GetTerrain(int x, int y);
	inline char TerrainColorChar(char terrain, int elev);
	char *add_color(char newc, char * prevc, char c);
};

class Unit {
public:
	bool m_bStarted;	
	int m_iWeight;
	int m_iSpeed;
};

class MECH: public XCode, public Unit {
public:
	MECH(dbref object);
	MECH(void);
	~MECH(void);

	void load();
	void save();
	void update();
	int size();

	int HandledCommand(dbref executor, char *pCommand);

	void Status(dbref executor);
};

#endif