#include <algorithm>
#include "aolxml.h"
#include "stl/stringUtils.h"
#include <map>

#ifdef AOLXML_FASTLOOKUP
#ifdef _WIN32
#include <crtdbg.h>
#endif
#endif

using namespace std;
using namespace stringUtil;

// stl helpers
static void ndelete(aolxml::node *n) { delete n; }
static bool matchNameVS2005(const aolxml::node* n, string name)
	{ return (n->name() == name); }
//////////

aolxml::node::~node()
{
	// delete the children
	for_each(m_children.begin(),m_children.end(),ndelete);
}

void aolxml::node::deleteChild(childIterator_t i)
{
	// deletes child and all subchildren
	aolxml::node *n = (*i);

#ifdef AOLXML_FASTLOOKUP
	bool found(false);
	for(nameToNodeMap_t::iterator mi = m_nameToNodeMap.begin(); mi != m_nameToNodeMap.end(); ++mi)
	{
		if ((*mi).second == n)
		{
			m_nameToNodeMap.erase(mi);
			found = true;
			break;
		}
	}
	_ASSERTE(found);
#endif
	m_children.erase(i);

	delete n;
}

bool aolxml::node::deleteChild(aolxml::node *n)
{
	list<node*>::iterator i = find(m_children.begin(),m_children.end(),n);
	if (i != m_children.end())
	{
		#ifdef AOLXML_FASTLOOKUP
			bool found(false);
			for(nameToNodeMap_t::iterator mi = m_nameToNodeMap.begin(); mi != m_nameToNodeMap.end(); ++mi)
			{
				if ((*mi).second == n)
				{
					m_nameToNodeMap.erase(mi);
					found = true;
					break;
				}
			}
			_ASSERTE(found);			
		#endif
		m_children.erase(i);
		delete n;
		return true;
	}
	return false;
}

void aolxml::node::snipChild(childIterator_t i)
{
	// remove child from this node but keeps subtree in tact
#ifdef AOLXML_FASTLOOKUP
	aolxml::node *n = (*i);
	bool found(false);
	for(nameToNodeMap_t::iterator mi = m_nameToNodeMap.begin(); mi != m_nameToNodeMap.end(); ++mi)
	{
		if ((*mi).second == n)
		{
			m_nameToNodeMap.erase(mi);
			found = true;
			break;
		}
	}
	_ASSERTE(found);
#endif
	(*i)->m_parent = 0;
	m_children.erase(i);
}

void aolxml::node::addChild(node *n)
{
	n->m_parent = this;
	m_children.push_back(n);
#ifdef AOLXML_FASTLOOKUP
	m_nameToNodeMap.insert(make_pair(n->name(),n));
#endif
}

void aolxml::node::insertChild(childIterator_t i,node *n)
{
	n->m_parent = this;
	m_children.insert(i,n);
#ifdef AOLXML_FASTLOOKUP
	m_nameToNodeMap.insert(make_pair(n->name(),n));
#endif
}

aolxml::node* aolxml::node::findNode(node *root,const string &path) throw()
{
	if (path.empty() || (!root))
	{
		return 0;
	}

	vector<string> v = tokenizer(path,'/');

	node *current = root;
	if (path[0] == '/')
	{
		// move to root of tree
		while (current->parent())
			current = current->parent();

		// make sure first tag matches first token
		if (current && (!v.empty()) && (current->name() != v[0]))
		{
			current = 0;
		}
		else
		{
			v.erase(v.begin());
		}
	}

	for(vector<string>::const_iterator i = v.begin(); (i != v.end()) && current; ++i)
	{
		string s = stripWhitespace(*i);
		if (s == "" || s == ".")
		{
			continue;
		}
		else if (s == "..")
		{
			current = current->parent();
		}
		else
		{
#ifdef AOLXML_FASTLOOKUP
			nameToNodeMap_t::iterator mi = current->m_nameToNodeMap.find(s);
			current = (mi == current->m_nameToNodeMap.end() ? 0 : (*mi).second);
#else
			list<aolxml::node*>::iterator ci = find_if(
				current->childrenBegin(),current->childrenEnd(),bind2nd(ptr_fun(matchNameVS2005),s));
			current = (ci == current->childrenEnd() ? 0 : (*ci));
#endif
		}
	}

	return current;
}

