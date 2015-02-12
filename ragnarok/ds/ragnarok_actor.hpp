#ifndef RAGNAROK_ACTOR_HPP
#define RAGNAROK_ACTOR_HPP

#include <tchar.h>
#include "enum.h"

////////////////////////////////////////////////////////////////////////////////


enum ACTOR
{
	ACTOR_SID,
	ACTOR_CID,
	ACTOR_BASEEXP,
	ACTOR_BASENEXT,
	ACTOR_JOBEXP,
	ACTOR_JOBNEXT,
	ACTOR_CURHP,
	ACTOR_MAXHP,
	ACTOR_CURSP,
	ACTOR_MAXSP,
	ACTOR_ZENY,
	ACTOR_CLASS,
	ACTOR_BASELV,
	ACTOR_JOBLV,

	ENUM_ACTOR
};

////////////////////////////////////////////////////////////////////////////////


class RoActor
{

private:

	unsigned long _value[ENUM_ACTOR];
	TCHAR _name[LN_NAME_CHAR];

public:

	RoActor();
	~RoActor();

	void setName(const TCHAR* name) { ::lstrcpy(name, _name); }
	void getName(TCHAR* buf) { ::CopyMemory(buf, _name, LN_NAME_CHAR); }
	void setValue(enum ACTOR order, unsigned long val) { _value[order] = val; }
	unsigned long getActorVal(enum ACTOR order) const { return _value[order]; }
};

#endif
