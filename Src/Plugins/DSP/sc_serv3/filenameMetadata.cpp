#include "filenameMetadata.h"
#include "metadata.h"
#include "stl/stringUtils.h"
#include "macros.h"
#include <functional>
#include <algorithm>
#include <string>
#include <list>
#include <assert.h>

using namespace std;
using namespace uniString;
using namespace stringUtil;

/*
	Overview of how it works:

	The setPattern() method looks at the pattern string and builds a stack of parseState objects.
	Each of these objects are responsible for finding their associated pattern within a text range.

	parsing is done right to left.

	the parseState_optional object is used to encapsulate  other state objects that are
	optional (bracketed by [] in the pattern).
*/

class filenameMetadata::impl
{
	// to make unicode compatibility easier, we're just going to store things as utf32
	utf32 m_pattern;
	utf32 m_data;

	// put the map will be in utf 8
	typedef map<utf8,utf8> tokenMap_t;
	tokenMap_t m_tokens;

	class parseState;
	typedef list<parseState*> parseStack_t;
	parseStack_t m_parseStack;
	static void clearparseStack(parseStack_t &ps) throw()
	{
		while (!ps.empty())
		{
			delete ps.back();
			ps.pop_back();
		}
	}

	void clearparseStack() throw() { clearparseStack(m_parseStack); }

	///////////// parse states //////////////////////////
	class parseState // virtual base
	{
	public:
		typedef utf32::const_reverse_iterator range_e;
		typedef pair<range_e, range_e> range_t;

		virtual ~parseState() throw() {}
		virtual range_t	findRange(range_e rbegin, range_e rend) throw() { return make_pair(rbegin, rend); }
		virtual void setFromRange(range_e /*rbegin*/, range_e /*rend*/) throw() {}
		virtual void reportValue(tokenMap_t &/*tm*/) const throw() {}
		virtual utf8 describe() const throw() = 0; // for diagnostics
		virtual void reset() throw() {}
		virtual bool optional() const throw() { return false; }
		virtual bool finite() const throw() { return false; } // fixed width match
	};

	class parseState_optional: public parseState
	{
		parseStack_t m_parseStack;

	public:
		parseState_optional(){}
		~parseState_optional() throw() { clearparseStack(m_parseStack); }
		parseStack_t* stack() throw() { return &m_parseStack; }

		virtual bool optional() const throw() { return true; }

		virtual void reset() throw()
		{
			for_each(m_parseStack.begin(), m_parseStack.end(), mem_fun(&parseState::reset));
		}

		virtual utf8 describe() const throw()
		{
			utf8 result("[");
			for (parseStack_t::const_iterator i = m_parseStack.begin(); i != m_parseStack.end(); ++i)
			{
				result += (*i)->describe();
			}
			result = result + utf8("]");
			return result;
		}

		virtual range_t findRange(range_e rbegin,range_e rend) throw()
		{
			const range_t NOTFOUND(make_pair(rend, rend));
			range_t result;

			reset();
			if (m_parseStack.empty())
			{
				return NOTFOUND;
			}

			parseStack_t::reverse_iterator s_cur = m_parseStack.rbegin();
			parseStack_t::reverse_iterator s_nxt = s_cur;
			++s_nxt;

			range_e data_start = rbegin;
			range_e data_end   = rend;
			range_e last_restart = rbegin;

			bool first(true);
			while (s_cur != m_parseStack.rend())
			{
				if (data_start == data_end)
				{
					reset();
					return NOTFOUND;
				}

				range_t curR(NOTFOUND);
				range_t nxtR(NOTFOUND);
				curR = (*s_cur)->findRange(data_start,data_end);
				if (curR.first == data_end)
				{
					reset();
					return NOTFOUND;
				}
				if ((!first) && (curR.first != data_start))
				{
					// must abut. Try moving forward again 
					reset(); 
					s_cur = m_parseStack.rbegin();
					s_nxt = s_cur;
					++s_nxt;
					++last_restart;
					data_start = last_restart;
					first = true;
					continue;
				}
				if (first)
				{
					result.first = curR.first;
				}
				first = false;

				// don't do this if we have a single character state followed
				// by anything (in particular, a string which eats all
				if (curR.first + 1 != curR.second)
				{
					if (s_nxt != m_parseStack.rend())
					{
						nxtR = (*s_nxt)->findRange(data_start,data_end);
					}
					if (nxtR.first < curR.second)
					{
						curR.second = nxtR.first;
					}
				}
				(*s_cur)->setFromRange(curR.first,curR.second);
				s_cur = s_nxt;
				if (s_nxt != m_parseStack.rend())
				{
					++s_nxt;
				}
				data_start = curR.second;
			}
			result.second = data_start;
			return result;
		}

