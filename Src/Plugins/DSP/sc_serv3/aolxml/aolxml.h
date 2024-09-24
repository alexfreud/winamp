#pragma once
#ifndef xml_H_
#define xml_H_

#include <string.h>
#include <string>
#include <stdexcept>
#include <list>
#include <numeric>
#include <functional>
#include "stl/stringUtils.h"
#include "unicode/uniString.h"

#define XMLIMPORT
#include "expat.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning( disable : 4290 ) 
#pragma warning( disable : 4996 ) 
#endif

//#define AOLXML_FASTLOOKUP 1

#ifdef AOLXML_FASTLOOKUP
#include <map>
#endif

namespace aolxml
{
	class node
	{
	public:
		typedef std::pair<std::string,std::string> attribute_t;
		typedef std::list<attribute_t> attributeList_t;
		typedef std::list<node*> nodeList_t;
		typedef nodeList_t::iterator childIterator_t;
		typedef nodeList_t::const_iterator const_childIterator_t;
		
	private:
		std::string			m_name;
		attributeList_t		m_attributes;
		std::string			m_pcdata;
		node*				m_parent;
		std::list<node*>	m_children;
		
#ifdef AOLXML_FASTLOOKUP
		typedef std::multimap<std::string,node*> nameToNodeMap_t;
		nameToNodeMap_t m_nameToNodeMap;
#endif
		bool				m_preserveSpaces;
		
	public:
		node(const std::string &name,const std::string &pcd = "")
			:m_name(name),m_pcdata(pcd),m_parent(0),m_preserveSpaces(false){}

		~node();
		
		const std::string &name() const throw() { return m_name; }
		std::string& name() throw() { return m_name; }
	
		attributeList_t& attributes() throw() { return m_attributes; }
		const attributeList_t& attributes() const throw() { return m_attributes; }
		void addAttribute(const attribute_t &a) throw() { m_attributes.push_back(a); }
		attributeList_t::iterator findAttribute(const std::string &attr) throw();
		attributeList_t::const_iterator findAttribute(const std::string &attr) const throw();
		std::string findAttributeString(const std::string &attr,const std::string &deflt = "") const throw();
		std::string findAttributeStringTHROW(const std::string &attr) const throw(std::runtime_error); // throws if attribute not found
		void findAttributeStringTHROW(const std::string &attr,std::string &s) const throw(std::runtime_error);
		void findAttributeStringTHROW(const std::string &attr,int &i) const throw(std::runtime_error);
		void findAttributeStringTHROW(const std::string &attr,double &f) const throw(std::runtime_error);
		
		node* parent() throw() { return m_parent;} 
		const node* parent() const throw() { return m_parent; }
		const std::string &pcdata() const throw() { return m_pcdata; }
		std::string& pcdata() throw() { return m_pcdata; }
		
		childIterator_t childrenBegin() throw() { return m_children.begin(); }
		childIterator_t childrenEnd()   throw() { return m_children.end(); }
		const_childIterator_t childrenBegin() const throw() { return m_children.begin(); }
		const_childIterator_t childrenEnd() const throw() { return m_children.end(); }
		bool childrenEmpty() const throw() { return m_children.empty(); }
		
		void deleteChild(childIterator_t i); // deletes child and all subchildren
		bool deleteChild(node *n);
		void snipChild(childIterator_t i); // remove child from this node but keeps subtree in tact

		void addChild(node *n);
		void insertChild(childIterator_t i,node *n);
		
		static node* findNode(node *root,const std::string &path) throw();
		static nodeList_t findNodes(node *root,const std::string &path) throw();

		static node* parse(const std::string &text) throw(std::runtime_error);
		static node* parse(const char *data,size_t len) throw(std::runtime_error);
		static node* parse(const uniString::utf8 &text) throw(std::runtime_error);
		static node* parse(const uniString::utf8::value_type *data,size_t len) throw(std::runtime_error);
		
		friend std::ostream& operator<<(std::ostream &o,const node *n);
		friend void StartTag (void *parser, const XML_Char * name, const XML_Char ** atts);
		friend void EndTag(void *parser, const XML_Char * name);
		friend void PCData (void *parser, const XML_Char * s, int len);		
		friend void CData(void *parser);
	};

