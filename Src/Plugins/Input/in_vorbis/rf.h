#ifndef _RF_H_
#define _RF_H_

//based on Tempura specs.
//NOT compatible with WA3 alphas

class WReader 
{
  protected:

	/* WReader
	** WReader constructor
	*/
	WReader() : m_player(0) { }
  
  public:

	/* m_player
	** Filled by Winamp. Pointer to Winamp 3 core interface
	*/
	/*WPlayer_callback*/ void *m_player;	//PP: hack. read_file.dll doesn't call it at all. simply don't touch it

	/* GetDescription
	** Retrieves your plug-in's text description
	*/
	virtual char *GetDescription() { return "Unknown"; };

	/* Open
	** Used to open a file, return 0 on success
	*/
	virtual int Open(char *url, bool *killswitch)=0; 

	/* Read
	** Returns number of BYTES read (if < length then eof or killswitch)
	*/
	virtual int Read(char *buffer, int length, bool *killswitch)=0; 
																
	/* GetLength
	** Returns length of the entire file in BYTES, return -1 on unknown/infinite (as for a stream)
	*/
	virtual int GetLength(void)=0; 
	
	/* CanSeek
	** Returns 1 if you can skip ahead in the file, 0 if not
	*/
	virtual int CanSeek(void)=0;	//PP: currently available read_file.dll vesions can always seek in any direction
	
	/* Seek
	** Jump to a certain absolute position
	*/
	virtual int Seek(int position, bool *killswitch)=0;
	
	/* GetHeader
	** Retrieve header. Used in read_http to retrieve the HTTP header
	*/
	virtual char *GetHeader(char *name) { return 0; }

	/* ~WReader
	** WReader virtual destructor
	*/
	//virtual ~WReader() { }
	virtual void Release(int) {};
	//PP: hack - shut up linker when getting rid of evil CRT library; seems to work OK under Tempura
};




#define READ_VER	0x100

typedef struct 
{
	/* version
	** Version revision number
	*/
	int version;
	
	/* description
	** Text description of the reader plug-in
	*/
	char *description;
	
	/* create
	** Function pointer to create a reader module
	*/
	WReader *(*create)();
	
	/* ismine
	** Determines whether or not a file should be read by this plug-in
	*/
	int (*ismine)(char *url);
	
} reader_source;

//exported symbol is:
//int readerSource(HINSTANCE,reader_source**);

/*
(not a part of Tempura specs)
int _stdcall gzip_writefile(char* path,void* buf,DWORD size) - writes a memory block to a GZIP file - in_midi calls it from file info box

other hacks:
recent versions understand file://... urls, can do partial file access (eg. "partial://00006666-66660000:c:\foo\bar.dat\zzz.wav" (zzz.wav is the "display name" + extension to make winamp select correct plug-in) and auto-detect CD drive letter (eg. #:\x.mp3 will scan all drives for that file; also works with partial:// )
you can (for an example) build a playlist which will play Unreal soundtrack directly from the game CD on any system
latest read_file.dll is bundled with the midi plug-in: http://www.blorp.com/~peter/zips/in_midi.zip
*/

#endif