#ifndef filenameMetadata_H_
#define filenameMetadata_H_

#include "unicode/uniString.h"
#include <stdexcept>
#include <map>

class filenameMetadata
{
	class impl;
	impl *m_impl;

public:
	filenameMetadata();
	~filenameMetadata() throw();

	void setPattern(const uniString::utf8 &pattern) throw(std::exception);
	void parse(const uniString::utf8 &data) throw(std::exception);
	void deleteToken(const uniString::utf8 &token) throw(std::exception);
	const size_t countTokens() throw(std::exception);
	uniString::utf8& operator[](const uniString::utf8 &key) throw(std::exception);
	const std::map<uniString::utf8,uniString::utf8>& getTokens() const throw(std::exception);
};
	
#endif