		virtual void setFromRange(utf32::const_reverse_iterator rbegin, utf32::const_reverse_iterator rend) throw()
		{ 
			findRange(rbegin, rend); // resets to restricted range if necessary
		}

		virtual void reportValue(tokenMap_t &tm) const throw() 
		{
			for (parseStack_t::const_iterator i = m_parseStack.begin(); i != m_parseStack.end(); ++i)
			{
				(*i)->reportValue(tm);
			}
		}

		virtual bool finite() const throw()
		{
			bool result = true;
			for (parseStack_t::const_iterator i = m_parseStack.begin(); i != m_parseStack.end(); ++i)
			{
				result &= (*i)->finite();
			}
			return result;
		}
	};

	class parseState_char: public parseState
	{
		utf32::value_type	m_char;

	public:
		explicit parseState_char(utf32::value_type c) : m_char(c){}
		virtual pair<utf32::const_reverse_iterator,utf32::const_reverse_iterator> 
		findRange(utf32::const_reverse_iterator rbegin,utf32::const_reverse_iterator rend) throw()
		{
			for (utf32::const_reverse_iterator i = rbegin; i != rend; ++i)
			{
				if ((*i) == m_char)
				{
					return make_pair(i, i + 1);
				}
			}
			return make_pair(rend,rend);
		}

		virtual utf8 describe() const throw() 
		{
			utf32 u32; u32.push_back(m_char);
			return u32.toUtf8();
		}

		virtual bool finite() const throw() { return true; }
	};

	class parseState_stringSymbol: public parseState
	{
		utf8	m_symbolName; // can be empty for any string
		utf32	m_value;

	public:
		parseState_stringSymbol() throw(){}
		explicit parseState_stringSymbol(const string &s) throw() : m_symbolName(s){}
		~parseState_stringSymbol() throw(){}
		void reset() throw() { m_value.clear(); }
		void setFromRange(utf32::const_reverse_iterator rbegin,utf32::const_reverse_iterator rend) throw()
		{
			if (!m_symbolName.empty())
			{
				m_value.clear();
				m_value.insert(m_value.begin(),rbegin,rend);
				reverse(m_value.begin(),m_value.end());
				m_value = stripWhitespace(m_value);
			}
		}

		virtual void reportValue(tokenMap_t &tm) const throw() 
		{
			if (!m_symbolName.empty() && !m_value.empty())
			{
				tm[m_symbolName] = m_value.toUtf8();
			}
		}

		virtual utf8 describe() const throw()
		{
			if (m_symbolName.empty()) return utf8("*");
			return utf8("%") + m_symbolName;
		}
	};

	class parseState_digits: public parseState
	{
	public:
		parseState_digits() throw(){}
		virtual range_t findRange(range_e rbegin,range_e rend) throw()
		{
			range_t result(make_pair(rend,rend));

			bool got_start = false;
			for (utf32::const_reverse_iterator i = rbegin; i != rend; ++i)
			{
				if (uniString::is_a_number(*i))
				{
					if (!got_start)
					{
						got_start = true;
						result.first = i;
					}
				}
				else
				{
					if (got_start)
					{
						result.second = i;
						return result;
					}
				}
			}
			return result;
		}

		virtual utf8 describe() const throw() { return utf8("%#"); }
	};

	class parseState_year: public parseState
	{
		utf32 m_value;

