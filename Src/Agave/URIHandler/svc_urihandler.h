#pragma once
#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include <bfc/std_mkncc.h> // for MKnCC()

class svc_urihandler : public Dispatchable
{
protected:
	svc_urihandler() {}
	~svc_urihandler() {}
public:
	static FOURCC getServiceType() { return svc_urihandler::SERVICETYPE; }
	enum
	{
		NOT_HANDLED = -1, // return if it's not yours
		HANDLED = 0, // return if it's yours, but other services might want to handle also
		HANDLED_EXCLUSIVE = 1, // if it's yours AND NO ONE ELSES
	};
	int ProcessFilename(const wchar_t *filename);
	int IsMine(const wchar_t *filename); // just like ProcessFilename but don't actually process
	int EnumProtocols(size_t n, wchar_t *protocol, size_t protocolCch, wchar_t *description, size_t descriptionCch); // return 0 on success
	int RegisterProtocol(const wchar_t *protocol, const wchar_t *winampexe);
	int UnregisterProtocol(const wchar_t *protocol);

	enum
	{
		PROCESSFILENAME=0,
		ISMINE=1,
		ENUMPROTOCOLS=2,
		REGISTERPROTOCOL=3,
		UNREGISTERPROTOCOL=4,
	};

	enum
	{
		SERVICETYPE = MK4CC('u','r','i', 'h')
	};
};

inline int svc_urihandler::ProcessFilename(const wchar_t *filename)
{
	return _call(PROCESSFILENAME, (int)NOT_HANDLED, filename);
}

inline int svc_urihandler::IsMine(const wchar_t *filename)
{
		return _call(ISMINE, (int)NOT_HANDLED, filename);
}

inline int svc_urihandler::EnumProtocols(size_t n, wchar_t *protocol, size_t protocolCch, wchar_t *description, size_t descriptionCch)
{
	return _call(ENUMPROTOCOLS, (int)1, n, protocol, protocolCch, description, descriptionCch);
}

inline int svc_urihandler::RegisterProtocol(const wchar_t *protocol, const wchar_t *winampexe)
{
	return _call(REGISTERPROTOCOL, (int)1, protocol, winampexe);
}

	inline int svc_urihandler::UnregisterProtocol(const wchar_t *protocol)
	{
		return _call(UNREGISTERPROTOCOL, (int)1, protocol);
	}
