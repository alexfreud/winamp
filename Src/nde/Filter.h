/* ---------------------------------------------------------------------------
 Nullsoft Database Engine
 --------------------
 codename: Near Death Experience
 --------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 
 Filter Class Prototypes
 
 --------------------------------------------------------------------------- */

#ifndef __FILTER_H
#define __FILTER_H

#include "LinkedList.h"
class Field;
class Filter : public LinkedListEntry
{
private:
	Field* DataField;
	unsigned char Op;
	unsigned char Id;
	
public:
	unsigned char GetOp(void) const;
	void SetOp(unsigned char Op);
	Field *Data(void) const;
	int GetId(void) const;
	Filter(unsigned char _Op);
	Filter(Field *Data, unsigned char Id, unsigned char Op);
	void SetData(Field *data);
	~Filter() ;
	
};

#endif