	//////////////////////////////////////////////
	///// utility functions on nodes ////////
	////////////////////////////////////////////////
	
	template<typename T> // convert from text
	inline T nodeText(node *n) { return n->pcdata(); }

	template<>
	inline int nodeText(node *n) { return atoi(n->pcdata().c_str()); }
		
	template<>
	inline unsigned short nodeText(node *n) { return atoi(n->pcdata().c_str()); }
		
#ifdef _WIN32
	template<>
	inline __int64 nodeText(node *n) { return _strtoi64(n->pcdata().c_str(),0,10); }
		
	template<>
	inline unsigned __int64 nodeText(node *n) { return _strtoui64(n->pcdata().c_str(),0,10); }
#endif

	template<>
	inline bool nodeText(node *n)
	{
		const std::string &s = n->pcdata();
		if (s.empty()) return false;
		return (s[0] == 't' || s[0] == 'T' || s[0] == '1' || s[0] == 'y' || s[0] == 'Y');
	}

	template<>
	inline double nodeText(node *n) { return atof(n->pcdata().c_str()); }


	// get a subnode value. Returns true if value existed
	template<typename T>
	inline bool subNodeText(node *n,const std::string &path,T& value,const T defaultValue) throw()
	{
		value = defaultValue;
		if (!n)
		{
			return false;
		}
		node *subNode = node::findNode(n,path);
		if (!subNode)
		{
			return false;
		}
		value = aolxml::nodeText<T>(subNode);

		return true;
	}

	// same as above, but you cannot detect missing value
	template<typename T>
	inline T subNodeText(node *n,const std::string &path,const T defaultValue) throw()
	{			
		if (!n)
		{
			return defaultValue;
		}
		node *subNode = node::findNode(n,path);
		if (!subNode)
		{
			return defaultValue;
		}
		return aolxml::nodeText<T>(subNode);
	}

	inline void subNodeText(node *n,const std::string &path,char *result,int maxLen,const char *defaultValue) throw()
	{
		strncpy(result,defaultValue,maxLen);

		if (!n)
		{
			return;
		}

		node *subNode = node::findNode(n,path);
		if (!subNode)
		{
			return;
		}
		std::string tmp = aolxml::nodeText<std::string>(subNode);
		strncpy(result,tmp.c_str(),maxLen);
	}

	// this one throws if the value is not found
	template<typename T>
	inline T subNodeTextTHROW(node *n,const std::string &path) throw(std::runtime_error)
	{
		if (!n)
		{
			throw std::runtime_error("node NULL");
		}
		node *subNode = node::findNode(n,path);
		if (!subNode)
		{
			throw std::runtime_error(path + " missing");
		}
		return aolxml::nodeText<T>(subNode);
	}

	// throws and returns value as a param instead of a function value
	template<typename T>
	inline void subNodeTextTHROW(node *n,const std::string &path,T &result) throw(std::runtime_error)
	{
		result = aolxml::subNodeTextTHROW<T>(n,path);
	}

	/// calls function 'f' with each node in the list and the value 'l'
	// The purpose is to allow you to write a function which will insert
	// data from each node into container 'l'
	template<typename L,typename Func>
	inline void XMLList(L &l,const node::nodeList_t &lnode,Func f) throw(std::runtime_error)
	{
		for(node::nodeList_t::const_iterator i = lnode.begin(); i != lnode.end(); ++i)
		{
			if (*i)
			{
				f(l,*i);
			}
		}
	}

	template<typename L,typename Func>
	inline void XMLList(L &l,node::nodeList_t &lnode,Func f) throw(std::runtime_error)
	{
		for(node::nodeList_t::iterator i = lnode.begin(); i != lnode.end(); ++i)
		{
			if (*i)
			{
				f(l,*i);
			}
		}
	}

	template<typename L,typename Func>
	inline void XMLList(L &l,node *n,const std::string &path,Func f) throw(std::runtime_error)
	{
		if (!n)
		{
			return;
		}
		node::nodeList_t lnode = node::findNodes(n,path);
		for(node::nodeList_t::iterator i = lnode.begin(); i != lnode.end(); ++i)
		{
			if (*i)
			{
				f(l,*i);
			}
		}
	}


