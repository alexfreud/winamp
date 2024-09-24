#ifndef _XMLPARSE_H
#define _XMLPARSE_H

class XMLParse {
private:
	void *parser;

public:
  XMLParse();
  virtual ~XMLParse();

	virtual void SetUserData(void *param);
	virtual void SetElementHandler(void (*start)(void *userData, const wchar_t *name, const wchar_t **atts),
																 void (*end)(void *userData, const wchar_t *name));
	virtual void SetCharacterDataHandler(void (*handler)(void *userData,const wchar_t *s, int len));
	virtual int Parse(const wchar_t *s, int len, int isFinal);
	virtual const wchar_t *ErrorString(int code);
	virtual int GetErrorCode();
	virtual int GetCurrentLineNumber();

};

#endif
