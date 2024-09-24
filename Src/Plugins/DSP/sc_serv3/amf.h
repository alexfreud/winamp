#ifndef amf_H_
#define amf_H_

#include <vector>
#include <map>
#include <stdexcept>
#include "unicode/uniString.h"

enum AMF0Marker_t
{
	AMF0_number_marker = 0x00,
	AMF0_boolean_marker,
	AMF0_string_marker,
	AMF0_object_marker,
	AMF0_movieclip_marker,
	AMF0_null_marker,
	AMF0_undefined_marker,
	AMF0_reference_marker,
	AMF0_ecma_array_marker,
	AMF0_object_end_marker,
	AMF0_strict_array_marker,
	AMF0_date_marker,
	AMF0_long_string_marker,
	AMF0_unsupported_marker,
	AMF0_recordset_marker,
	AMF0_xml_Document_marker,
	AMF0_typed_object_marker,
	AMF0_amvplus_object_marker
};

enum AMF3Marker_t
{
	AMF3_undefined_marker = 0x00,
	AMF3_null_marker,
	AMF3_false_marker,
	AMF3_true_marker,
	AMF3_integer_marker,
	AMF3_double_marker,
	AMF3_string_marker,
	AMF3_xml_doc_marker,
	AMF3_date_marker,
	AMF3_array_marker,
	AMF3_object_marker,
	AMF3_xml_marker,
	AMF3_byte_array_marker,
};

struct AMFStrictArray
{
};

struct AMFDate
{
};

class AMFVal;

struct AMFEMCAArray
{
private:
	typedef std::map<uniString::utf8,AMFVal*> propertyMap_t;
	std::map<uniString::utf8,AMFVal*> m_properties;

public:
	AMFEMCAArray() throw();
	~AMFEMCAArray() throw();
	AMFEMCAArray(const AMFEMCAArray &obj) throw();
	AMFEMCAArray& operator=(const AMFEMCAArray &obj) throw();

	void loadFromBitstream(const char *&bitstream,int &bitstreamLen,int mode,const uniString::utf8 &logMsgPrefix) throw(std::exception);
	const AMFVal* getProperty(const uniString::utf8 &key) const throw();
	void addProperty(const uniString::utf8 &key,AMFVal *v) throw(std::exception); // takes possession of "v"
	void clearProperties() throw();
	void serialize(std::vector<__uint8> &s,int mode,const uniString::utf8 &logMsgPrefix) const throw(std::exception);
	uniString::utf8 prettyPrint(int mode,const uniString::utf8 &tabs) const throw();
};

class AMFObject
{
private:
	typedef std::map<uniString::utf8,AMFVal*> propertyMap_t;
	std::map<uniString::utf8,AMFVal*> m_properties;

public:
	AMFObject() throw();
	~AMFObject() throw();
	AMFObject(const AMFObject &obj) throw();
	AMFObject& operator=(const AMFObject &obj) throw();

	void loadFromBitstream(const char *&bitstream,int &bitstreamLen,int mode,const uniString::utf8 &logMsgPrefix) throw(std::exception);
	const AMFVal* getProperty(const uniString::utf8 &key) const throw();
	void addProperty(const uniString::utf8 &key,AMFVal *v) throw(std::exception); // takes possession of "v"
	void clearProperties() throw();
	void serialize(std::vector<__uint8> &s,int mode,const uniString::utf8 &logMsgPrefix) const throw(std::exception);
	uniString::utf8 prettyPrint(int mode,const uniString::utf8 &tabs) const throw();
};

class AMFVal
{
private:
	AMF0Marker_t	m_type0;		// type
	AMF3Marker_t	m_type3;

	// value depends on type
	int				m_integer_val;
	double			m_number_val;
	bool			m_boolean_val;
	uniString::utf8	m_string_val;
	AMFObject		m_object_val;
	__uint16		m_reference_val;
	AMFEMCAArray	m_ecma_array_val;	
	AMFStrictArray	m_strict_array_val;
	AMFDate			m_date_val;

public:
	void loadFromBitstream(const char *&bitstream,int &bitstreamLen,int mode,const uniString::utf8 &logMsgPrefix) throw(std::exception);

	const uniString::utf8 &getString() const throw(std::exception);
	double			getNumber() const throw(std::exception);
	bool			getBool() const throw(std::exception);
	const AMFObject& getObject() const throw(std::exception);
	int				getInteger() const throw(std::exception);

	AMFVal()						 throw(): m_type0(AMF0_null_marker),m_type3(AMF3_undefined_marker){}
	AMFVal(double v)				 throw(): m_type0(AMF0_number_marker),m_type3(AMF3_undefined_marker) ,m_number_val(v){}
	AMFVal(bool v)					 throw(): m_type0(AMF0_boolean_marker),m_type3(AMF3_undefined_marker),m_boolean_val(v){}
	AMFVal(const uniString::utf8 &v) throw(): m_type0(AMF0_string_marker),m_type3(AMF3_undefined_marker) ,m_string_val(v){}
	AMFVal(const AMFObject &v)		 throw(): m_type0(AMF0_object_marker),m_type3(AMF3_undefined_marker) ,m_object_val(v){}
	AMFVal(const AMFEMCAArray &v)	 throw(): m_type0(AMF0_ecma_array_marker),m_type3(AMF3_undefined_marker),m_ecma_array_val(v){}

	void serialize(std::vector<__uint8> &s,int mode,const uniString::utf8 &logMsgPrefix) const throw(std::exception);
	uniString::utf8 prettyPrint(int mode,const uniString::utf8 &tabs) const throw();
};
	
class AMFEncoding
{
private:
	std::vector<AMFVal> m_values;
	int m_mode;

public:
	AMFEncoding(int mode = 0) throw():m_mode(mode){}
	~AMFEncoding() throw(){}

	void clear() throw() { m_values.clear(); }

	void loadFromBitstream(const char *bitstream,int bitstreamLen,const uniString::utf8 &logMsgPrefix) throw(std::exception);	
	const AMFVal& getValue(size_t index) const throw(std::exception);

	void appendValue(const AMFVal &v) throw();
	void serialize(std::vector<__uint8> &s,const uniString::utf8 &logMsgPrefix) const throw(std::exception);

	uniString::utf8 prettyPrint() const throw();
};

#endif
