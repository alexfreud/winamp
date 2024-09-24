/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 IndexField Class Prototypes

--------------------------------------------------------------------------- */

#ifndef __INDEXFIELD_H
#define __INDEXFIELD_H

class IndexField : public Field
{
public:
	IndexField(unsigned char id, int Pos, int type, const wchar_t *FieldName);
	IndexField();
	~IndexField();
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	virtual int Compare(Field *Entry);
	void InitField(void);

	wchar_t *GetIndexName(void);
	int TranslateToIndex(int Id, IndexField *index);
	Index *index; // TODO: make protected	
protected:
	int Position;
	int DataType;
	wchar_t *Name;
};

#endif