list<aolxml::node*> aolxml::node::findNodes(node *root,const string &path) throw()
{
	list<aolxml::node*> result;

	if (path.empty() || (!root))
	{
		return result;
	}

	vector<string> v = tokenizer(path,'/');

	node *current = root;
	if (path[0] == '/')
	{
		// move to root of tree
		while (current->parent())
		{
			current = current->parent();
		}

		// make sure first tag matches first token
		if (current && (!v.empty()) && (current->name() != v[0]))
		{
			current = 0;
		}
		else
		{
			v.erase(v.begin());
		}
	}

	string last;
	if (!v.empty())
	{
		last = v.back();
		v.pop_back();
	}

	for(vector<string>::const_iterator i = v.begin(); (i != v.end()) && current; ++i)
	{
		string s = stripWhitespace(*i);
		if (s == "" || s == ".")
		{
			continue;
		}
		else if (s == "..")
		{
			current = current->parent();
		}
		else
		{
#ifdef AOLXML_FASTLOOKUP
			nameToNodeMap_t::iterator mi = current->m_nameToNodeMap.find(s);
			current = (mi == current->m_nameToNodeMap.end() ? 0 : (*mi).second);
#else
			list<aolxml::node*>::iterator ci = find_if(
				current->childrenBegin(),current->childrenEnd(),bind2nd(ptr_fun(matchNameVS2005),s));
			current = (ci == current->childrenEnd() ? 0 : (*ci));
#endif
		}
	}
	// now do last
	if (current && last != "")
	{
		list<node*> nv = current->m_children;
		copy(nv.begin(),stable_partition(nv.begin(),nv.end(),bind2nd(ptr_fun(matchNameVS2005),last)),back_inserter(result));
	}	

	return result;
}

static bool isAttr(pair<string,string> attr,string name) throw()
{
	return (attr.first == name);
}

aolxml::node::attributeList_t::iterator aolxml::node::findAttribute(const string &name) throw()
{
	return find_if(m_attributes.begin(),m_attributes.end(),bind2nd(ptr_fun(isAttr),name));
}

aolxml::node::attributeList_t::const_iterator aolxml::node::findAttribute(const string &name) const throw()
{
	return find_if(m_attributes.begin(),m_attributes.end(),bind2nd(ptr_fun(isAttr),name));
}

std::string aolxml::node::findAttributeString(const std::string &attr,const std::string &deflt) const throw()
{
	aolxml::node::attributeList_t::const_iterator i = findAttribute(attr);
	return (i == m_attributes.end() ? deflt : (*i).second);
}

std::string aolxml::node::findAttributeStringTHROW(const std::string &attr) const throw(std::runtime_error)
{
	aolxml::node::attributeList_t::const_iterator i = findAttribute(attr);
	if (i == m_attributes.end())
	{
		throw runtime_error("attribute " + attr + " in node " + m_name + " not found.");
	}
	return (*i).second;
}

void aolxml::node::findAttributeStringTHROW(const std::string &attr,std::string &s) const throw(std::runtime_error)
{
	s = findAttributeStringTHROW(attr);
}

void aolxml::node::findAttributeStringTHROW(const std::string &attr,int &i) const throw(std::runtime_error)
{
	i = atoi(findAttributeStringTHROW(attr).c_str());
}

void aolxml::node::findAttributeStringTHROW(const std::string &attr,double &d) const throw(std::runtime_error)
{
	d = atof(findAttributeStringTHROW(attr).c_str());
}

/////////////////////////////////////////////////////////////////////////////
////////////////////// Parsing (expat) //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void aolxml::StartTag (void *parser, const XML_Char * name, const XML_Char ** atts)
{
	aolxml::node* parent = reinterpret_cast<aolxml::node*>(XML_GetUserData((XML_Parser) parser));
	aolxml::node* current = new aolxml::node(name);

	if (parent)
	{
		current->m_preserveSpaces = parent->m_preserveSpaces;
		parent->addChild(current);
	}

	int attnum = XML_GetSpecifiedAttributeCount((XML_Parser) parser);
	for (int i = 0; i < attnum; i += 2) 
	{
		string key = atts[i];
		string value = atts[i+1];
		current->addAttribute(aolxml::node::attribute_t(key,value));
		if (key == "xml:space")
		{
			current->m_preserveSpaces = (value == "preserve");
		}
	}

	XML_SetUserData((XML_Parser) parser, current);
}