	public:
		parseState_year() throw(){}
		~parseState_year() throw(){}
		void reset() throw() { m_value.clear(); }
		virtual pair<utf32::const_reverse_iterator,utf32::const_reverse_iterator> 
		findRange(utf32::const_reverse_iterator rbegin,utf32::const_reverse_iterator rend) throw()
		{
			int count = 4;

			pair<utf32::const_reverse_iterator,utf32::const_reverse_iterator> result(make_pair(rend,rend));

			bool got_start = false;
			for (utf32::const_reverse_iterator i = rbegin; i != rend; ++i)
			{
				if (uniString::is_a_number(*i))
				{
					if (!got_start)
					{
						got_start = true;
						result.first = i;
					}
					count -= 1;
					if (count == 0)
					{
						result.second = ++i;
						return result;
					}
				}
				else
				{
					if (got_start)
					{
						got_start = false;
						result.first = rend;
					}
				}
			}
			return make_pair(rend,rend);
		}

		void setFromRange(utf32::const_reverse_iterator rbegin,utf32::const_reverse_iterator rend) throw()
		{
			m_value.clear();
			m_value.insert(m_value.begin(),rbegin,rend);
			reverse(m_value.begin(),m_value.end());
		}

		virtual void reportValue(tokenMap_t &tm) const throw() 
		{
			if (!m_value.empty())
			{
				tm[utf8(metadata::YEAR())] = m_value.toUtf8();
			}
		}

		virtual utf8 describe() const throw() { return utf8("%YEAR"); }
		virtual bool finite() const throw() { return true; }
	};

	class parseState_fixed: public parseState
	{
		utf32 m_value;

	public:
		// fixed string
		explicit parseState_fixed(const utf32 &val) throw() : m_value(val) {}
		~parseState_fixed() throw(){}

		virtual pair<utf32::const_reverse_iterator,utf32::const_reverse_iterator> 
		findRange(utf32::const_reverse_iterator rbegin,utf32::const_reverse_iterator rend) throw()
		{
			assert(!m_value.empty());
			if (m_value.empty()) return make_pair(rend,rend);

			for (utf32::const_reverse_iterator i = rbegin; i != rend; ++i)
			{
				if ((*i) == (*(m_value.rbegin())))
				{
					utf32::const_reverse_iterator t_i = i;
					utf32::const_reverse_iterator v_i = m_value.rbegin();
					utf32::const_reverse_iterator v_i_end = m_value.rend();
					bool match(true);
					for (; match && (v_i != v_i_end); ++t_i, ++v_i)
					{
						if ((t_i == rend) || ((*t_i) != (*v_i)))
						{
							match = false;
						}
					}
					if (match)
					{
						return make_pair(i, i + m_value.size());
					}
				}
			}
			return make_pair(rend,rend);
		}

		virtual utf8 describe() const throw() 
		{
			return m_value.toUtf8();
		}

		virtual bool finite() const throw() { return true; }
	};

	static string stringify(utf32::value_type v) throw()
	{
		if (v >= '0' && v <= 'z') return string(1,(string::value_type)v);
		return tos((int)v);
	}

public:

	impl(){}
	~impl() throw() 
	{ 
		clearparseStack(); 
	}

	void deleteToken(const utf8 &token) throw()
	{
		tokenMap_t::iterator i = m_tokens.find(token);
		if (i != m_tokens.end()) m_tokens.erase(i);
	}

	const tokenMap_t::size_type countTokens() const throw()		{ return m_tokens.size(); }
	utf8& operator[](const utf8 &key) throw()		{ return m_tokens[key]; }
	const map<utf8,utf8>& getTokens() const throw()	{ return m_tokens; }

