#pragma once


// TODO: benski> change this to use a smarter data structure. 
// this initial implementation is known to work, however
class JNL_Headers
{
public:
	JNL_Headers();
	~JNL_Headers();

	const char *GetAllHeaders();
	const char *GetHeader( const char *header_name );
	int         Add( const char *buf );
	void        Reset();
	
private:
	char   *m_recvheaders;
	size_t  m_recvheaders_size;
};