void aolxml::EndTag(void *parser, const XML_Char * /*name*/)
{
	aolxml::node* current = reinterpret_cast<aolxml::node*>(XML_GetUserData ((XML_Parser) parser));
	if (!current->m_preserveSpaces)
	{
		current->pcdata() = stripWhitespace(current->pcdata());
	}
	XML_SetUserData((XML_Parser) parser,current->parent() ? current->parent() : current);
}

void aolxml::CData(void *parser)
{
	aolxml::node* tag = reinterpret_cast<aolxml::node*>(XML_GetUserData ((XML_Parser) parser));
	tag->m_preserveSpaces = true;
}

void aolxml::PCData (void *parser, const XML_Char * s, int len)
{
	aolxml::node* tag = reinterpret_cast<aolxml::node*>(XML_GetUserData ((XML_Parser) parser));

	string ss(s,s+len);
	tag->pcdata() = tag->pcdata() + ss;//(tag->m_preserveSpaces ? ss : stripWhitespace(ss));
	// whitespace stripping moved to end tag because sometimes we get the PCData in
	// multiple calls
}

aolxml::node* aolxml::node::parse(const char *data,size_t len) throw(runtime_error)
{
	aolxml::node *root = 0;

	XML_Parser parser = XML_ParserCreate (NULL);

	// setup the parser
	XML_UseParserAsHandlerArg(parser);
	XML_SetElementHandler(parser, StartTag, EndTag);
	XML_SetCharacterDataHandler(parser, PCData);
	XML_SetUserData(parser, 0);
	XML_SetStartCdataSectionHandler(parser,CData);

	if (XML_Parse(parser, data,(int)len, 1)) 
	{
		root = reinterpret_cast<aolxml::node*>(XML_GetUserData (parser));
	} 
	else
	{
		// cleanup tree fragment
		root = reinterpret_cast<aolxml::node*>(XML_GetUserData (parser));
		if (root)
		{
			while (root->parent())
			{
				root = root->parent();
			}
			delete root;
		}
		//////////

		std::ostringstream o;
		o << "[XML] " << XML_ErrorString(XML_GetErrorCode(parser)) <<
			" at line " << XML_GetCurrentLineNumber(parser);
		XML_ParserFree(parser);
		throw std::runtime_error(o.str());
	}

	XML_ParserFree (parser);
	return root;
}

aolxml::node* aolxml::node::parse(const string &text) throw(runtime_error)
{
	return aolxml::node::parse(text.c_str(),text.size());
}

aolxml::node* aolxml::node::parse(const uniString::utf8::value_type *data,size_t len) throw(std::runtime_error)
{
	return aolxml::node::parse((const char *)data,len);
}

aolxml::node* aolxml::node::parse(const uniString::utf8 &text) throw(std::runtime_error)
{
	return aolxml::node::parse(&(text[0]),text.size());	
}

static void out_tabs(ostream &o,int t)
{
	for(int x = 0; x < t; ++x) o << "\t";
}

static void outputXML(ostream &o,const aolxml::node *n,int tabs)
{
	o << endl; out_tabs(o,tabs);
	o << "<" << n->name();
	for(aolxml::node::attributeList_t::const_iterator i = n->attributes().begin();
		i != n->attributes().end(); ++i)
	o << " " << (*i).first << "=\"" << (*i).second << "\"";
	o << ">" << n->pcdata();
	if (!n->childrenEmpty())
	{
		for(aolxml::node::const_childIterator_t i = n->childrenBegin(); i != n->childrenEnd(); ++i)
		{
			outputXML(o,(*i),tabs+1);
		}
		o << endl;
		out_tabs(o,tabs);
	}
	o << "</" << n->name() << ">";
}

ostream& aolxml::operator<<(ostream &o,const aolxml::node *n)
{
	outputXML(o,n,0);
	o << endl;
	return o;
}