	void setPattern(const utf8 &pattern) throw(runtime_error)
	{
		parseState_optional *opt = 0;
		parseStack_t *stack = &m_parseStack;

		try
		{
			utf32 fixedAccumulator; // fixed string value

			#define DUMPACCUMULATOR	{ if (!fixedAccumulator.empty()) { stack->push_back(new parseState_fixed(fixedAccumulator)); fixedAccumulator.clear(); } }

			clearparseStack();
			m_pattern.assign(pattern);
			for (utf32::const_iterator i = m_pattern.begin(); i != m_pattern.end(); ++i)
			{
				if ((*i) == ']')
				{
					DUMPACCUMULATOR
					if (!opt) throw runtime_error("Unmatched ']' in pattern");
					stack = &m_parseStack;
					stack->push_back(opt);
					opt = 0;
				}
				else if ((*i) == '[')
				{
					DUMPACCUMULATOR
					if (opt) throw runtime_error("Optional sequences cannot be nested in pattern");
					opt = new parseState_optional;
					stack = opt->stack();
				}
				else if ((*i) == '%')
				{
					++i;
					if (i == m_pattern.end()) throw runtime_error("Bad pattern. Trailing %");
					switch (*i)
					{
						case 'N':	DUMPACCUMULATOR stack->push_back(new parseState_stringSymbol(metadata::NAME()));	break;
						case 'G':	DUMPACCUMULATOR stack->push_back(new parseState_stringSymbol(metadata::GENRE()));	break;
						case 'A':	DUMPACCUMULATOR stack->push_back(new parseState_stringSymbol(metadata::ALBUM()));	break;
						case 'R':	DUMPACCUMULATOR stack->push_back(new parseState_stringSymbol(metadata::ARTIST()));break;
						case 'Y':	DUMPACCUMULATOR stack->push_back(new parseState_year);					break;
						case '#':	DUMPACCUMULATOR stack->push_back(new parseState_digits);				break;
						case '%':	fixedAccumulator.push_back('%'); break;
						default:	throw runtime_error("Unknown symbol %" + stringify(*i));
					}
				}
				else if ((*i) == '*')
				{
					DUMPACCUMULATOR
					stack->push_back(new parseState_stringSymbol);
				}
				else
				{
					fixedAccumulator.push_back(*i);
				}
			}
			if (opt)
			{
				throw runtime_error("Unterminated optional sequence in pattern");
			}
			DUMPACCUMULATOR
		}
		catch(...)
		{
			delete opt;
			throw;
		}
	}

	static utf8 describeStackRange(parseStack_t::const_reverse_iterator begin,parseStack_t::const_reverse_iterator end) throw()
	{
		parseStack_t stck(begin,end);
		reverse(stck.begin(),stck.end());
		utf8 result;
		for (parseStack_t::const_iterator i = stck.begin(); i != stck.end(); ++i)
		{
			result = result + (*i)->describe();
		}
		return result;
	}

	static utf8 describeRemainingData(utf32::const_reverse_iterator begin,utf32::const_reverse_iterator end) throw()
	{
		utf32 u32(begin,end);
		reverse(u32.begin(),u32.end());
		return u32.toUtf8();
	}

