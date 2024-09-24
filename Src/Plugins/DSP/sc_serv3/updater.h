#pragma once
#ifndef download_H_
#define download_H_

#include "webClient.h"
#include "webNet/urlUtils.h"
#include "yp2.h"

#define UPDATER_LOGNAME "[UPDATER] " 
class updater: public webClient
{
public:
	struct verInfo
	{
		int needsUpdating;
		int downloaded;
		uniString::utf8 ver;
		uniString::utf8 url;
		uniString::utf8 log;
		uniString::utf8 info;
		uniString::utf8 message;
		uniString::utf8 slimmsg;
		uniFile::filenameType fn;
		uniFile::filenameType fn_alt;
		verInfo() : needsUpdating(0), downloaded(0) {}
	};

	updater() throw();
	~updater() throw();

	// used in main during shutdown to wait for request queue to clear out
	static size_t requestsInQueue() throw();

	static bool getNewVersion(verInfo &ver) throw();
	static bool setNewVersion(verInfo &ver, bool no_lock = false) throw();

private:
	AOL_namespace::mutex	m_serverMapLock;

	verInfo m_verInfo;
	bool m_running;

	virtual uniString::utf8 name() const throw() { return "updater"; }

	virtual void gotResponse(const request &q, const response &r) throw(std::exception);
	virtual	void gotFailure(const request &q) throw(std::exception);

	static void response_updater(const request &q,const response &r) throw(std::exception);
	void failure_updater(const request &q) throw();

	static void updaterBandWidthSent(webClient::request r) throw();
	static void updaterBandWidthReceived(const response r) throw();

	void pvt_downloadUpdate() throw(std::exception);
	static void updateVersion() throw();
};
	
#endif