	//****************************************************************
	//* Various templates for constructing XML data
	//****************************************************************
	
	// embed a value in a set of tags (<tag>value</tag>) providing a function
	// which will convert the value to a string
	template<typename T,typename ConverterFunc>
	std::string xmlTag(const std::string &tag,const T value,ConverterFunc func)
	{
		return std::string("<") + tag + ">" + func(value) + "</" + tag + ">";
	}

	// embed a string convertible value to <tag>value</tag>
	template<typename T>
	std::string xmlTag(const std::string &tag,const T value)
	{
		return std::string("<") + tag + ">" + stringUtil::tos(value) + "</" + tag + ">";
	}

	inline std::string xmlTag(const std::string &tag,bool value)
	{
		return std::string("<") + tag + ">" + (value ? "1" : "0") + "</" + tag + ">";
	}

	// helper functor to make lists of web taged values using accumulate
	template<typename T,typename ConverterFunc>
	class xmlListFunctor: public std::binary_function<std::string,T,std::string>
	{
		std::string		m_tag;
		ConverterFunc	m_func;

	public:
		xmlListFunctor(const std::string &tag,ConverterFunc f): m_tag(tag),m_func(f){}
		std::string operator()(const std::string &accum,const T &val)
		{
			return accum + xmlTag(m_tag,val,m_func);
		}
	};

	// template function so we don't have to use the entire accumulate syntax.
	/* example:
		
			list<string> items;
			items.push_back("foo");
			items.push_back("bar");
			items.push_back("narf");
			
			string result = xmlList(items,"junk");
	
			// the value of result is now
			// <junk>foo</junk><junk>bar</junk><junk>narf</junk>
	*/
	
	template<typename Container>
	std::string xmlList(const Container &c,const std::string &tag)
	{
		typedef std::string (*tostype)(const typename Container::value_type);

		return std::accumulate(c.begin(),c.end(),std::string(""),
							   xmlListFunctor<typename Container::value_type,tostype>(tag,stringUtil::tos<Container::value_type>));
	}

	// similar to xmlList, but you provide a converter func to make your values into a string
	template<typename Container,typename ConverterFunc>
	std::string xmlList(const Container &c,const std::string &tag,ConverterFunc func)
	{
		return std::accumulate(c.begin(),c.end(),std::string(""),
							   xmlListFunctor<typename Container::value_type,ConverterFunc>(tag,func));
	}

	// similar to xmlList, but with auto indenting
	template<typename Container>
	static std::string xmlListIndented(const Container &c,const std::string &tag,const std::string &indent) 
	{
		std::string result;
		for (typename Container::const_iterator i = c.begin(); i != c.end(); ++i)
		{
			result += indent + "<" + tag + ">" + stringUtil::tos(*i) + "</" + tag + ">" + stringUtil::eol();
		}
		return result;
	}

	template<typename Container,typename Func>
	static std::string xmlListIndented(const Container &c,const std::string &tag,const std::string &indent,Func f) 
	{
		std::string result;
		for (typename Container::const_iterator i = c.begin(); i != c.end(); ++i)
		{
			result += indent + "<" + tag + ">" + stringUtil::eol();
			result += f(*i,indent + "\t");
			result += indent + "</" + tag + ">" + stringUtil::eol();
		}
		return result;
	}

	// escape a string so it is valid inside an XML tag
	std::string escapeXML(const std::string &s) throw();
	uniString::utf8 escapeXML(const uniString::utf8 &s) throw();
	/*static std::string utf8Header() throw() { return "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"; }
	static std::string latin1Header() throw() { return "<?xml version=\"1.0\" encoding=\"latin1\" ?>\n"; }*/
	
	std::ostream& operator<<(std::ostream &o,const node *n);
	void StartTag (void *parser, const XML_Char * name, const XML_Char ** atts);
	void EndTag(void *parser, const XML_Char * name);
	void PCData (void *parser, const XML_Char * s, int len);		
	void CData(void *parser);
}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