	void parse(const utf8 &data) throw(runtime_error)
	{
		m_data.assign(data);

		// beginning and end of data string
		utf32::const_reverse_iterator data_start = m_data.rbegin();
		utf32::const_reverse_iterator data_end = m_data.rend();

		// current and next object pointers from the parse stack
		parseStack_t::reverse_iterator s_cur = m_parseStack.rbegin();
		parseStack_t::reverse_iterator s_nxt = s_cur;
		++s_nxt;

		while(s_cur != m_parseStack.rend())
		{
			// if we haven't finished the parse stack, and we're out of data then it's an error
			if (data_start == data_end)
			{
				throw runtime_error("Premature end of data (" + describeStackRange(s_cur,m_parseStack.rend()).hideAsString() + ")");
			}

			// we do one lookahead. Get the range match for the current parse object and
			// the next parse object. Note that there is some added complexity due to optional objects
			pair<utf32::const_reverse_iterator,utf32::const_reverse_iterator> curR(make_pair(data_end,data_end));
			pair<utf32::const_reverse_iterator,utf32::const_reverse_iterator> nxtR(make_pair(data_end,data_end));

			// find widest possible match for current state
			curR = (*s_cur)->findRange(data_start,data_end);

			// if no match, and the object is optional, just move on to the next (continue)
			if ((curR.first == data_end) && (*s_cur)->optional())
			{
				s_cur = s_nxt;
				if (s_nxt != m_parseStack.rend())
				{
					++s_nxt;
				}
				continue;
			}

			// if no match, but object is not optional, then we have an error
			if (curR.first == data_end)
			{
				throw runtime_error("Parse error, symbol not found (" + describeStackRange(s_cur,m_parseStack.rend()).hideAsString() + ") (" + describeRemainingData(data_start,data_end).hideAsString() + ")");
			}

			// if match was not found at our current starting point, then we have an error
			if (curR.first != data_start)
			{
				throw runtime_error("Parse error, data skipped to find symbol (" + describeStackRange(s_cur,m_parseStack.rend()).hideAsString() + ") (" + describeRemainingData(data_start,data_end).hideAsString() + ")");
			}

			// restrict match range by one lookahead. Do not do lookahead
			// if our current state is a single character match
			if (!(*s_cur)->finite()) //curR.first + 1 != curR.second)
			{
				// we must loop in case the followup objects are optional and we must
				// continue to look ahead
				while (true)
				{
					if (s_nxt == m_parseStack.rend()) break;
					// to handle the case of two optional string elements in a row, we
					// repeat this if the range of the current and follow up objects match by
					// incrementing the start
					nxtR = (*s_nxt)->findRange(data_start,data_end);
					if ((nxtR.first == curR.first) && (!(*s_nxt)->finite()))
					{
						nxtR = (*s_nxt)->findRange(data_start+1,data_end);
					}
					if (nxtR.first < curR.second)
					{
						// lookahead object restricts range
						curR.second = nxtR.first;
						break;
					}
					if ((nxtR.first == data_end) && (nxtR.second == data_end) && (*s_nxt)->optional())
					{
						// lookahead object not found and is optional. try the next
						++s_nxt;
					}
					else
					{
						// no restriction
						break;
					}
				}
			}

			// set value and advance to next parse object
			(*s_cur)->setFromRange(curR.first,curR.second);
			s_cur = s_nxt;
			if (s_nxt != m_parseStack.rend())
			{
				++s_nxt;
			}
			data_start = curR.second;
		}
		if (data_start != data_end)
		{
			throw runtime_error("Data extends beyond pattern (" + describeRemainingData(data_start,data_end).hideAsString() + ")");		
		}

		m_tokens.clear();
		for (parseStack_t::const_iterator i = m_parseStack.begin(); i != m_parseStack.end(); ++i)
		{
			(*i)->reportValue(m_tokens);
		}
	}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

filenameMetadata::filenameMetadata(): m_impl(0)
{
	m_impl = new filenameMetadata::impl;
}

filenameMetadata::~filenameMetadata() throw()
{
	forget(m_impl);
}

void filenameMetadata::setPattern(const utf8 &pattern) throw(exception)
{
	assert(m_impl);
	if (!m_impl) throw logic_error(string(__FUNCTION__) + " internal impl object is null");
	m_impl->setPattern(pattern);
}

void filenameMetadata::parse(const utf8 &data) throw(exception)
{
	assert(m_impl);
	if (!m_impl) throw logic_error(string(__FUNCTION__) + " internal impl object is null");
	m_impl->parse(data);
}

void filenameMetadata::deleteToken(const utf8 &token) throw(exception)
{
	assert(m_impl);
	if (!m_impl) throw logic_error(string(__FUNCTION__) + " internal impl object is null");
	m_impl->deleteToken(token);
}

const size_t filenameMetadata::countTokens() throw(exception)
{
	assert(m_impl);
	if (!m_impl) throw logic_error(string(__FUNCTION__) + " internal impl object is null");
	return m_impl->countTokens();
}

utf8& filenameMetadata::operator[](const utf8 &key) throw(exception)
{
	assert(m_impl);
	if (!m_impl) throw logic_error(string(__FUNCTION__) + " internal impl object is null");
	return m_impl->operator[](key);
}

const map<utf8,utf8>& filenameMetadata::getTokens() const throw(exception)
{
	assert(m_impl);
	if (!m_impl) throw logic_error(string(__FUNCTION__) + " internal impl object is null");
	return m_impl->getTokens();
}
