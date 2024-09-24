#ifndef NULLSOFT_WINAMP_ATTRIBUTES_H
#define NULLSOFT_WINAMP_ATTRIBUTES_H

#include "../Agave/Config/ifc_configitem.h"

class _bool_base : public ifc_configitem
{
public:
	_bool_base();
	bool GetBool();
	void SetBool(bool boolValue);
	intptr_t GetInt();
	void SetInt(intptr_t intValue);
	operator intptr_t();
	intptr_t operator =(intptr_t intValue);
	bool operator =(bool boolValue);
	operator bool();
	operator UINT(); // for CheckDlgButton
	bool operator !();
protected:
	bool value;
};

class _bool : public _bool_base
{
public:
	_bool(bool defaultValue);
protected:
		RECVS_DISPATCH;
};

/* _mutable_bool allows the config item to be changed via users of api_config */
class _mutable_bool : public _bool_base
{
public:
	_mutable_bool(bool defaultValue);
protected:
	RECVS_DISPATCH;
};

class _unsigned : public ifc_configitem
{
public:
	_unsigned();
	_unsigned(uintptr_t defaultValue);
	
	uintptr_t GetUnsigned() { return value; }
	uintptr_t operator =(uintptr_t uintValue); 
	operator uintptr_t() { return value; }

protected:
	RECVS_DISPATCH;
private:
	uintptr_t value;
};

class _int : public ifc_configitem
{
public:
	_int();
	_int(intptr_t defaultValue);
	
	intptr_t GetInt() { return value; }
	float GetFloat() { return (float)value; }
	intptr_t operator =(intptr_t uintValue); 
	operator intptr_t() { return value; }

protected:
	RECVS_DISPATCH;
private:
	intptr_t value;
};

class _float : public ifc_configitem
{
public:
	_float();
	_float(float defaultValue);
	
	intptr_t GetInt() { return (intptr_t)value; }
	intptr_t operator =(intptr_t uintValue); 
	operator intptr_t() { return static_cast<intptr_t>(value); }

	float GetFloat() { return value; }
	float operator =(float uintValue); 
	operator float () { return value; }

protected:
	RECVS_DISPATCH;
private:
	float value;
};

#endif