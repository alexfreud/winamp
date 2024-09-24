#if 0
#include "amf.h"
#include "global.h"

#define DEBUG_LOG(x) { if (gOptions.RTMPClientDebug()) DLOG((x)); }

#ifndef _WIN32
#include <arpa/inet.h>
#endif

using namespace std;
using namespace stringUtil;
using namespace uniString;

#ifdef _WIN32
// Windows is little endian only
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN    4321
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __FLOAT_WORD_ORDER __BYTE_ORDER

typedef unsigned char uint8_t;

#else /* !_WIN32 */

#include <sys/param.h>

#if defined(BYTE_ORDER) && !defined(__BYTE_ORDER)
#define __BYTE_ORDER    BYTE_ORDER
#endif

#if defined(BIG_ENDIAN) && !defined(__BIG_ENDIAN)
#define __BIG_ENDIAN	BIG_ENDIAN
#endif

#if defined(LITTLE_ENDIAN) && !defined(__LITTLE_ENDIAN)
#define __LITTLE_ENDIAN	LITTLE_ENDIAN
#endif

#endif /* !_WIN32 */

// define default endianness
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN	1234
#endif

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN	4321
#endif

#ifndef __BYTE_ORDER
#warning "Byte order not defined on your system, assuming little endian!"
#define __BYTE_ORDER	__LITTLE_ENDIAN
#endif

// ok, we assume to have the same float word order and byte order if float word order is not defined
#ifndef __FLOAT_WORD_ORDER
//#warning "Float word order not defined, assuming the same as byte order!"
#define __FLOAT_WORD_ORDER	__BYTE_ORDER
#endif

#if !defined(__BYTE_ORDER) || !defined(__FLOAT_WORD_ORDER)
#error "Undefined byte or float word order!"
#endif

#if __FLOAT_WORD_ORDER != __BIG_ENDIAN && __FLOAT_WORD_ORDER != __LITTLE_ENDIAN
#error "Unknown/unsupported float word order!"
#endif

#if __BYTE_ORDER != __BIG_ENDIAN && __BYTE_ORDER != __LITTLE_ENDIAN
#error "Unknown/unsupported byte order!"
#endif

static double
AMF0_DecodeNumber(const char *data)
{
	double dVal;
#if __FLOAT_WORD_ORDER == __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
	memcpy(&dVal, data, 8);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned char *ci, *co;
	ci = (unsigned char *)data;
	co = (unsigned char *)&dVal;
	co[0] = ci[7];
	co[1] = ci[6];
	co[2] = ci[5];
	co[3] = ci[4];
	co[4] = ci[3];
	co[5] = ci[2];
	co[6] = ci[1];
	co[7] = ci[0];
#endif
#else
#if __BYTE_ORDER == __LITTLE_ENDIAN	// __FLOAT_WORD_ORER == __BIG_ENDIAN
	unsigned char *ci, *co;
	ci = (unsigned char *)data;
	co = (unsigned char *)&dVal;
	co[0] = ci[3];
	co[1] = ci[2];
	co[2] = ci[1];
	co[3] = ci[0];
	co[4] = ci[7];
	co[5] = ci[6];
	co[6] = ci[5];
	co[7] = ci[4];
#else // __BYTE_ORDER == __BIG_ENDIAN && __FLOAT_WORD_ORER == __LITTLE_ENDIAN
	unsigned char *ci, *co;
	ci = (unsigned char *)data;
	co = (unsigned char *)&dVal;
	co[0] = ci[4];
	co[1] = ci[5];
	co[2] = ci[6];
	co[3] = ci[7];
	co[4] = ci[0];
	co[5] = ci[1];
	co[6] = ci[2];
	co[7] = ci[3];
#endif
#endif
	return dVal;
}

