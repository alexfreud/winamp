#pragma once
#ifndef _stlx_H_
#define _stlx_H_

#include <functional>

namespace stlx {

template<class Type1, class Type2, class Type3>
struct triple 
{
	typedef Type1 first_type;
	typedef Type2 second_type;
	typedef Type3 third_type;
	Type1 first;
	Type2 second;
	Type3 third;
	triple(){}
	triple(const Type1& __Val1, const Type2& __Val2,const Type3& __Val3)
		:first(__Val1),second(__Val2),third(__Val3){}
	template<class Other1, class Other2, class Other3>
	triple(const triple<Other1, Other2, Other3>& _Right)
		:first(_Right.first),second(_Right.second),third(_Right.third){}
};

template<class Type1, class Type2, class Type3>
	triple<Type1, Type2, Type3> make_triple(
		Type1 _Val1,
		Type2 _Val2,
		Type3 _Val3
	)
	{ return triple<Type1,Type2,Type3>(_Val1,_Val2,_Val3); }

/// templates which allow STL iterations using a class and it's method
/// (as opposed to mem_fun which requires the container class to hold the
/// class and it's method
///
/// ie
///
/// vector<CLASSA> a
/// B b;
/// for_each(a.begin(),a.end(),class_method_ref(b,B::foo))
///
/// this will invoke b.foo(a) for each element.

template<typename Result,typename T,typename P>
class class_method_functor: public std::unary_function<P,Result>
{
	typedef Result (T::*methodType)(P s);
	T			*m_class;
	methodType	m_method;
public:
	class_method_functor(T *c,methodType m):m_class(c),m_method(m){}
	Result operator()(P s) { return (m_class->*m_method)(s); }
};

template<typename Result,typename T,typename P>
class class_method_functor_const: public std::unary_function<P,Result>
{
	typedef Result (T::*methodType)(P s) const;
	const T		*m_class;
	methodType	m_method;
public:
	class_method_functor_const(const T *c,methodType m):m_class(c),m_method(m){}
	Result operator()(P s) const { return (m_class->*m_method)(s); }
};

template<typename Result,typename T,typename P>
class class_method_ref_functor: public std::unary_function<P,Result>
{
	typedef Result (T::*methodType)(P s);
	T			&m_class;
	methodType	m_method;
public:
	class_method_ref_functor(T &c,methodType m):m_class(c),m_method(m){}
	Result operator()(P s) { return (m_class.*m_method)(s); }
};

template<typename Result,typename T,typename P>
class class_method_ref_functor_const: public std::unary_function<P,Result>
{
	typedef Result (T::*methodType)(P s) const;
	const T		&m_class;
	methodType	m_method;
public:
	class_method_ref_functor_const(const T &c,methodType m):m_class(c),m_method(m){}
	Result operator()(P s) const { return (m_class.*m_method)(s); }
};

template<typename Result,typename T,typename P,typename PP>
class class_method_functor1: public std::binary_function<P,PP,Result>
{
	typedef Result (T::*methodType)(P s,PP ss);
	T			*m_class;
	methodType	m_method;
public:
	class_method_functor1(T *c,methodType m):m_class(c),m_method(m){}
	Result operator()(P s,PP ss) { return (m_class->*m_method)(s,ss); }
};

template<typename Result,typename T,typename P,typename PP>
class class_method_functor1_const: public std::binary_function<P,PP,Result>
{
	typedef Result (T::*methodType)(P s,PP ss) const;
	const T		*m_class;
	methodType	m_method;
public:
	class_method_functor1_const(const T *c,methodType m):m_class(c),m_method(m){}
	Result operator()(P s,PP ss) const { return (m_class->*m_method)(s,ss); }
};

template<typename Result,typename T,typename P,typename PP>
class class_method_ref_functor1: public std::binary_function<P,PP,Result>
{
	typedef Result (T::*methodType)(P s,PP ss);
	T			&m_class;
	methodType	m_method;
public:
	class_method_ref_functor1(T &c,methodType m):m_class(c),m_method(m){}
	Result operator()(P s,PP ss) { return (m_class.*m_method)(s,ss); }
};

template<typename Result,typename T,typename P,typename PP>
class class_method_ref_functor1_const: public std::binary_function<P,PP,Result>
{
	typedef Result (T::*methodType)(P s,PP ss) const;
	const T		&m_class;
	methodType	m_method;
public:
	class_method_ref_functor1_const(const T &c,methodType m):m_class(c),m_method(m){}
	Result operator()(P s,PP ss) const { return (m_class.*m_method)(s,ss); }
};

template<typename Result,typename T,typename P>
	class_method_functor<Result,T,P>
		class_method(T *c,Result (T::*m)(P s))
		{ return class_method_functor<Result,T,P>(c,m); }

template<typename Result,typename T,typename P>
	class_method_functor_const<Result,T,P>
	class_method(const T *c,Result (T::*m)(P s) const)
		{ return class_method_functor_const<Result,T,P>(c,m); }

template<typename Result,typename T,typename P>
	class_method_ref_functor<Result,T,P>
		class_method_ref(T &c,Result (T::*m)(P s))
			{ return class_method_ref_functor<Result,T,P>(c,m); }

template<typename Result,typename T,typename P>
	class_method_ref_functor_const<Result,T,P>
		class_method_ref(const T &c,Result (T::*m)(P s) const)
			{ return class_method_ref_functor_const<Result,T,P>(c,m); }

template<typename Result,typename T,typename P,typename PP>
	class_method_functor1<Result,T,P,PP>
		class_method(T *c,Result (T::*m)(P s,PP ss))
			{ return class_method_functor1<Result,T,P,PP>(c,m); }

template<typename Result,typename T,typename P,typename PP>
	class_method_functor1_const<Result,T,P,PP>
		class_method(const T *c,Result (T::*m)(P s,PP ss) const)
			{ return class_method_functor1_const<Result,T,P,PP>(c,m); }

template<typename Result,typename T,typename P,typename PP>
	class_method_ref_functor1<Result,T,P,PP>
		class_method_ref(T &c,Result (T::*m)(P s,PP ss))
			{ return class_method_ref_functor1<Result,T,P,PP>(c,m); }

template<typename Result,typename T,typename P,typename PP>
	class_method_ref_functor1_const<Result,T,P,PP>
		class_method_ref(const T &c,Result (T::*m)(P s,PP ss) const)
			{ return class_method_ref_functor1_const<Result,T,P,PP>(c,m); }

///////////////////////////////////////////////////////////
/*
	Allows search matches on members

	class foo
	{
		string m_memberToMatchOn;
	};

	vector<foo> myList;
	find_if(myList.begin(),m_myList.end(),
		member_match(string("hello"),&foo::m_memberToMatchOn));

*/
////////////////////////////////////////////////////////////
template<typename T,typename OBJ>
class member_match_functor
{
private:
	T m_value;
	T (OBJ::*m_member);
public:
	member_match_functor(const T &s,T OBJ::*m):m_value(s),m_member(m){}
	bool operator()(const OBJ &obj) throw()
	{ 
		return m_value == (obj.*m_member); 
	}
};

template<typename T,typename OBJ>
	member_match_functor<T,OBJ> member_match(const T &s,T OBJ::* m)
		{ return member_match_functor<T,OBJ>(s,m); }

template<typename T1,typename T2,typename OBJ>
class member2_match_functor
{
private:
	T1 m_value1;
	T1 (OBJ::*m_member1);
	T2 m_value2;
	T2 (OBJ::*m_member2);
public:
	member2_match_functor(const T1 &s1,T1 OBJ::*m1,const T2 &s2,T2 OBJ::*m2):m_value1(s1),m_member1(m1),m_value2(s2),m_member2(m2){}
	bool operator()(const OBJ &obj) throw()
		{ return ((m_value1 == (obj.*m_member1)) && (m_value2 == (obj.*m_member2))); }
};

template<typename T1,typename T2,typename OBJ>		
	member2_match_functor<T1,T2,OBJ> member2_match(const T1 &s1,T1 OBJ::* m1,const T2 &s2,T2 OBJ::* m2)
		{ return member2_match_functor<T1,T2,OBJ>(s1,m1,s2,m2); }

//***********************************************************
//* accumulate with a delimiter
//*
//***********************************************************
template<class _InIt,class _Ty,class _Tdel> inline _Ty accumulate_with_delimiter(_InIt _First, _InIt _Last, _Ty _Val, _Tdel _del)
{	// return sum of _Val and all in [_First, _Last)
	if (_First != _Last)
	{
		_Val = _Val + *_First;
		++_First;
	}

	for (; _First != _Last; ++_First)
	{
		_Val = _Val + _del + *_First;
	}
	return (_Val);
}

// with binop. Not sure if this one makes much sense
template<class _InIt,class _Ty,class _Fn2,class _Tdel> inline _Ty accumulate_with_delimiter(_InIt _First, _InIt _Last, _Ty _Val, _Tdel _del,_Fn2 _Func)
{	// return sum of _Val and all in [_First, _Last), using _Func
	if (_First != _Last)
	{
		_Val = _Func(_Val, *_First);
		++_First;
	}

	for (; _First != _Last; ++_First)
	{
		_Val = _Val + _del;
		_Val = _Func(_Val, *_First);
	}
	return (_Val);
}

//**********************************************************
//* streamOutFunctor
//*
//* functor class which will output an element to a stream
//* with a prefix and suffix string. Useful for outputing
//* elements of a container
//*********************************************************
template<class T> class streamOutFunctor
{
	std::ostream &m_o;
	const std::string m_prefix;
	const std::string m_suffix;
public:
	inline streamOutFunctor(std::ostream &o) : m_o(o){}
	inline streamOutFunctor(std::ostream &o,const std::string &prefix,const std::string &suffix)
		: m_o(o),m_prefix(prefix),m_suffix(suffix){}
	inline void operator()(const T &t) { m_o << m_prefix << t << m_suffix; }
};

//// for use on maps. Sort of the opposite of lower_bound. Returns the largest element
//// less than the key
template<typename Map> typename Map::const_iterator 
greatest_less(Map const& m, typename Map::key_type const& k) {
	typename Map::const_iterator it = m.lower_bound(k);
	if(it != m.begin()) {
		return --it;
	}
	return m.end();
}

template<typename Map> typename Map::iterator 
greatest_less(Map & m, typename Map::key_type const& k) {
	typename Map::iterator it = m.lower_bound(k);
	if(it != m.begin()) {
		return --it;
	}
	return m.end();
}

template<typename Map> typename Map::const_iterator 
greatest_less_or_equal(Map const& m, typename Map::key_type const& k) {
	typename Map::const_iterator it = m.lower_bound(k);
	if ((it != m.end()) && ((*it).first == k)) 
		return it;
	if(it != m.begin()) {
		return --it;
	}
	return m.end();
}

template<typename Map> typename Map::iterator 
greatest_less_or_equal(Map & m, typename Map::key_type const& k) {
	typename Map::iterator it = m.lower_bound(k);
	if ((it != m.end()) && ((*it).first == k)) 
		return it;
	if(it != m.begin()) {
		return --it;
	}
	return m.end();
}

}

#endif
