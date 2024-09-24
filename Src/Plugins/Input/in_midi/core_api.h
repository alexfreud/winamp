#ifndef __CORE_API_H
#define __CORE_API_H

//ancient tempura header. yay.

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(x) if (!(x)) MessageBox(NULL,"ASSERT FAILED: " #x,"ASSERT FAILED in " __FILE__ ,MB_OK|MB_ICONSTOP);
#else
#define ASSERT(x)
#endif
#endif

class WReader;

class WPlayer_callback
{
  public:
    virtual WReader *GetReader(char *url)=0;
	virtual void Error(char *reason)=0;
	virtual void Warning(char *warning)=0;
	virtual void Status(char *status)=0;
	virtual void TitleChange(char *new_title)=0;
	virtual void InfoChange(char *new_info_str, int new_length)=0;
	virtual void UrlChange(char *new_url)=0;
};

class WInfo_callback
{
  public:
    virtual WReader *GetReader(char *url)=0;
};

class WReader 
{
  protected:
	WReader() : m_player(0) { }
  public:
	WPlayer_callback *m_player;
	virtual char *GetDescription() { return 0; };
	virtual int Open(char *url, char *killswitch)=0; 
	virtual int Read(char *buffer, int length, char *killswitch)=0; 
	virtual int GetLength(void)=0; 
	virtual int CanSeek(void)=0;
	virtual int Seek(int position, char *killswitch)=0;
	virtual char *GetHeader(char *name) { return 0; }
	virtual ~WReader() { }
};

#define READ_VER	0x100
#define OF_VER		0x100

typedef struct 
{
	int version;
	char *description;
	WReader *(*create)();
	int (*ismine)(char *url);
} reader_source;

#endif