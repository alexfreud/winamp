#ifndef NULLSOT_LOCALMEDIA_DB_H
#define NULLSOT_LOCALMEDIA_DB_H

#define MAINTABLE_ID_FILENAME		0
#define MAINTABLE_ID_TITLE			1
#define MAINTABLE_ID_ARTIST			2
#define MAINTABLE_ID_ALBUM			3
#define MAINTABLE_ID_YEAR			4
#define MAINTABLE_ID_GENRE			5
#define MAINTABLE_ID_COMMENT			6
#define MAINTABLE_ID_TRACKNB			7
#define MAINTABLE_ID_LENGTH			8  //in seconds
#define MAINTABLE_ID_TYPE			9 //0=audio, 1=video
#define MAINTABLE_ID_LASTUPDTIME		10 // last time (seconds since 1970) of db update of this item
#define MAINTABLE_ID_LASTPLAY		11 // last time (seconds since 1970) of last play
#define MAINTABLE_ID_RATING			12
#define MAINTABLE_ID_GRACENOTE_ID	14
#define MAINTABLE_ID_PLAYCOUNT		15 // play count
#define MAINTABLE_ID_FILETIME		16 // file time
#define MAINTABLE_ID_FILESIZE		17 // file size, kilobytes
#define MAINTABLE_ID_BITRATE			18 // file bitratea, kbps

#include "../nde/nde.h"
#include <map>
#include <string>

// DataBase manipulations
class DB
{
// construcotrs
public: 
	DB();
	~DB();

// methods
public:
	int Open();
	int Close();
	int Nuke();
	int AddColumn(char* metaKey, int type); // returns index of the new column or -1 on error
private:
	BOOL Discover(void);
	void ClearMap(void);


// properties
public:
	void SetTableDir(const char* tableDir);
	const char* GetTableDir();
	int GetColumnsCount();
	int GetColumnId(char *metaKey); // returns index of the column or -1 if can't find

// fields
private:
	char * tableDir;
	Database db;
	Table	*table;
	Scanner *sc;

	std::map< std::string, int> columnsMap; 

};

#endif //NULLSOT_LOCALMEDIA_DB_H