char *
AMF0_EncodeNumber(char *output, char *outend, double dVal)
{
	if (output+8 > outend)
		return NULL;

//  *output++ = AMF_NUMBER;	// type: Number

#if __FLOAT_WORD_ORDER == __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
	memcpy(output, &dVal, 8);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	{
		unsigned char *ci, *co;
		ci = (unsigned char *)&dVal;
		co = (unsigned char *)output;
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];
	}
#endif
#else
#if __BYTE_ORDER == __LITTLE_ENDIAN	/* __FLOAT_WORD_ORER == __BIG_ENDIAN */
	{
		unsigned char *ci, *co;
		ci = (unsigned char *)&dVal;
		co = (unsigned char *)output;
		co[0] = ci[3];
		co[1] = ci[2];
		co[2] = ci[1];
		co[3] = ci[0];
		co[4] = ci[7];
		co[5] = ci[6];
		co[6] = ci[5];
		co[7] = ci[4];
	}
#else /* __BYTE_ORDER == __BIG_ENDIAN && __FLOAT_WORD_ORER == __LITTLE_ENDIAN */
	{
		unsigned char *ci, *co;
		ci = (unsigned char *)&dVal;
		co = (unsigned char *)output;
		co[0] = ci[4];
		co[1] = ci[5];
		co[2] = ci[6];
		co[3] = ci[7];
		co[4] = ci[0];
		co[5] = ci[1];
		co[6] = ci[2];
		co[7] = ci[3];
	}
#endif
#endif
	return output+8;
}

static void serialize(vector<__uint8> &s,const utf8 &v,int mode,const utf8 &logMsgPrefix) throw(exception)
{
	assert(mode == 0); // AMF3 not yet implemented
	__uint16 slen = (__uint16)v.length();
	slen = htons(slen);
	s.insert(s.end(),(const char *)&slen,((const char *)&slen) + 2);
	s.insert(s.end(),v.begin(),v.end());
}

/////////////////////////////////////////////////////////////////////////////////////////////////

AMFObject::AMFObject() throw() {}

AMFObject::~AMFObject() throw()
{
	clearProperties();
}

AMFObject::AMFObject(const AMFObject &obj) throw()
{
	for(propertyMap_t::const_iterator i = obj.m_properties.begin(); i != obj.m_properties.end(); ++i)
	{
		m_properties[(*i).first] = new AMFVal(*(*i).second);
	}
}

AMFObject& AMFObject::operator=(const AMFObject &obj) throw()
{
	clearProperties();
	for(propertyMap_t::const_iterator i = obj.m_properties.begin(); i != obj.m_properties.end(); ++i)
	{
		m_properties[(*i).first] = new AMFVal(*(*i).second);
	}	
	return *this;
}

void AMFObject::clearProperties() throw()
{	
	for(propertyMap_t::iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		delete (*i).second;
	}
	m_properties.clear();
}

// add property. Takes possession of value "v"
// throws if value already exists
void AMFObject::addProperty(const utf8 &key,AMFVal *v) throw(exception)
{
	assert(v);
	assert(m_properties.find(key) == m_properties.end());

	if (m_properties.find(key) != m_properties.end())
		throwEx<runtime_error>(string(__FUNCTION__) + " property " + key + " already exists");
	if (!v)
		throwEx<runtime_error>(string(__FUNCTION__) + " value is null.");

	m_properties[key] = v;
}

const AMFVal* AMFObject::getProperty(const uniString::utf8 &key) const throw()
{
	propertyMap_t::const_iterator i = m_properties.find(key);
	if (i == m_properties.end()) return 0;
	return (*i).second;
}

utf8 AMFObject::prettyPrint(int mode,const utf8 &tabs) const throw()
{
	utf8 result;
	result += tabs + "{" + eol();

	for(propertyMap_t::const_iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		assert((*i).second);
		result += tabs + "\t" + (*i).first + ": ";
		result += (*i).second->prettyPrint(mode,tabs + "\t");
		result += eol();
	}

	result += tabs + "}";
	return result;
}

void AMFObject::serialize(vector<__uint8> &s,int mode,const utf8 &logMsgPrefix) const throw(exception)
{
	assert(mode == 0); // AMF3 not yet implemented
	for(propertyMap_t::const_iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		assert((*i).second);
		::serialize(s,(*i).first,mode,logMsgPrefix);
		(*i).second->serialize(s,mode,logMsgPrefix);
	}
	s.push_back(0);
	s.push_back(0);
	s.push_back(AMF0_object_end_marker);	
}

void AMFObject::loadFromBitstream(const char *&bitstream,int &bitstreamLen,int mode,const utf8 &logMsgPrefix) throw(exception)
{
	assert(mode == 0); // AMF3 not implemented yet
	propertyMap_t pmap;

	AMFVal *val = 0;
	try
	{
		while(true)
		{
			if (bitstreamLen < 3) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 object marker.");
			if (bitstream[0] == 0 && bitstream[1] == 0 && bitstream[2] == 9)
			{
				// end of object
				bitstream += 3;
				bitstreamLen -= 3;
				break;
			}
			if (bitstream[0] == 0 && bitstream[1] == 0) throwEx<runtime_error>(logMsgPrefix + " AMF0 object has null string keyed property");
			if (bitstreamLen < 4) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 object marker.");

			// alright, we've taken care of the abberant cases and end-of-object. Now let's get a property
			int slen = ntohs(*(__uint16*)bitstream);
			bitstream += 2;
			bitstreamLen -= 2;
			if (bitstreamLen < slen) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 object property key.");
			utf8 key(bitstream,bitstream + slen);
			bitstream += slen;
			bitstreamLen -= slen;
			assert(!val);
			val = new AMFVal;
			val->loadFromBitstream(bitstream,bitstreamLen,mode,logMsgPrefix);
			pmap[key] = val;
			val = 0;
		}

		clearProperties();
		m_properties = pmap;
	}
	catch(...)
	{
		delete val;
		throw;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

AMFEMCAArray::AMFEMCAArray() throw() {}

AMFEMCAArray::~AMFEMCAArray() throw()
{
	clearProperties();
}

AMFEMCAArray::AMFEMCAArray(const AMFEMCAArray &obj) throw()
{
	for(propertyMap_t::const_iterator i = obj.m_properties.begin(); i != obj.m_properties.end(); ++i)
	{
		m_properties[(*i).first] = new AMFVal(*(*i).second);
	}
}

AMFEMCAArray& AMFEMCAArray::operator=(const AMFEMCAArray &obj) throw()
{
	clearProperties();
	for(propertyMap_t::const_iterator i = obj.m_properties.begin(); i != obj.m_properties.end(); ++i)
	{
		m_properties[(*i).first] = new AMFVal(*(*i).second);
	}	
	return *this;
}

void AMFEMCAArray::clearProperties() throw()
{	
	for(propertyMap_t::iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		delete (*i).second;
	}
	m_properties.clear();
}

// add property. Takes possession of value "v"
// throws if value already exists
void AMFEMCAArray::addProperty(const utf8 &key,AMFVal *v) throw(exception)
{
	assert(v);
	assert(m_properties.find(key) == m_properties.end());

	if (m_properties.find(key) != m_properties.end())
		throwEx<runtime_error>(string(__FUNCTION__) + " property " + key + " already exists");
	if (!v)
		throwEx<runtime_error>(string(__FUNCTION__) + " value is null.");

	m_properties[key] = v;
}

const AMFVal* AMFEMCAArray::getProperty(const uniString::utf8 &key) const throw()
{
	propertyMap_t::const_iterator i = m_properties.find(key);
	if (i == m_properties.end()) return 0;
	return (*i).second;
}

utf8 AMFEMCAArray::prettyPrint(int mode,const utf8 &tabs) const throw()
{
	utf8 result;
	result += tabs + "{" + eol();

	for(propertyMap_t::const_iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		assert((*i).second);
		result += tabs + "\t" + (*i).first + ": ";
		result += (*i).second->prettyPrint(mode,tabs + "\t");
		result += eol();
	}

	result += tabs + "}";
	return result;
}

void AMFEMCAArray::serialize(vector<__uint8> &s,int mode,const utf8 &logMsgPrefix) const throw(exception)
{
	assert(mode == 0); // amf3 not implemented
	// wowza seems to always just use zero as array length
	s.push_back(0); s.push_back(0); s.push_back(0); s.push_back(0);

	for(propertyMap_t::const_iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		assert((*i).second);
		::serialize(s,(*i).first,mode,logMsgPrefix);
		(*i).second->serialize(s,mode,logMsgPrefix);
	}
	s.push_back(0);
	s.push_back(0);
	s.push_back(AMF0_object_end_marker);	
}

void AMFEMCAArray::loadFromBitstream(const char *&bitstream,int &bitstreamLen,int mode,const utf8 &logMsgPrefix) throw(exception)
{
	assert(mode == 0); // AMF3 not implemented yet
	propertyMap_t pmap;

	AMFVal *val = 0;
	try
	{
		if (bitstreamLen < 7) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 ECMA array type.");

		// skip length
		bitstream += 4;
		bitstreamLen -= 4;

		while(true)
		{
			if (bitstreamLen < 3) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for ECMA array type.");
			if (bitstream[0] == 0 && bitstream[1] == 0 && bitstream[2] == 9)
			{
				// end of object
				bitstream += 3;
				bitstreamLen -= 3;
				break;
			}
			if (bitstream[0] == 0 && bitstream[1] == 0) throwEx<runtime_error>(logMsgPrefix + " AMF0 ECMA array type has null string keyed property");
			if (bitstreamLen < 4) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 ECMA array type.");

			// alright, we've taken care of the abberant cases and end-of-object. Now let's get a property
			int slen = ntohs(*(__uint16*)bitstream);
			bitstream += 2;
			bitstreamLen -= 2;
			if (bitstreamLen < slen) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 ECMA array type property key.");
			utf8 key(bitstream,bitstream + slen);
			bitstream += slen;
			bitstreamLen -= slen;
			assert(!val);
			val = new AMFVal;
			val->loadFromBitstream(bitstream,bitstreamLen,mode,logMsgPrefix);
			pmap[key] = val;
			val = 0;
		}

		clearProperties();
		m_properties = pmap;
	}
	catch(...)
	{
		delete val;
		throw;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uniString::utf8 & AMFVal::getString() const throw(std::exception)	
{
	if (m_type3 == AMF3_string_marker) return m_string_val;
	if (m_type0 != AMF0_string_marker) throwEx<runtime_error>("AMFVal type error. Wanted string but type is " + tos(m_type0));
	return m_string_val;
}

int AMFVal::getInteger() const throw(std::exception)
{
	if (m_type3 != AMF3_integer_marker) throwEx<runtime_error>("AMFVal type error. Wanted integer but type is " + tos(m_type3));
	return m_integer_val;
}

double AMFVal::getNumber() const throw(std::exception)
{
	if (m_type3 == AMF3_double_marker) return m_number_val;
	if (m_type0 != AMF0_number_marker) throwEx<runtime_error>("AMFVal type error. Wanted number but type is " + tos(m_type0));
	return m_number_val;
}

bool AMFVal::getBool() const throw(std::exception)
{
	if (m_type3 == AMF3_true_marker) return true;
	if (m_type3 == AMF3_false_marker) return false;
	if (m_type0 != AMF0_boolean_marker) throwEx<runtime_error>("AMFVal type error. Wanted boolean but type is " + tos(m_type0));
	return m_boolean_val;
}

const AMFObject& AMFVal::getObject() const throw(std::exception)
{
	if (m_type3 == AMF3_object_marker) return m_object_val;
	if (m_type0 != AMF0_object_marker) throwEx<runtime_error>("AMFVal type error. Wanted object but type is " + tos(m_type0));
	return m_object_val;
}

void AMFVal::serialize(vector<__uint8> &s,int mode,const utf8 &logMsgPrefix) const throw(exception)
{
	s.push_back(mode == 3 ? (__uint8)m_type3 : (__uint8)m_type0);
	if (mode == 3)
	{
		switch(m_type3)
		{
			case AMF3_double_marker:
			{
				__uint8 buf[8] = {0};
				AMF0_EncodeNumber((char *)buf,(char *)buf+8,m_number_val);
				s.insert(s.end(),buf,buf+8);
			}
			break;

			default:
				throwEx<runtime_error>(logMsgPrefix + __FUNCTION__ + " unsupported type " + tos(m_type3));
		}
	}
	else
	{
		switch(m_type0)
		{
			case AMF0_number_marker:
			{
				__uint8 buf[8] = {0};
				AMF0_EncodeNumber((char *)buf,(char *)buf+8,m_number_val);
				s.insert(s.end(),buf,buf+8);
			}
			break;

			case AMF0_boolean_marker:
				s.push_back(m_boolean_val ? 1 : 0);
			break;

			case AMF0_string_marker:
				::serialize(s,m_string_val,mode,logMsgPrefix);
			break;

			case AMF0_object_marker:
				m_object_val.serialize(s,mode,logMsgPrefix);
			break;

			case AMF0_ecma_array_marker:
				m_ecma_array_val.serialize(s,mode,logMsgPrefix);
			break;

			case AMF0_null_marker:
			break;

			default:
				throwEx<runtime_error>(logMsgPrefix + __FUNCTION__ + " unsupported type " + tos(m_type0));
			break;
		}
	}
}

utf8 AMFVal::prettyPrint(int mode,const utf8 &tabs) const throw()
{
	if (mode == 3)
	{
		switch(m_type3)
		{
			case AMF3_double_marker:
			return tabs + tos(m_number_val);

			case AMF3_integer_marker:
			return tabs + tos(m_integer_val);

			default:
			break;
		}
	}
	else
	{
		switch(m_type0)
		{
			case AMF0_number_marker:
			return tabs + tos(m_number_val);

			case AMF0_boolean_marker:
			return tabs + (m_boolean_val ? "true" : "false");

			case AMF0_string_marker:
			return tabs + m_string_val;

			case AMF0_object_marker:
			return m_object_val.prettyPrint(mode,tabs);

			case AMF0_ecma_array_marker:
			return m_ecma_array_val.prettyPrint(mode,tabs);

			default:
			break;
		}
	}
	return "";
}

void AMFVal::loadFromBitstream(const char *&bitstream,int &bitstreamLen,int mode,const utf8 &logMsgPrefix) throw(exception)
{
	if (!bitstreamLen) throwEx<runtime_error>(logMsgPrefix + " AMF bitstream is empty");
	if (mode == 3)
	{
		m_type3 = (AMF3Marker_t)*bitstream;
		bitstreamLen -= 1;
		bitstream += 1;
		switch(m_type3)
		{
			case AMF3_double_marker:
				if (bitstreamLen < 8) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF3 double marker.");
				m_number_val = AMF0_DecodeNumber(bitstream);
				bitstream += 8;
				bitstreamLen -= 8;
			break;

			case AMF3_null_marker:
			case AMF3_false_marker:
			case AMF3_true_marker:
			case AMF3_integer_marker:
			case AMF3_string_marker:
			case AMF3_xml_doc_marker:
			case AMF3_date_marker:
			case AMF3_array_marker:
			case AMF3_object_marker:
			case AMF3_xml_marker:
			case AMF3_byte_array_marker:
			case AMF3_undefined_marker:
				throwEx<runtime_error>(logMsgPrefix + " Unsupported AMF3 marker " + tos(m_type3));
			break;

			default:
				throwEx<runtime_error>(logMsgPrefix + " Unknown AMF3 marker " + tos(m_type3));
			break;
		}
	}
	else
	{
		m_type0 = (AMF0Marker_t)*bitstream;
		bitstreamLen -= 1;
		bitstream += 1;
		switch(m_type0)
		{
			case AMF0_number_marker:
				if (bitstreamLen < 8) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 number marker.");
				m_number_val = AMF0_DecodeNumber(bitstream);
				bitstream += 8;
				bitstreamLen -= 8;
			break;

			case AMF0_boolean_marker:
				if (bitstreamLen < 1) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 boolean marker.");
				m_boolean_val = ((*bitstream) ? true : false);
				bitstream += 1;
				bitstreamLen -= 1;
			break;

			case AMF0_string_marker:
			{
				if (bitstreamLen < 2) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 string marker.");
				__uint16 slen = ntohs(*(__uint16*)bitstream);
				bitstream += 2;
				bitstreamLen -= 2;
				if (bitstreamLen < slen) throwEx<runtime_error>(logMsgPrefix + " Insufficient data for AMF0 string marker.");
				m_string_val = utf8(bitstream,bitstream + slen);
				bitstream += slen;
				bitstreamLen -= slen;
			}
			break;

			case AMF0_object_marker:
				m_object_val.loadFromBitstream(bitstream,bitstreamLen,0,logMsgPrefix);
			break;

			case AMF0_ecma_array_marker:
				m_ecma_array_val.loadFromBitstream(bitstream,bitstreamLen,0,logMsgPrefix);
			break;

			case AMF0_undefined_marker:
				DEBUG_LOG(logMsgPrefix + "Warning -  Undefined AMF0 marker " + tos(m_type0));
			case AMF0_null_marker:
			break;

			case AMF0_reference_marker:
			case AMF0_object_end_marker:
			case AMF0_strict_array_marker:
			case AMF0_date_marker:
			case AMF0_long_string_marker:
			case AMF0_unsupported_marker:
			case AMF0_recordset_marker:
			case AMF0_xml_Document_marker:
			case AMF0_typed_object_marker:
			case AMF0_amvplus_object_marker:
			case AMF0_movieclip_marker:
				throwEx<runtime_error>(logMsgPrefix + " Unsupported AMF0 marker " + tos(m_type0));
			break;

			default:
				throwEx<runtime_error>(logMsgPrefix + " Unknown AMF0 marker " + tos(m_type0));
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////

void AMFEncoding::loadFromBitstream(const char *bitstream, int bitstreamLen,const uniString::utf8 &logMsgPrefix) throw(exception)
{
	int blen = bitstreamLen;
	const char *bs = bitstream;
	vector<AMFVal> values;
	while(blen)
	{
		AMFVal v;
		v.loadFromBitstream(bs,blen,m_mode,logMsgPrefix);
		values.push_back(v);
	}
	m_values=values;
}

const AMFVal& AMFEncoding::getValue(size_t index) const throw(std::exception)
{
	if (index >= m_values.size())
		throwEx<runtime_error>("AMFEncoding::getValue(" + tos(index) + ") out of range");
	return m_values[index];
}

void AMFEncoding::appendValue(const AMFVal &v) throw()
{
	m_values.push_back(v);
}

void AMFEncoding::serialize(vector<__uint8> &s,const utf8 &logMsgPrefix) const throw(exception)
{
	for(vector<AMFVal>::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
		(*i).serialize(s,m_mode,logMsgPrefix);
}

utf8 AMFEncoding::prettyPrint() const throw()
{
	utf8 result("INVOKE(");
	result += eol();
	for(vector<AMFVal>::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
		result += (*i).prettyPrint(m_mode,"\t") + eol();
	result += ")";
	return result;
}
#endif
