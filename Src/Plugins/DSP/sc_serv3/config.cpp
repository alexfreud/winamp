#include "config.h"
#include "stl/stringUtils.h"
#include <set>
#include "cpucount.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"
#ifdef _WIN32
#include "win32/rezFuncs.h"
#else
#include "unixversion.h"
#endif
#include "banList.h"
#include "ripList.h"
#include "adminList.h"
#include "agentList.h"
#include "w3cLog.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

#define LOGNAME "[CONFIG] "

using namespace std;
using namespace stringUtil;
using namespace uniString;

config::config() throw()
{
	#define OPTINIT(n) accessor_t(&config::assign_##n, &config::fetch_##n, &config::count_##n, &config::multi_##n, &config::def_##n)

	m_optMap["configrewrite"] = OPTINIT(configRewrite);

	m_optMap["log"] = OPTINIT(log);
	m_optMap["screenlog"] = OPTINIT(screenLog);
	m_optMap["logfile"] = OPTINIT(logFile);
	m_optMap["reallogfile"] = OPTINIT(realLogFile);
	m_optMap["logrotates"] = OPTINIT(logRotates);
	m_optMap["logarchive"] = OPTINIT(logArchive);
	m_optMap["rotateinterval"] = OPTINIT(rotateInterval);

	// networking
	m_optMap["namelookups"] = OPTINIT(nameLookups);
	m_optMap["alternateports"] = OPTINIT(alternatePorts);
	m_optMap["portbase"] = OPTINIT(portBase);
	m_optMap["publicport"] = OPTINIT(publicPort);
	m_optMap["portlegacy"] = OPTINIT(portLegacy);
	m_optMap["autodumptime"] = OPTINIT(autoDumpTime);
	m_optMap["maxheaderlinesize"] = OPTINIT(maxHeaderLineSize);
	m_optMap["maxheaderlinecount"] = OPTINIT(maxHeaderLineCount);
	m_optMap["password"] = OPTINIT(password);
	m_optMap["adminpassword"] = OPTINIT(adminPassword);
    m_optMap["sslcertificatefile"] = OPTINIT(sslCertificateFile);
    m_optMap["sslcertificatekeyfile"] = OPTINIT(sslCertificateKeyFile);

	m_optMap["buffertype"] = OPTINIT(bufferType);
	m_optMap["fixedbuffersize"] = OPTINIT(fixedBufferSize);
	m_optMap["adaptivebuffersize"] = OPTINIT(adaptiveBufferSize);
	m_optMap["bufferhardlimit"] = OPTINIT(bufferHardLimit);
	m_optMap["metainterval"] = OPTINIT(metaInterval);

	m_optMap["adtestfileloop"] = OPTINIT(adTestFileLoop);
	m_optMap["adtestfile"] = OPTINIT(adTestFile);
	m_optMap["adtestfile2"] = OPTINIT(adTestFile2);
	m_optMap["adtestfile3"] = OPTINIT(adTestFile3);
	m_optMap["adtestfile4"] = OPTINIT(adTestFile4);
	m_optMap["introfile"] = OPTINIT(introFile);
	m_optMap["backupfile"] = OPTINIT(backupFile);
	m_optMap["backuptitle"] = OPTINIT(backupTitle);
	m_optMap["backuploop"] = OPTINIT(backupLoop);
	m_optMap["maxspecialfilesize"] = OPTINIT(maxSpecialFileSize);

	m_optMap["artworkfile"] = OPTINIT(artworkFile);

	m_optMap["uvoxcipherkey"] = OPTINIT(uvoxCipherKey);

	//// w3c logging
	m_optMap["w3cenable"] = OPTINIT(w3cEnable);
	m_optMap["w3clog"] = OPTINIT(w3cLog);

	m_optMap["pidfile"] = OPTINIT(pidFile);

	//// relaying
	m_optMap["relayreconnecttime"] = OPTINIT(relayReconnectTime);
	m_optMap["relayconnectretries"] = OPTINIT(relayConnectRetries);
	m_optMap["maxhttpredirects"] = OPTINIT(maxHTTPRedirects);
	m_optMap["allowrelay"] = OPTINIT(allowRelay);
	m_optMap["allowpublicrelay"] = OPTINIT(allowPublicRelay);
	/////

	//// stream configuration
	m_optMap["streamid"] = OPTINIT(stream_ID);
	m_optMap["streamauthhash"] = OPTINIT(stream_authHash);
	m_optMap["streampath"] = OPTINIT(stream_path);
	m_optMap["streamrelayurl"] = OPTINIT(stream_relayURL);
	m_optMap["streambackupurl"] = OPTINIT(stream_backupURL);
	m_optMap["streamminbitrate"] = OPTINIT(stream_minBitrate);
	m_optMap["streammaxbitrate"] = OPTINIT(stream_maxBitrate);
	m_optMap["streammaxuser"] = OPTINIT(stream_maxUser);
	m_optMap["streampassword"] = OPTINIT(stream_password);
	m_optMap["streamadminpassword"] = OPTINIT(stream_adminPassword);
	m_optMap["streampublicserver"] = OPTINIT(stream_publicServer);
	m_optMap["streamallowrelay"] = OPTINIT(stream_allowRelay);
	m_optMap["streamallowpublicrelay"] = OPTINIT(stream_allowPublicRelay);
	m_optMap["streamriponly"] = OPTINIT(stream_ripOnly);
	m_optMap["streamautodumpsourcetime"] = OPTINIT(stream_autoDumpSourceTime);
	m_optMap["streamautodumptime"] = OPTINIT(stream_autoDumpTime);
	m_optMap["streamautodumpusers"] = OPTINIT(stream_autoDumpUsers);
	m_optMap["streamlistenertime"] = OPTINIT(stream_listenerTime);
	m_optMap["streamsonghistory"] = OPTINIT(stream_songHistory);
	m_optMap["streamuvoxcipherkey"] = OPTINIT(stream_uvoxCipherKey);
    m_optMap["streamlogfile"] = OPTINIT(stream_logFile);
	m_optMap["streamadtestfileloop"] = OPTINIT(stream_adTestFileLoop);
	m_optMap["streamadtestfile"] = OPTINIT(stream_adTestFile);
	m_optMap["streamadtestfile2"] = OPTINIT(stream_adTestFile2);
	m_optMap["streamadtestfile3"] = OPTINIT(stream_adTestFile3);
	m_optMap["streamadtestfile4"] = OPTINIT(stream_adTestFile4);
	m_optMap["streamintrofile"] = OPTINIT(stream_introFile);
	m_optMap["streambackupfile"] = OPTINIT(stream_backupFile);
	m_optMap["streambackuptitle"] = OPTINIT(stream_backupTitle);
	m_optMap["streambackuploop"] = OPTINIT(stream_backupLoop);
	m_optMap["streambanfile"] = OPTINIT(stream_banFile);
	m_optMap["streamripfile"] = OPTINIT(stream_ripFile);
	m_optMap["streamagentfile"] = OPTINIT(stream_agentFile);
	m_optMap["streamartworkfile"] = OPTINIT(stream_artworkFile);
	m_optMap["streamw3clog"] = OPTINIT(stream_w3cLog);
	m_optMap["streamhidestats"] = OPTINIT(stream_hideStats);
	m_optMap["streamredirecturl"] = OPTINIT(stream_redirectUrl);
	m_optMap["streammovedurl"] = OPTINIT(stream_movedUrl);
	m_optMap["streamratelimitwait"] = OPTINIT(stream_rateLimitWait);

	m_optMap["requirestreamconfigs"] = OPTINIT(requireStreamConfigs);
	m_optMap["userid"] = OPTINIT(userId);
	m_optMap["licenceid"] = OPTINIT(licenceId);

	//// cdn
	m_optMap["cdn"] = OPTINIT(cdn);
	m_optMap["cdnmaster"] = OPTINIT(cdn_master);
	m_optMap["cdnslave"] = OPTINIT(cdn_slave);

	//// flash
	m_optMap["flashpolicyfile"] = OPTINIT(flashPolicyFile);
	m_optMap["flashpolicyserverport"] = OPTINIT(flashPolicyServerPort);

	//// yp
	m_optMap["yptimeout"] = OPTINIT(ypTimeout);
	m_optMap["ypaddr"] = OPTINIT(ypAddr);
	m_optMap["ypport"] = OPTINIT(ypPort);
	m_optMap["yport"] = OPTINIT(ypPort);
	m_optMap["yppath"] = OPTINIT(ypPath);
	m_optMap["ypmaxretries"] = OPTINIT(ypMaxRetries);
	m_optMap["ypreportinterval"] = OPTINIT(ypReportInterval);
	m_optMap["ypminreportinterval"] = OPTINIT(ypMinReportInterval);
	m_optMap["publicserver"] = OPTINIT(publicServer);

	/// agent
	m_optMap["agentfile"] = OPTINIT(agentFile);
	m_optMap["saveagentlistonexit"] = OPTINIT(saveAgentListOnExit);
	m_optMap["blockemptyuseragent"] = OPTINIT(blockEmptyUserAgent);

	//// stats
	m_optMap["hidestats"] = OPTINIT(hideStats);
	m_optMap["minbitrate"] = OPTINIT(minBitrate);
	m_optMap["maxbitrate"] = OPTINIT(maxBitrate);
	m_optMap["maxuser"] = OPTINIT(maxUser);

	// radionomy metrics
	m_optMap["admetricsdebug"] = OPTINIT(adMetricsDebug);
	m_optMap["metricsmaxqueue"] = OPTINIT(metricsMaxQueue);

	m_optMap["authdebug"] = OPTINIT(authDebug);

	/// client behaviour
	m_optMap["listenertime"] = OPTINIT(listenerTime);
	m_optMap["autodumpusers"] = OPTINIT(autoDumpUsers);
	m_optMap["srcdns"] = OPTINIT(srcIP);
	m_optMap["srcip"] = OPTINIT(srcIP);
	m_optMap["destdns"] = OPTINIT(destIP);
	m_optMap["destip"] = OPTINIT(destIP);
	m_optMap["dstip"] = OPTINIT(destIP);
	m_optMap["publicdns"] = OPTINIT(publicIP);
	m_optMap["publicip"] = OPTINIT(publicIP);
	m_optMap["titleformat"] = OPTINIT(titleFormat);
	m_optMap["urlformat"] = OPTINIT(urlFormat);

	//// banning
	m_optMap["banfile"] = OPTINIT(banFile);
	m_optMap["savebanlistonexit"] = OPTINIT(saveBanListOnExit);

	//// rip
	m_optMap["ripfile"] = OPTINIT(ripFile);
	m_optMap["saveriplistonexit"] = OPTINIT(saveRipListOnExit);
	m_optMap["riponly"] = OPTINIT(ripOnly);

	m_optMap["adminfile"] = OPTINIT(adminFile);

	//// debugging
	m_optMap["webclientdebug"] = OPTINIT(webClientDebug);
	m_optMap["yp2debug"] = OPTINIT(yp2Debug);
	m_optMap["shoutcastsourcedebug"] = OPTINIT(shoutcastSourceDebug);
	m_optMap["uvox2sourcedebug"] = OPTINIT(uvox2SourceDebug);
	m_optMap["httpsourcedebug"] = OPTINIT(HTTPSourceDebug);
	m_optMap["streamdatadebug"] = OPTINIT(streamDataDebug);
	m_optMap["microserverdebug"] = OPTINIT(microServerDebug);
	m_optMap["httpstyledebug"] = OPTINIT(httpStyleDebug);
	m_optMap["shoutcast1clientdebug"] = OPTINIT(shoutcast1ClientDebug);
	m_optMap["shoutcast2clientdebug"] = OPTINIT(shoutcast2ClientDebug);
	m_optMap["httpclientdebug"] = OPTINIT(HTTPClientDebug);
	m_optMap["flvclientdebug"] = OPTINIT(flvClientDebug);
	m_optMap["m4aclientdebug"] = OPTINIT(m4aClientDebug);
	m_optMap["relaydebug"] = OPTINIT(relayDebug);
	m_optMap["relayshoutcastdebug"] = OPTINIT(relayShoutcastDebug);
	m_optMap["relayuvoxdebug"] = OPTINIT(relayUvoxDebug);
	m_optMap["statsdebug"] = OPTINIT(statsDebug);
	m_optMap["threadrunnerdebug"] = OPTINIT(threadRunnerDebug);

	m_optMap["songhistory"] = OPTINIT(songHistory);
	m_optMap["showlastsongs"] = OPTINIT(songHistory);

	///// misc nonsense
	m_optMap["unique"] = OPTINIT(unique);
	m_optMap["include"] = OPTINIT(include);
	m_optMap["cpucount"] = OPTINIT(cpuCount);
	m_optMap["clacks"] = OPTINIT(clacks);
	m_optMap["startinactive"] = OPTINIT(startInactive);
	m_optMap["ratelimit"] = OPTINIT(rateLimit);
	m_optMap["ratelimitwait"] = OPTINIT(rateLimitWait);
	m_optMap["usexff"] = OPTINIT(useXFF);
	m_optMap["logclients"] = OPTINIT(logClients);
	m_optMap["admincssfile"] = OPTINIT(adminCSSFile);
	m_optMap["faviconfile"] = OPTINIT(faviconFile);
	m_optMap["faviconmimetype"] = OPTINIT(faviconFileMimeType);
	m_optMap["robotstxtfile"] = OPTINIT(robotstxtFile);
	m_optMap["redirecturl"] = OPTINIT(redirectUrl);
	m_optMap["forceshortsends"] = OPTINIT(forceShortSends);
	m_optMap["adminnowrap"] = OPTINIT(adminNoWrap);

	// used to control the cache handling
	m_favIconTime = m_styleCustomHeaderTime = 0;
}

// return the streamConfig entries that reference relays
const vector<config::streamConfig> config::getRelayList()
{
	vector<streamConfig> result;
	streams_t stream_configs;
	getStreamConfigs(stream_configs);

	// get all stream configs and then make a vector list of only valid configs
	// which ensures we're only getting known stream configs unlike prior to
	// the behaviour with builds 24 which usually gave an extra config than was
	for (config::streams_t::const_iterator i = stream_configs.begin(); i != stream_configs.end(); ++i)
	{
		if ((*i).second.m_relayUrl.isSet())
		{
			result.push_back((*i).second);
		}
	}

	return result;
}

// return the streamConfig entry that references the backup url asked
const vector<config::streamConfig> config::getBackupUrl(const size_t streamID) throw(exception)
{
	vector<streamConfig> result;
	streams_t stream_configs;
	getStreamConfigs(stream_configs);
	config::streams_t::const_iterator i = stream_configs.find(streamID);

	if (i != stream_configs.end())
	{
		if ((*i).second.m_backupUrl.isSet())
		{
			result.push_back((*i).second);
		}
	}

	return result;
}

unsigned short config::getMetaInterval(const size_t streamID) const throw()
{
	unsigned short metainterval = stream_metaInterval(streamID);
	if (!read_stream_metaInterval(streamID))
	{
		metainterval = metaInterval();
	}

	// don't allow less than 256 and
	// due to overflow we don't need
	// to do an upper check as it'll
	// have wrapped around for this.
	return (metainterval < 256 ? 256 : metainterval);
}

int config::getBackupLoop(const size_t streamID) const throw()
{
	int backuploop = stream_backupLoop(streamID);
	if (!read_stream_backupLoop(streamID))
	{
		backuploop = backupLoop();
	}
	return backuploop;
}

int config::getRateLimitWait(const size_t streamID) const throw()
{
	int ratelimitwait = stream_rateLimitWait(streamID);
	if (!read_stream_rateLimitWait(streamID))
	{
		ratelimitwait = rateLimitWait();
	}
	// just to make sure that we're giving a sane value
	return (ratelimitwait > 0 ? ratelimitwait : def_rateLimitWait().toInt());
}

const int config::isBitrateDisallowed(const size_t streamID, const int bitrate,
									  int &streamMinBitrate, int &streamMaxBitrate) const throw()
{
	int ret = 0;

	// check that these bitrates are allowed (looking at both max and average values)
	streamMinBitrate = stream_minBitrate(streamID);
	if (!read_stream_minBitrate(streamID) || !streamMinBitrate)
	{
		streamMinBitrate = minBitrate();
	}

	if ((streamMinBitrate > 0) && (bitrate < streamMinBitrate))
	{
		ret |= 1;
	}

	streamMaxBitrate = stream_maxBitrate(streamID);
	if (!read_stream_maxBitrate(streamID) || !streamMaxBitrate)
	{
		streamMaxBitrate = maxBitrate();
	}

	if ((streamMaxBitrate > 0) && (bitrate > streamMaxBitrate))
	{
		ret |= 2;
	}

	return ret;
}

size_t config::getSongHistorySize(const size_t streamID) const throw()
{
	size_t songhistory = stream_songHistory(streamID);
	if (!read_stream_songHistory(streamID))
	{
		songhistory = songHistory();
	}
	return songhistory;
}

const int config::getAutoDumpTime(const size_t streamID) const throw()
{
	size_t sid = (!streamID ? streamID : DEFAULT_SOURCE_STREAM);
	int dumpTime = stream_autoDumpTime(sid);
	if (!read_stream_autoDumpTime(sid) || !dumpTime)
	{
		dumpTime = autoDumpTime();
	}
	return dumpTime;
}

const int config::getCPUCount() const throw()
{
    int cpu_count = cpuCount();	                // check options

    if (cpu_count < 1)
    {
        int maxclients = gOptions.maxUser();
        if (maxclients < 1100)
            cpu_count = (maxclients/350) +1;
        else
        {
            int hwcpus = cpucount();
            if (hwcpus < 5)
                cpu_count = hwcpus;
            else
                cpu_count = (((int)::log10 ((double)maxclients) - 2) * 6) - 2; // should be range of 4-12
        }
    }
    if (cpu_count < 1) cpu_count = 2;			// eh? can never be less than 1
    return cpu_count;
}

bool config::setupPasswords(const streams_t &stream_configs) throw(exception)
{
	// now form the per stream versions of passwords if there are any
	bool passwordError = false;
	for (streams_t::const_iterator i = stream_configs.begin(); i != stream_configs.end(); ++i)
	{
		utf8 tempAdminPassword = (*i).second.m_adminPassword;
		if (tempAdminPassword.empty())
		{
			tempAdminPassword = gOptions.adminPassword();
		}

		utf8 tempPassword = (*i).second.m_password;
		if (tempPassword.empty())
		{
			tempPassword = gOptions.password();
		}

		// follow legacy behaviour (just incase)
		if (tempAdminPassword.empty())
		{
			tempAdminPassword = tempPassword;
		}
		else
		{
			// otherwise if explicitly set as the same then we abort
			if (tempAdminPassword == tempPassword)
			{
				WLOG(logSectionName() + "Stream " + tos((*i).first) + " should not have matching passwords for `adminpassword' and `password'.");
			}
		}

		if (tempAdminPassword.empty() && tempPassword.empty())
		{
			ELOG(logSectionName() + "Stream " + tos((*i).first) + " does not have any passwords specified.");
			passwordError = true;
		}
	}

	return passwordError;
}

config::streamConfig& config::getPerStreamConfig(streamConfig& stream, const size_t sid, const bool useParent)
{
	// tweak the maxuser setting on the stream so it can be made to follow the
	// per stream setting or for it to revert to the global server limit
	int tempMaxUser = native_fetch_stream_maxUser(sid);
	if (!tempMaxUser || (tempMaxUser > gOptions.maxUser()))
	{
		if (useParent)
		{
			tempMaxUser = 0;
		}
	}

	int tempMaxBitrate = native_fetch_stream_maxBitrate(sid);
	if (!read_stream_maxBitrate(sid))
	{
		if (useParent)
		{
			tempMaxBitrate = gOptions.maxBitrate();
		}
	}

	int tempMinBitrate = native_fetch_stream_minBitrate(sid);
	if (!read_stream_minBitrate(sid))
	{
		if (useParent)
		{
			tempMinBitrate = gOptions.minBitrate();
		}
	}

	bool tempAllowRelay = native_fetch_stream_allowRelay(sid);
	if (!read_stream_allowRelay(sid))
	{
		if (useParent)
		{
			tempAllowRelay = gOptions.allowRelay();
		}
	}

	bool tempAllowPublicRelay = native_fetch_stream_allowPublicRelay(sid);
	if (!read_stream_allowPublicRelay(sid))
	{
		if (useParent)
		{
			tempAllowPublicRelay = gOptions.allowPublicRelay();
		}
	}

	utf8 tempAdminPassword = native_fetch_stream_adminPassword(sid);
	if (tempAdminPassword.empty())
	{
		if (useParent)
		{
			tempAdminPassword = gOptions.adminPassword();
		}
	}

	utf8 tempPassword = native_fetch_stream_password(sid);
	if (tempPassword.empty())
	{
		if (useParent)
		{
			tempPassword = gOptions.password();
		}
	}

	utf8 tempPublicServer = native_fetch_stream_publicServer(sid);
	if (tempPublicServer.empty())
	{
		if (useParent)
		{
			tempPublicServer = gOptions.publicServer();
		}
	}

	return (stream = streamConfig(native_fetch_stream_ID(sid), native_fetch_stream_authHash(sid),
								  native_fetch_stream_path(sid), native_fetch_stream_relayURL(sid),
								  native_fetch_stream_backupURL(sid), tempMaxUser, tempMaxBitrate,
								  tempMinBitrate, tempAdminPassword, tempPassword,
								  tempPublicServer, tempAllowRelay, tempAllowPublicRelay));
}

// return a streamConfig entry for the streamID	
const bool config::getStreamConfig(streamConfig& stream, const size_t streamID)
{
	stackLock sml(m_lock);

	map<size_t, size_t>::iterator i = m_stream_ID.find(streamID);
	if (i != m_stream_ID.end())
	{
		config::getPerStreamConfig(stream, streamID);
		return true;
	}

	return false;
}

// return all streamConfig entries organized by streamID	
void config::getStreamConfigs(streams_t& streams, const bool useParent)
{
	stackLock sml(m_lock);

	for (map<size_t, size_t>::const_iterator i = m_stream_ID.begin(); i != m_stream_ID.end(); ++i)
	{
		streamConfig stream;
		streams[native_fetch_stream_ID((*i).second)] = getPerStreamConfig(stream, (*i).second, useParent);
	}


}

// attempt to update an existing stream configuration with the new data
// and then attempt to pass it to the currently active stream based on it
__uint64 config::updateStreamConfig(config &readConfig, streamConfig update) throw(exception)
{
	if (!update.m_streamID)
	{
		return 0;
	}

	stackLock sml(m_lock);
	size_t streamID = update.m_streamID;
	__uint64 updated = 0;

	if (native_fetch_stream_authHash(streamID) != update.m_authHash)
	{
		native_assign_stream_authHash(streamID, update.m_authHash);
		updated |= AUTH_HASH;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("AUTH_HASH");
		#endif
	}

	if (native_fetch_stream_path(streamID) != update.m_urlPath)
	{
		native_assign_stream_path(streamID, update.m_urlPath);
		updated |= URL_PATH;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("URL_PATH");
		#endif
	}
	
	if (native_fetch_stream_relayURL(streamID) != update.m_relayUrl.url())
	{
		native_assign_stream_relayURL(streamID, update.m_relayUrl.url());
		updated |= RELAY_URL;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("RELAY_URL");
		#endif
	}

	if (native_fetch_stream_backupURL(streamID) != update.m_backupUrl.url())
	{
		native_assign_stream_backupURL(streamID, update.m_backupUrl.url());
		updated |= BACKUP_URL;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("BACKUP_URL");
		#endif
	}

	if (native_fetch_stream_maxUser(streamID) != update.m_maxStreamUser)
	{
		native_assign_stream_maxUser(streamID, update.m_maxStreamUser);
		updated |= MAX_USER;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("MAX_USER");
		#endif
	}

	if (native_fetch_stream_maxBitrate(streamID) != update.m_maxStreamBitrate)
	{
		native_assign_stream_maxBitrate(streamID, update.m_maxStreamBitrate);
		updated |= MAX_BITRATE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("MAX_BITRATE");
		#endif
	}

	if (native_fetch_stream_minBitrate(streamID) != update.m_minStreamBitrate)
	{
		native_assign_stream_minBitrate(streamID, update.m_minStreamBitrate);
		updated |= MIN_BITRATE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("MIN_BITRATE");
		#endif
	}

	utf8 publicServer = readConfig.stream_publicServer(streamID);
	if (native_fetch_stream_publicServer(streamID) != publicServer)
	{
		if (!readConfig.read_stream_publicServer(streamID))
		{
			publicServer = gOptions.publicServer();
		}
		else
		{
			updated |= PUBLIC_SRV;
			#if defined(_DEBUG) || defined(DEBUG)
			ELOG("PUBLIC_SRV");
			#endif
		}
		native_assign_stream_publicServer(streamID, publicServer);
	}

	bool allowRelay = readConfig.stream_allowRelay(streamID);
	if (native_fetch_stream_allowRelay(streamID) != allowRelay)
	{
		if (!readConfig.read_stream_allowRelay(streamID))
		{
			allowRelay = gOptions.allowRelay();
		}
		native_assign_stream_allowRelay(streamID, allowRelay);
		updated |= ALLOW_RELAY;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("ALLOW_RELAY");
		#endif
	}

	bool allowPublicRelay = readConfig.stream_allowPublicRelay(streamID);
	if (native_fetch_stream_allowPublicRelay(streamID) != allowPublicRelay)
	{
		if (!readConfig.read_stream_allowPublicRelay(streamID))
		{
			allowPublicRelay = gOptions.allowPublicRelay();
		}
		native_assign_stream_allowPublicRelay(streamID, allowPublicRelay);
		updated |= ALLOW_PUBLIC_RELAY;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("ALLOW_PUBLIC_RELAY");
		#endif
	}

	bool ripOnly = readConfig.stream_ripOnly(streamID);
	if (native_fetch_stream_ripOnly(streamID) != ripOnly)
	{
		if (!readConfig.read_stream_ripOnly(streamID))
		{
			ripOnly = gOptions.ripOnly();
		}
		native_assign_stream_ripOnly(streamID, ripOnly);
		updated |= RIP_ONLY;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("RIP_ONLY");
		#endif
	}

	int autoDumpTime = readConfig.stream_autoDumpTime(streamID);
	if (native_fetch_stream_autoDumpTime(streamID) != autoDumpTime)
	{
		if (!readConfig.read_stream_autoDumpTime(streamID))
		{
			autoDumpTime = gOptions.autoDumpTime();
		}
		native_assign_stream_autoDumpTime(streamID, autoDumpTime);
		updated |= DUMP_TIME;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("DUMP_TIME");
		#endif
	}

	bool autoDumpUsers = readConfig.stream_autoDumpUsers(streamID);
	if (native_fetch_stream_autoDumpUsers(streamID) != autoDumpUsers)
	{
		if (!readConfig.read_stream_autoDumpUsers(streamID))
		{
			autoDumpUsers = gOptions.autoDumpUsers();
		}
		native_assign_stream_autoDumpUsers(streamID, autoDumpUsers);
		updated |= DUMP_USER;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("DUMP_USER");
		#endif
	}

	size_t listenerTime = readConfig.stream_listenerTime(streamID);
	if (native_fetch_stream_listenerTime(streamID) != listenerTime)
	{
		if (!readConfig.read_stream_listenerTime(streamID))
		{
			listenerTime = gOptions.listenerTime();
		}
		native_assign_stream_listenerTime(streamID, listenerTime);
		updated |= LIST_TIME;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("LIST_TIME");
		#endif
	}

	int songHistory = readConfig.stream_songHistory(streamID);
	if (native_fetch_stream_songHistory(streamID) != songHistory)
	{
		if (!readConfig.read_stream_songHistory(streamID))
		{
			songHistory = gOptions.songHistory();
		}
		native_assign_stream_songHistory(streamID, songHistory);
		updated |= SONG_HIST;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("SONG_HIST");
		#endif
	}

	utf8 uvoxCipherKey = readConfig.stream_uvoxCipherKey(streamID);
	if (native_fetch_stream_uvoxCipherKey(streamID) != uvoxCipherKey)
	{
		if (!readConfig.read_stream_uvoxCipherKey(streamID))
		{
			uvoxCipherKey = gOptions.uvoxCipherKey();
		}
		native_assign_stream_uvoxCipherKey(streamID, uvoxCipherKey);
		updated |= CIPHER_KEY;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("CIPHER_KEY");
		#endif
	}

	// passwords are handled slightly differently as clearing of the
	// password will lead to an invalid scenario so we check and block
	utf8 streamPassword = readConfig.stream_password(streamID);
	if (native_fetch_stream_password(streamID) != streamPassword)
	{
		if (!readConfig.read_stream_password(streamID))
		{
			// clear what was being previously stored if possible
			m_stream_password[streamID].clear();
			updated |= SOURCE_PWD;
		}
		else
		{
			// if empty then set back to the master password
			if(streamPassword.empty())
			{
				streamPassword = readConfig.password();
			}

			// if empty then set back to the master admin password
			if(streamPassword.empty())
			{
				ELOG(LOGNAME "'password' cannot be set to empty. Not applying this change to stream config# " + tos(streamID));
			}
			else
			{
				native_assign_stream_password(streamID, streamPassword);
				updated |= SOURCE_PWD;
			}
		}

		#if defined(_DEBUG) || defined(DEBUG)
		if (updated & SOURCE_PWD)
		{
			ELOG("SOURCE_PWD");
		}
		#endif
	}

	utf8 adminPassword = readConfig.stream_adminPassword(streamID);
	if (native_fetch_stream_adminPassword(streamID) != adminPassword)
	{
		if (!readConfig.read_stream_adminPassword(streamID))
		{
			// clear what was being previously stored if possible
			m_stream_adminPassword[streamID].clear();
			updated |= ADMIN_PWD;
		}
		// if empty then set back to the master admin password
		else if (adminPassword.empty())
		{
			ELOG(LOGNAME "'adminpassword' cannot be set to empty. Not applying this change to stream config# " + tos(streamID));
		}
		else
		{
			native_assign_stream_adminPassword(streamID, adminPassword);
			updated |= ADMIN_PWD;
		}

		#if defined(_DEBUG) || defined(DEBUG)
		if (updated & ADMIN_PWD)
		{
			ELOG("ADMIN_PWD");
		}
		#endif
	}

	utf8 movedUrl = readConfig.stream_movedUrl(streamID);
	if (native_fetch_stream_movedUrl(streamID) != movedUrl)
	{
		native_assign_stream_movedUrl(streamID, movedUrl);
		updated |= MOVED_URL;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("MOVED_URL");
		#endif
	}

	utf8 artworkFile = readConfig.stream_artworkFile(streamID);
	if (native_fetch_stream_artworkFile(streamID) != artworkFile)
	{
		if (!readConfig.read_stream_artworkFile(streamID))
		{
			artworkFile = gOptions.artworkFile();
		}
		native_assign_stream_artworkFile(streamID, artworkFile);
		updated |= ARTWORK_FILE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("ARTWORK_FILE");
		#endif
	}

	utf8 adTestFile = readConfig.stream_adTestFile(streamID);
	if (native_fetch_stream_adTestFile(streamID) != adTestFile)
	{
		if (!readConfig.read_stream_adTestFile(streamID))
		{
			adTestFile = gOptions.adTestFile();
		}
		native_assign_stream_adTestFile(streamID, adTestFile);
		updated |= AD_TEST_FILE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("AD_TEST_FILE");
		#endif
	}

	utf8 adTestFile2 = readConfig.stream_adTestFile2(streamID);
	if (native_fetch_stream_adTestFile2(streamID) != adTestFile2)
	{
		if (!readConfig.read_stream_adTestFile2(streamID))
		{
			adTestFile2 = gOptions.adTestFile2();
		}
		native_assign_stream_adTestFile2(streamID, adTestFile2);
		updated |= AD_TEST_FILE_2;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("AD_TEST_FILE_2");
		#endif
	}

	utf8 adTestFile3 = readConfig.stream_adTestFile3(streamID);
	if (native_fetch_stream_adTestFile3(streamID) != adTestFile3)
	{
		if (!readConfig.read_stream_adTestFile3(streamID))
		{
			adTestFile3 = gOptions.adTestFile3();
		}
		native_assign_stream_adTestFile3(streamID, adTestFile3);
		updated |= AD_TEST_FILE_3;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("AD_TEST_FILE_3");
		#endif
	}

	utf8 adTestFile4 = readConfig.stream_adTestFile4(streamID);
	if (native_fetch_stream_adTestFile4(streamID) != adTestFile4)
	{
		if (!readConfig.read_stream_adTestFile4(streamID))
		{
			adTestFile4 = gOptions.adTestFile4();
		}
		native_assign_stream_adTestFile4(streamID, adTestFile4);
		updated |= AD_TEST_FILE_4;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("AD_TEST_FILE_4");
		#endif
	}

	int adTestFileLoop = readConfig.stream_adTestFileLoop(streamID);
	if (native_fetch_stream_adTestFileLoop(streamID) != adTestFileLoop)
	{
		if (!readConfig.read_stream_adTestFileLoop(streamID))
		{
			adTestFileLoop = gOptions.adTestFileLoop();
		}
		native_assign_stream_adTestFileLoop(streamID, adTestFileLoop);
		updated |= AD_TEST_FILE_LOOP;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("AD_TEST_FILE_LOOP");
		#endif
	}

	utf8 introFile = readConfig.stream_introFile(streamID);
	if (native_fetch_stream_introFile(streamID) != introFile)
	{
		if (!readConfig.read_stream_introFile(streamID))
		{
			introFile = gOptions.introFile();
		}
		native_assign_stream_introFile(streamID, introFile);
		updated |= INTRO_FILE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("INTRO_FILE");
		#endif
	}

	utf8 backupFile = readConfig.stream_backupFile(streamID);
	if (native_fetch_stream_backupFile(streamID) != backupFile)
	{
		if (!readConfig.read_stream_backupFile(streamID))
		{
			backupFile = gOptions.backupFile();
		}
		native_assign_stream_backupFile(streamID, backupFile);
		updated |= BACKUP_FILE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("BACKUP_FILE");
		#endif
	}

	utf8 backupTitle = readConfig.stream_backupTitle(streamID);
	if (native_fetch_stream_backupTitle(streamID) != backupTitle)
	{
		if (!readConfig.read_stream_backupTitle(streamID))
		{
			backupTitle = gOptions.backupTitle();
		}
		native_assign_stream_backupTitle(streamID, backupTitle);
		updated |= BACKUP_TITLE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("BACKUP_TITLE");
		#endif
	}

	utf8 banFile = readConfig.stream_banFile(streamID);
	if (native_fetch_stream_banFile(streamID) != banFile)
	{
		if (!readConfig.read_stream_banFile(streamID))
		{
			m_stream_banFile[streamID].clear();
			gOptions.unread_stream_banFile(streamID);

			// unload the IPs from the list
			g_banList.remove("", 0, streamID, true);
		}
		else
		{
			// load the IPs from the list
			size_t sID = (!banFile.empty() ? streamID : 0);
			g_banList.load(banFile,sID);
		}
		native_assign_stream_banFile(streamID, banFile);
		updated |= BAN_FILE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("BAN_FILE");
		#endif
	}

	utf8 ripFile = readConfig.stream_ripFile(streamID);
	if (native_fetch_stream_ripFile(streamID) != ripFile)
	{
		if (!readConfig.read_stream_ripFile(streamID))
		{
			m_stream_ripFile[streamID].clear();
			gOptions.unread_stream_ripFile(streamID);

			// unload the IPs from the list
			g_ripList.remove("",streamID,true);
		}
		else
		{
			// load the IPs from the list
			size_t sID = (!ripFile.empty() ? streamID : 0);
			g_ripList.load(ripFile,sID);
		}
		native_assign_stream_ripFile(streamID, ripFile);
		updated |= RIP_FILE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("RIP_FILE");
		#endif
	}

	utf8 agentFile = readConfig.stream_agentFile(streamID);
	if (native_fetch_stream_agentFile(streamID) != agentFile)
	{
		if (!readConfig.read_stream_agentFile(streamID))
		{
			m_stream_agentFile[streamID].clear();
			gOptions.unread_stream_agentFile(streamID);

			// unload the agents from the list
			g_agentList.remove("", streamID, true);
		}
		else
		{
			// load the agents from the list
			size_t sID = (!agentFile.empty() ? streamID : 0);
			g_agentList.load(agentFile, sID);
		}
		native_assign_stream_agentFile(streamID, agentFile);
		updated |= AGENT_FILE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("AGENT_FILE");
		#endif
	}

	utf8 w3cLog = readConfig.stream_w3cLog(streamID);
	if (native_fetch_stream_w3cLog(streamID) != w3cLog)
	{
		if (!readConfig.read_stream_w3cLog(streamID))
		{
			m_stream_w3cLog[streamID].clear();
			gOptions.unread_stream_w3cLog(streamID);
			w3cLog::close(streamID);
		}
		else
		{
			// could have just changed so update as needed
			utf8 oldw3cLog = native_fetch_stream_w3cLog(streamID);
			if (!oldw3cLog.empty())
			{
				w3cLog::close(streamID);
			}
			size_t sID = (!w3cLog.empty() ? streamID : 0);
			if (gOptions.w3cEnable())
			{
				w3cLog::open(w3cLog, sID);
			}
		}
		native_assign_stream_w3cLog(streamID, w3cLog);
		updated |= W3C_FILE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("W3C_FILE");
		#endif
	}

	utf8 hideStats = readConfig.stream_hideStats(streamID);
	if (native_fetch_stream_hideStats(streamID) != hideStats)
	{
		if (!readConfig.read_stream_hideStats(streamID))
		{
			hideStats = gOptions.hideStats();
		}
		native_assign_stream_hideStats(streamID, hideStats);
		updated |= HIDE_STATS;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("HIDE_STATS");
		#endif
	}

	int cdnMaster = readConfig.cdn_master(streamID);
	if (native_fetch_cdn_master(streamID) != cdnMaster)
	{
		if (!readConfig.read_cdn_master(streamID))
		{
			cdnMaster = -1;
		}
		native_assign_cdn_master(streamID, cdnMaster);
		updated |= CDN_MASTER;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("CDN_MASTER");
		#endif
	}

	int cdnSlave = readConfig.cdn_slave(streamID);
	if (native_fetch_cdn_slave(streamID) != cdnSlave)
	{
		if (!readConfig.read_cdn_slave(streamID))
		{
			cdnSlave = -1;
		}
		native_assign_cdn_slave(streamID, cdnSlave);
		updated |= CDN_SLAVE;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("CDN_SLAVE");
		#endif
	}

	int backupLoop = readConfig.stream_backupLoop(streamID);
	if (native_fetch_stream_backupLoop(streamID) != backupLoop)
	{
		if (!readConfig.read_stream_backupLoop(streamID))
		{
			backupLoop = gOptions.backupLoop();
		}
		native_assign_stream_backupLoop(streamID, backupLoop);
		updated |= DUMP_USER;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("BACKUP_LOOP");
		#endif
	}

	int rateLimitWait = readConfig.stream_rateLimitWait(streamID);
	if (native_fetch_stream_rateLimitWait(streamID) != rateLimitWait)
	{
		if (!readConfig.read_stream_rateLimitWait(streamID))
		{
			rateLimitWait = gOptions.rateLimitWait();
		}
		native_assign_stream_rateLimitWait(streamID, rateLimitWait);
		updated |= RATE_LIMIT_WAIT;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("RATE_LIMIT_WAIT");
		#endif
	}

	int metainterval = readConfig.stream_metaInterval(streamID);
	if (native_fetch_stream_metaInterval(streamID) != metainterval)
	{
		if (!readConfig.read_stream_metaInterval(streamID))
		{
			metainterval = gOptions.metaInterval();
		}
		native_assign_stream_metaInterval(streamID, metainterval);
		updated |= METAINTERVAL;
		#if defined(_DEBUG) || defined(DEBUG)
		ELOG("METAINTERVAL");
		#endif
	}

	if (updated)
	{
		ILOG(LOGNAME "Updates applied to stream config# " + tos(streamID) + " [code: 0x" + tohex(updated) + "]");
	}
	else
	{
		ILOG(LOGNAME "No updates required for stream config# " + tos(streamID));
	}
	return updated;
}

void config::addStreamConfig(config &readConfig, streamConfig add) throw(exception)
{
	if (!add.m_streamID)
	{
		return;
	}

	stackLock sml(m_lock);
	size_t streamID = add.m_streamID;

	native_assign_stream_ID(streamID, streamID);
	native_assign_stream_adminPassword(streamID, add.m_adminPassword);
	native_assign_stream_allowPublicRelay(streamID, add.m_allowPublicRelay);
	native_assign_stream_allowRelay(streamID, add.m_allowRelay);
	native_assign_stream_authHash(streamID, add.m_authHash);
	native_assign_stream_password(streamID, add.m_password);
	native_assign_stream_publicServer(streamID, add.m_publicServer);
	native_assign_stream_maxBitrate(streamID, add.m_maxStreamBitrate);
	native_assign_stream_minBitrate(streamID, add.m_minStreamBitrate);
	native_assign_stream_maxUser(streamID, add.m_maxStreamUser);
	native_assign_stream_relayURL(streamID, add.m_relayUrl.url());
	native_assign_stream_backupURL(streamID, add.m_backupUrl.url());
	native_assign_stream_path(streamID, add.m_urlPath);

	if (gOptions.w3cEnable())
	{
		utf8 w3cLog = readConfig.stream_w3cLog(streamID);
		if (readConfig.read_stream_w3cLog(streamID))
		{
			if (gOptions.w3cEnable())
			{
				w3cLog::open(w3cLog, streamID);
			}
		}
	}

	if (gOptions.saveBanListOnExit())
	{
		if (readConfig.read_stream_banFile(streamID) && !gOptions.stream_banFile(streamID).empty())
		{
			g_banList.save(gOptions.stream_banFile(streamID),streamID);
		}
	}

	if (gOptions.saveRipListOnExit())
	{
		if (readConfig.read_stream_ripFile(streamID) && !gOptions.stream_ripFile(streamID).empty())
		{
			g_ripList.save(gOptions.stream_ripFile(streamID),streamID);
		}
	}

	if (gOptions.saveAgentListOnExit())
	{
		if (readConfig.read_stream_agentFile(streamID) && !gOptions.stream_agentFile(streamID).empty())
		{
			g_agentList.save(gOptions.stream_agentFile(streamID),streamID);
		}
	}

	ILOG(LOGNAME "Added stream config# " + tos(streamID));
}

void config::removeStreamConfig(streamConfig remove) throw(exception)
{
	if (!remove.m_streamID)
	{
		return;
	}

	stackLock sml(m_lock);
	size_t streamID = remove.m_streamID;

	// clean up the per-stream files
	if (gOptions.w3cEnable())
	{
		w3cLog::close(streamID);
	}

	if (gOptions.saveBanListOnExit())
	{
		if (gOptions.read_stream_banFile(streamID) && !gOptions.stream_banFile(streamID).empty())
		{
			g_banList.save(gOptions.stream_banFile(streamID),streamID);
		}
	}

	if (gOptions.saveRipListOnExit())
	{
		if (gOptions.read_stream_ripFile(streamID) && !gOptions.stream_ripFile(streamID).empty())
		{
			g_ripList.save(gOptions.stream_ripFile(streamID),streamID);
		}
	}

	if (gOptions.saveAgentListOnExit())
	{
		if (gOptions.read_stream_agentFile(streamID) && !gOptions.stream_agentFile(streamID).empty())
		{
			g_agentList.save(gOptions.stream_agentFile(streamID),streamID);
		}
	}

	// now clear out where possible the per-stream values
	gOptions.unread_stream_ID(streamID);
	gOptions.unread_stream_adminPassword(streamID);
	gOptions.unread_stream_agentFile(streamID);
	gOptions.unread_stream_allowPublicRelay(streamID);
	gOptions.unread_stream_allowRelay(streamID);
	gOptions.unread_stream_authHash(streamID);
	gOptions.unread_stream_autoDumpTime(streamID);
	gOptions.unread_stream_autoDumpUsers(streamID);
	gOptions.unread_stream_backupFile(streamID);
	gOptions.unread_stream_backupTitle(streamID);
	gOptions.unread_stream_backupLoop(streamID);
	gOptions.unread_stream_banFile(streamID);
	gOptions.unread_stream_introFile(streamID);
	gOptions.unread_stream_listenerTime(streamID);
	gOptions.unread_stream_maxBitrate(streamID);
	gOptions.unread_stream_minBitrate(streamID);
	gOptions.unread_stream_maxUser(streamID);
	gOptions.unread_stream_password(streamID);
	gOptions.unread_stream_path(streamID);
	gOptions.unread_stream_publicServer(streamID);
	gOptions.unread_stream_relayURL(streamID);
	gOptions.unread_stream_backupURL(streamID);
	gOptions.unread_stream_ripFile(streamID);
	gOptions.unread_stream_ripOnly(streamID);
	gOptions.unread_stream_agentFile(streamID);
	gOptions.unread_stream_songHistory(streamID);
	gOptions.unread_stream_uvoxCipherKey(streamID);
	gOptions.unread_stream_w3cLog(streamID);

	gOptions.unread_cdn_master(streamID);
	gOptions.unread_cdn_slave(streamID);

	map<size_t, size_t>::iterator i = m_stream_ID.find(streamID);
	if (i != m_stream_ID.end())
	{
		m_stream_ID.erase(i);
	}

	ILOG(LOGNAME "Removed stream config# " + tos(streamID));
}

config::~config() throw()
{
}

static void write_option(const utf8 &name, const utf8 &value, FILE *f,
						 const uniFile::filenameType &fn, bool &written) throw(exception)
{
	const char *delimiter = "=";
	const size_t delimiter_size = 1;
	size_t name_size = name.size();
	size_t value_size = value.size();
	size_t eol_size = eol().size();

	if (written && (::fwrite(eol().c_str(), 1, eol_size, f) != eol_size))
	{
		throw runtime_error("I/O error writing to file " + fn.hideAsString());
	}
	if (::fwrite(name.c_str(), 1, name_size, f) != name_size)
	{
		throw runtime_error("I/O error writing to file " + fn.hideAsString());
	}
	if (::fwrite(delimiter, 1, delimiter_size, f) != delimiter_size)
	{
		throw runtime_error("I/O error writing to file " + fn.hideAsString());
	}
	if (::fwrite(value.c_str(), 1, value_size, f) != value_size)
	{
		throw runtime_error("I/O error writing to file " + fn.hideAsString());
	}
	written = true;
}

std::string config::logSectionName()
{
	return LOGNAME;
}

bool config::rewriteConfigurationFile(bool minimal, bool messages, bool setup) const throw(exception)
{
	if (!cdn().empty())
	{
		WLOG(LOGNAME "CDN mode prevents re-writing of the configuration file");
		return false;
	}

	if (messages)
	{
		ILOG(LOGNAME "Rewriting config file");
	}

	stackLock sml(m_lock);

	// add any config options to be ignored when re-writing
	set<utf8> ignore_set;
	ignore_set.insert("streamid");
	ignore_set.insert("include");
	ignore_set.insert("dstip");
	ignore_set.insert("yport");
	ignore_set.insert("showlastsongs");
	ignore_set.insert("streambitrate");
	ignore_set.insert("relayserver");
	ignore_set.insert("relayport");
	ignore_set.insert("reallogfile");
	ignore_set.insert("clacks");
	if (flashPolicyServerPort() == -1)
	{
		ignore_set.insert("flashpolicyserverport");
	}
	if (portLegacy() == -1)
	{
		ignore_set.insert("portlegacy");
	}
	if (publicPort() == -1)
	{
		ignore_set.insert("publicport");
	}

	bool saved = true;
	FILE *f = 0;
	try
	{
		uniFile::filenameType fn = _confFile();
		// create a backup of the current config file and then use that
		if (fileUtil::fileExists(fn))
		{
			uniFile::filenameType fn_backup(_confFile() + ".backup");
			uniFile::unlink(fn_backup);
			#ifdef _WIN32
			if (!::MoveFileW(fn.toWString().c_str(), fn_backup.toWString().c_str()))
			{
				throw runtime_error("Cannot create backup of the current config file `" +
									fn.hideAsString() + "' (" + errMessage().hideAsString() + ")");
			}
			#else
			if (::rename(fn.hideAsString().c_str(), fn_backup.hideAsString().c_str()) < 0)
			{
				throw runtime_error("Cannot create backup of the current config file `" +
									fn.hideAsString() + "' (" + errMessage().hideAsString() + ")");
			}
			#endif
		}

		f = uniFile::fopen(fn,"wb");
		if (!f)
		{
			throw runtime_error("Cannot open config file `" + fn.hideAsString() +
								"' for writing (" + errMessage().hideAsString() + ")");
		}

		map<utf8, utf8> deferredOptions = m_deferredOptions;
		bool written = false;
		for (optMap_t::const_iterator i = m_optMap.begin(); i != m_optMap.end(); ++i)
		{
			const utf8 &optName = (*i).first;
			const accessor_t &a = (*i).second;

			if (ignore_set.find(optName) != ignore_set.end())
			{
				continue;
			}

			map<utf8,utf8>::iterator deferred_i = deferredOptions.find(optName);
			if (deferred_i != deferredOptions.end())
			{
				if (!minimal || (!((*deferred_i).second == (this->*a.m_defaultFunc)())))
				{
					write_option(optName,(*deferred_i).second, f, fn, written);
				}
				deferredOptions.erase(deferred_i);
			}
			else
			{
				if ((this->*a.m_multiFunc)())
				{
					const size_t num = (this->*a.m_countFunc)();
					for (size_t x = 0; x < num; ++x)
					{
						size_t index = 0;
						if (!minimal || (!((this->*a.m_fetchFunc)(x, &index) == (this->*a.m_defaultFunc)())))
						{
							write_option(optName + "_" + tos(index), (this->*a.m_fetchFunc)(index, 0), f, fn, written);
						}
					}
				}
				else
				{
					if (!minimal || (!((this->*a.m_fetchFunc)(1, 0) == (this->*a.m_defaultFunc)())))
					{
						write_option(optName,(this->*a.m_fetchFunc)(1, 0), f, fn, written);
					}
				}
			}
		}

		// remaining deferred options
		for (map<utf8,utf8>::const_iterator i = deferredOptions.begin(); i != deferredOptions.end(); ++i)
		{
			if (ignore_set.find((*i).first) != ignore_set.end())
			{
				continue;
			}

			optMap_t::const_iterator m = m_optMap.find((*i).first);
			const accessor_t &a = (*m).second;

			if (!minimal || (!((*i).second == (this->*a.m_defaultFunc)())))
			{
				write_option((*i).first, (*i).second, f, fn, written);
			}
		}
		deferredOptions.clear();

		::fclose(f);
	}
	catch(const exception &ex)
	{
		saved = false;
		ELOG(utf8(setup ? "[SETUP] " : LOGNAME) + ex.what());
		if (f)
		{
			::fclose(f);
		}
	}
	return saved;
}

utf8 config::dumpConfigFile() throw()
{
	utf8 streams, general, debug;

	stackLock sml(m_lock);

	// add any config options to be ignored when re-writing
	set<utf8> ignore_set;
	ignore_set.insert("streamid");
	ignore_set.insert("include");
	ignore_set.insert("dstip");
	ignore_set.insert("yport");
	ignore_set.insert("showlastsongs");
	ignore_set.insert("streambitrate");
	ignore_set.insert("relayserver");
	ignore_set.insert("relayport");
	ignore_set.insert("reallogfile");
	if (flashPolicyServerPort() == -1)
	{
		ignore_set.insert("flashpolicyserverport");
	}
	if (portLegacy() == -1)
	{
		ignore_set.insert("portlegacy");
	}
	if (publicPort() == -1)
	{
		ignore_set.insert("publicport");
	}

	for (optMap_t::const_iterator i = m_optMap.begin(); i != m_optMap.end(); ++i)
	{
		const utf8 &optName = (*i).first;
		const accessor_t &a = (*i).second;

		if (ignore_set.find(optName) != ignore_set.end())
		{
			continue;
		}

		if ((this->*a.m_multiFunc)())
		{
			size_t num = (this->*a.m_countFunc)();
			for (size_t x = 0; x < num; ++x)
			{
				size_t index = 0;
				if (!((this->*a.m_fetchFunc)(x, &index) == (this->*a.m_defaultFunc)()))
				{
					if (optName.find((utf8)"stream") == 0)
					{
						streams += optName + "_" + tos(index) + "=<b>" + (this->*a.m_fetchFunc)(index, 0) + "</b><br>";
					}
					else if (optName.find((utf8)"debug") != utf8::npos)
					{
						debug += optName + "_" + tos(index) + "=<b>" + (this->*a.m_fetchFunc)(index, 0) + "</b><br>";
					}
					else
					{
						general += optName + "_" + tos(index) + "=<b>" + (this->*a.m_fetchFunc)(index, 0) + "</b><br>";
					}
				}
			}
		}
		else
		{
			if (!((this->*a.m_fetchFunc)(1, 0) == (this->*a.m_defaultFunc)()))
			{
				if (optName.find((utf8)"debug") != utf8::npos)
				{
					debug += optName + "=<b>" + (this->*a.m_fetchFunc)(1, 0) + "</b><br>";
				}
				else
				{
					general += optName + "=<b>" + (this->*a.m_fetchFunc)(1, 0) + "</b><br>";
				}
			}
		}
	}

	return "<div style=\"float:left;padding:0 1em;\">" +
		   (!streams.empty() ? "<br><b><u>Stream Settings</u>:</b><br><br>" : (utf8)"") +
		   streams + "<br></div><div style=\"float:left;padding:0 1em;\">" +
		   (!general.empty() ? "<br><b><u>General Settings</u>:</b><br><br>" : "") +
		   general + "<br></div><div style=\"float:left;padding:0 1em;\">" +
		   (!debug.empty() ? "<br><b><u>Debugging Settings</u>:</b><br><br>" : "") +
		   debug + "<br></div>";
}

int config::promptConfigFile() throw()
{
	if (!sDaemon)
	{
		#ifdef _WIN32
		vector<wstring> fileList = fileUtil::directoryFileList(gStartupDirectory.toWString() + L"*.ini", L"", true, true);
		vector<wstring> fileListConf = fileUtil::directoryFileList(gStartupDirectory.toWString() + L"*.conf", L"", true, true);
		#else
		vector<string> fileList = fileUtil::directoryFileList(gStartupDirectory.hideAsString() + "*.ini", "");
		vector<string> fileListConf = fileUtil::directoryFileList(gStartupDirectory.hideAsString() + "*.conf", "");
		#endif

		if (!fileList.empty())
		{
			#ifdef _WIN32
			// exclude desktop.ini on windows builds as that can cause other issues
			for (vector<wstring>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
			{
				if ((*i) == L".\\desktop.ini")
				{
					fileList.erase(i);
					break;
				}
			}
			#endif
			fileList.insert(fileList.end(), fileListConf.begin(), fileListConf.end());
		}
		else
		{
			fileList = fileListConf;
		}

		if (!fileList.empty())
		{
			ILOG(LOGNAME "Choose one of the listed config file(s) to load or");
			ILOG(LOGNAME "enter `s' to enable the setup mode for the DNAS or");
#if CONFIG_BUILDER
			ILOG(LOGNAME "enter `b' to enable the builder mode for the DNAS or");
#endif
			ILOG(LOGNAME "enter `x' to close this instance of the DNAS:");
			ILOG(LOGNAME "Note: Press `Enter' after choosing the config file.");
			if (fileList.size() > 10)
			{
				ILOG(LOGNAME "      Only the first 10 config files detected will be shown.");
			}

			int option = 0;
			if (fileList.size() > 10)
			{
				fileList.resize(10);
			}
			#ifdef _WIN32
			for (vector<wstring>::const_iterator i = fileList.begin(); i != fileList.end(); ++i, option++)
			#else
			for (vector<string>::const_iterator i = fileList.begin(); i != fileList.end(); ++i, option++)
			#endif
			{
				#ifdef _WIN32
				utf32 u32file(*i);
				utf8 u8f(u32file.toUtf8().substr(1 + u32file.rfind(utf32("\\"))));
				ILOG(LOGNAME " [" + tos(option) + "]  :  " + u8f + (fileUtil::getSuffix(*i) == L"ini" ? " [1.x config file]" : ""));
				#else
				ILOG(LOGNAME " [" + tos(option) + "]  :  " + (*i).substr(1 + (*i).rfind("/")) + (fileUtil::getSuffix(*i) == "ini" ? " [1.x config file]" : ""));
				#endif
			}

			string input;
			cin >> input;

			while ((!input.empty() && (input[0] != 'x') && (input[0] != 's') && (!isdigit(input[0]) || !((size_t)(input[0] - '0') < fileList.size()))))
			{
				ELOG(LOGNAME "You have entered an invalid option. Please enter a number matching the config file required.");
				input.clear();
				cin >> input;
			}

			if (input[0] == 'x')
			{
				return -2;
			}

			if (input[0] == 's')
			{
				ILOG(LOGNAME "Entering setup mode. Open 127.0.0.1:8000/setup in a");
				ILOG(LOGNAME "browser on the same machine the DNAS is started on.");
				return 2;
			}
#ifdef CONFIG_BUILDER
			if (input[0] == 'b')
			{
				ILOG(LOGNAME "Entering builder mode. Open 127.0.0.1:8000/builder in");
				ILOG(LOGNAME "a browser on the same machine the DNAS is started on.");
				return 2;
			}
#endif
			if (!input.empty() && isdigit(input[0]))
			{
				size_t index = input[0] - '0';
				// clamped to 0 - 9 (fileList can be larger but input is only 0-9)
				if (index < fileList.size())
				{
					#ifdef _WIN32
					utf32 u32file(fileList[index]);
					utf8 u8f(u32file.toUtf8().substr(2));
					return load(u8f);
					#else
					return load(fileList[index]);
					#endif
				}
			}
		}
		else
		{
			return -1;
		}
	}
	#ifndef _WIN32
	else
	{
		// if running as a daemon then make checks just for the default
		// if we've not had a configuration file specified to be used.
		vector<string> fileList = fileUtil::directoryFileList(gStartupDirectory.hideAsString() + "sc_serv.ini", "");
		vector<string> fileListConf = fileUtil::directoryFileList(gStartupDirectory.hideAsString() + "sc_serv.conf", "");

		if (!fileList.empty())
		{
			fileList.insert(fileList.end(), fileListConf.begin(), fileListConf.end());
		}
		else
		{
			fileList = fileListConf;
		}

		if (!fileList.empty())
		{
			load(fileList[0]);
		}
	}
	#endif
	return 0;
}

bool config::load(const uniFile::filenameType &filename, bool load) throw()
{
	stackLock sml(m_lock);

	// set the option that returns the name of the config file
	m_confFile[DEFAULT_CLIENT_STREAM_ID] = filename;

	// only need to set the filename when creating a new file
	if (!load)
	{
		return true;
	}

	// check for 'setup' and abort config file loading
#ifdef CONFIG_BUILDER
	if ((filename == "builder") || (filename == "setup"))
#else
	if (filename == "setup")
#endif
	{
		return false;
	}

	// setup the legacy relay config options to cope with reloads, etc
	m_legacyRelayServer.clear();
	m_legacyRelayPort = ":80";

	bool result = _load(filename, (utf8)"", true);

	// if the server hasn't been read then skip over this as it's
	// likely there wasn't any legacy relay config options read
	if (!m_legacyRelayServer.empty() && !(m_legacyRelayServer == "http://"))
	{
		// make sure that the old relay url is added by force setting streamid=1
		optMap_t::const_iterator i = m_optMap.find("streamid");
		(this->*((*i).second.m_assignFunc))(tos(DEFAULT_CLIENT_STREAM_ID), DEFAULT_CLIENT_STREAM_ID);

		utf8 legacyRelay = m_legacyRelayServer + m_legacyRelayPort;
		i = m_optMap.find("streamrelayurl");
		(this->*((*i).second.m_assignFunc))(legacyRelay, DEFAULT_CLIENT_STREAM_ID);
	}

	// use this to map the v2.0.0 value to that from 2.0.1+
	if (hideStats() == "1")
	{
		optMap_t::const_iterator i = m_optMap.find("hidestats");
		(this->*((*i).second.m_assignFunc))("stats", 0);
	}

	// detect a failed attempt to load a config file and ensure we follow defaults on fail
	if (m_log.empty())
	{
		m_log[DEFAULT_CLIENT_STREAM_ID] = true;
	}
	if (result == true)
	{
		if (m_logFile.empty())
		{
			m_log[DEFAULT_CLIENT_STREAM_ID] = true;
		}
		else if (m_logFile[DEFAULT_CLIENT_STREAM_ID].size() < 1)
		{
			m_log[DEFAULT_CLIENT_STREAM_ID] = false;
		}
		else if (compareStringsWithoutCase(m_logFile[0], uniFile::filenameType("none")))
		{
			m_log[DEFAULT_CLIENT_STREAM_ID] = false;
		}
		else if (compareStringsWithoutCase(m_logFile[0], uniFile::filenameType("/dev/null")))
		{
			m_log[DEFAULT_CLIENT_STREAM_ID] = false;
		}
	}
	return result;
}

// load options from a config file. Generally only done at startup.
// Config file should be in utf8 format
const bool config::_load(const uniFile::filenameType &filename, const utf8& uniqueStr, const bool parent = false) throw()
{
	const utf8 unique_matching_token("$");
	utf8 unique(unique_matching_token);
	if (uniqueStr.empty())
	{
		unique = unique_matching_token;
	}
	else
	{
		unique = uniqueStr;
	}

	int l = 0;	// line counter
	bool loaded = true;

	// open it. Use the deferred messages since this happens at startup
	FILE *conf = uniFile::fopen(filename, "rb");
	if (!conf)
	{
		loaded = false;
		m_deferredWarnLogMessages.push_back(LOGNAME "Could not find `" + fileUtil::getFullFilePath(filename) +
											(parent == true ? "' - looking for config file to load..." : "'"));
		goto no_read_conf;
	}

	// parse each line of the file
	while (true)
	{
		int subIndex = DEFAULT_CLIENT_STREAM_ID; // for items of form xxxxxx_#  (multi-options)
		char buffer[4096] = {0};

		if (!fgets(buffer, sizeof(buffer), conf))
		{
			break; // get a line
		}

		size_t offset = strlen(buffer) - 1;
		while (buffer[offset] == '\n' || buffer[offset] == '\r')
		{
			buffer[offset]='\0'; // get rid of cr/lf
		}
		++l; // increment line counter

		char *pbuffer = buffer;
		while (pbuffer && (*pbuffer == ' ' || *pbuffer == '\t'))
		{
			++pbuffer; // remove trailing whitespace
		}

		if (!pbuffer || !*pbuffer || (*pbuffer == ';') || (*pbuffer == '[') || (*pbuffer == '#'))
		{
			continue; // blank lines or comments skipped
		}

		char *tok = pbuffer;
		while (pbuffer && *pbuffer && (*pbuffer != '=') && (*pbuffer != '\r'))
		{
			++pbuffer; // look for = sign
		}
		if (!pbuffer || !*pbuffer)
		{ 
			m_deferredWarnLogMessages.push_back(LOGNAME "Invalid statement on line " + tos(l) + " of " + filename + " -> `" + utf8(buffer) + "'");
			continue;
		}
		*pbuffer++=0;

		// skip utf-8 BOM
		if ((strlen(tok) > 2) &&
			(((unsigned char*)tok)[0] == 0xef) &&
			(((unsigned char*)tok)[1] == 0xbb) &&
			(((unsigned char*)tok)[2] == 0xbf))
		{
			tok += 3;
		}

		utf8 stok = tok;
		// see if it's a multi option and set the subIndex value
		const vector<utf8> tokens = tokenizer(stok, '_');
		if (tokens.size() == 2)
		{
			stok = tokens[0];
			subIndex = atoi((const char *)stripWhitespace(tokens[1]).c_str());

			// this ensure the subIndex has been specified even if there is no streamid
			// this is to fix a load of config setup issues from people not setting it.
			optMap_t::const_iterator sid = m_optMap.find("streamid");
			(this->*((*sid).second.m_assignFunc))(tos(subIndex), subIndex);
		}
		stok = stripWhitespace(stok); // cleanup some more whitespace just in case

		// look for the base option name in the option map
		utf8 base_option_name = toLower(stok);
		if (base_option_name.empty())
		{
			// skip over empty values
			continue;
		}

		// used for mapping *autodumpsourcetime to *autodumptime
		int autoDumpTime = 0;
		if (base_option_name == "autodumpsourcetime")
		{
			base_option_name = "autodumptime";
			autoDumpTime = 1;
		}
		else if (base_option_name == "streamautodumpsourcetime")
		{
			base_option_name = "streamautodumptime";
			autoDumpTime = 2;
		}
		else if (base_option_name == "reallogfile" ||
				 base_option_name == "autoauthhash")
		{
			// skip over as not pubiically exposed
			continue;
		}

		optMap_t::const_iterator i = m_optMap.find(base_option_name);
		bool legacyRelay = false;
		bool stream_bitrate = (base_option_name == "streambitrate");
		if (i == m_optMap.end())
		{
			if (!((base_option_name == "relayserver") || (base_option_name == "relayport") || stream_bitrate))
			{
				m_deferredWarnLogMessages.push_back(LOGNAME "Invalid item on line " + tos(l) + " of " + filename + " -> `" + base_option_name + "'");
				continue;
			}
			else
			{
				if (!stream_bitrate)
				{
					legacyRelay = true;
				}
			}
		}
		else
		{
			// do we have a stream* entry and no stream id mentioned / working against default
			// if yes then force streamid=1 to be set so that the options will be recognised
			uniString::utf8::size_type spos = base_option_name.find(utf8("stream"));
			if ((spos != uniString::utf8::npos) && (spos == 0) && (subIndex == DEFAULT_CLIENT_STREAM_ID))
			{
				// this ensure the subIndex has been specified even if there is no streamid
				// this is to fix a load of config setup issues from people not setting it.
				const optMap_t::const_iterator sid = m_optMap.find("streamid");
				(this->*((*sid).second.m_assignFunc))(tos(subIndex), subIndex);
			}
		}

		// move tok to the value
		tok = pbuffer;
		while (tok && *tok && *tok == ' ')
		{
			++tok;
		}

		utf8 value = stripWhitespace(utf8(tok));

		// if we're re-mapping *autodumpsourcetime then indicate in the log output
		if (autoDumpTime > 0)
		{
			m_deferredWarnLogMessages.push_back(LOGNAME "Deprecated statement found on line " + tos(l) + " of " +
												filename + " -> change " + (autoDumpTime == 2 ? "stream" : "") +
												"autodumpsourcetime_" + tos(subIndex) + "=" + value + " to " +
												(autoDumpTime == 2 ? "stream" : "") + "autodumptime_" +
												tos(subIndex) + "=" + value);
		}

		// this is used in the mapping of the old ocnfig options to the new style streamrelayurl
		if (legacyRelay == true)
		{
			if (base_option_name == "relayserver")
			{
				utf8::size_type check = (!value.empty() ? value.find(utf8("http://")) : 0);
				if (check == utf8::npos)
				{
					m_legacyRelayServer = "http://" + value;
				}
				else
				{
					m_legacyRelayServer = value;
				}
			}
			if (base_option_name == "relayport")
			{
				utf8::size_type check = value.find(utf8(":"));
				if ((check == utf8::npos) || (check != 0))
				{
					m_legacyRelayPort = ":" + value;
				}
				else
				{
					m_legacyRelayPort = value;
				}
			}

			continue;
		}

		// check for yes / no values and map them to 1 or 0 to cope with legacy config loading
		if ((base_option_name == "w3cenable") ||
			(base_option_name == "allowrelay") ||
			(base_option_name == "allowpublicrelay") ||
			(base_option_name == "riponly"))
		{
			if (toLower(value) == "yes")
			{
				value = "1";
			}
			if (toLower(value) == "no")
			{
				value = "0";
			}
		}

		// validation checks on the 'cdn' parameter to ensure we're all good else where
		if (base_option_name == "cdn")
		{
			value = toLower(value);
			if (!(value == "on") && !(value == "always") && !(value == "master"))
			{
				value = "";
			}
		}

		if (stream_bitrate)
		{
			const optMap_t::const_iterator min = m_optMap.find("streamminbitrate");
			(this->*((*min).second.m_assignFunc))(value, subIndex);
			const optMap_t::const_iterator max = m_optMap.find("streammaxbitrate");
			(this->*((*max).second.m_assignFunc))(value, subIndex);
			continue;
		}

		// validation checks on the port parameters to detect the 'any'
		// case which is meant to be treated the same as an empty value
		if ((base_option_name == "srcdns") ||
			(base_option_name == "srcip") ||
			(base_option_name == "destdns") ||
			(base_option_name == "destip") ||
			(base_option_name == "dstip") ||
			(base_option_name == "publicdns") ||
			(base_option_name == "publicip"))
		{
			if (toLower(value) == "any")
			{
				value = "";
			}
		}

		// check for streampath values and fix to have / on the start to ensure it will
		// work for client connections without impacting on client connection time, etc
		if (base_option_name == "streampath")
		{
			utf8::size_type path = value.find(utf8(":"));
			if (path != utf8::npos)
			{
				m_deferredWarnLogMessages.push_back(LOGNAME "Ignoring streampath_" + tos(subIndex) + "=" + value + " as this will produce an invalid path.");
				value.clear();
				continue;
			}

			path = value.find(utf8("var/www"));
			// catches var/www... or /var/www... style paths
			// which are best just filtered as it gives away
			// paths on the host machine which looks crappy!
			if ((path != utf8::npos) && (path < 2))
			{
				m_deferredWarnLogMessages.push_back(LOGNAME "Ignoring streampath_" + tos(subIndex) + "=" + value + " as this will produce an invalid path.");
				value.clear();
				continue;
			}			

			// check for empty as we can incorrectly end up
			// setting streampath_xx=/ which causes issues!
			if (!value.empty())
			{
				path = value.find(utf8("/"));
				if ((path == utf8::npos) || (path != 0))
				{
					value = "/" + value;
				}
			}

			// additionally we check for specific 'default' paths and block their usage
			// based on a match against the start of the streampath (might block some
			// possibly valid streampaths but it'll avoid stupid configuration issues).
			const char * disallowed[] = { "/listen.pls", "/listen.m3u", "/listen.asx", "/listen.xspf",
										  "/listen.qtl", "/listen", "/7.html", "/index.html",
										  "/played", "/played.html", "/admin.cgi", "/statistics",
										  "/stats", "/streamart", "/playingart", "/nextsong", "/home",
										  "/home.html", "/nextsongs", "/currentsong", "/shoutcast.swf",
										  "/crossdomain.xml", "/index.css", "/images/", "/favicon.ico",
										  "/robots.txt", "/images/favicon.ico", "/images/listen.png",
										  "/images/history.png", "/images/lock.png", "/images/noadavail.png",
										  "/images/streamart.png", "/images/adavail.png", "/images/v2.png",
										  "/images/playingart.png", "/adplayed/adplayed.png", "/images/v1.png",
										  "/images/relay.png", "/images/wa.png", "/images/chrome.png",
										  "/images/firefox.png", "/images/safari.png", "/images/ie.png",
										  "/images/vlc.png", "/images/fb2k.png", "/images/wmp.png",
										  "/images/icecast.png", "/images/html5.png", "/images/rtb.png",
										  "/images/ps.png", "/images/mplayer.png", "/images/apple.png",
										  "/images/roku.png", "/images/itunes.png", "/images/warn.png",
										  "/images/xff.png", "/images/radionomy.png", "/images/curl.png",
										  "/images/flash.png", "/images/synology.png", "/images/wiimc.png"
			};
			for (size_t x = 0; x < sizeof(disallowed) / sizeof(disallowed[0]); x++)
			{
				path = value.find(utf8(disallowed[x]));
				if ((path != utf8::npos) && !path)
				{
					m_deferredWarnLogMessages.push_back(LOGNAME "Ignoring streampath_" + tos(subIndex) + "=" + value + " as this contains a reserved path.");
					value.clear();
					continue;
				}
			}
		}

		// check for redirecturl and streamredirecturl and ensure there is a http://
		// otherwise we need to process multiple times on page hits which isn't good
		if ((base_option_name == "redirecturl" || base_option_name == "streamredirecturl") && !value.empty())
		{
			utf8::size_type url = (!value.empty() ? value.find(utf8("http://")) : 0);
			if (url == utf8::npos)
			{
				value = "http://" + value;
			}
		}

		// check the passwords for validity e.g. they're not allowed to contain a colon
		// as that can break the multiple-1.x source support as well as 2.x source join
		if (base_option_name == "password" || base_option_name == "streampassword" ||
			base_option_name == "adminpassword" || base_option_name == "streamadminpassword")
		{
			if (value.find(utf8(":")) != utf8::npos)
			{
				m_deferredErrorLogMessages.push_back(LOGNAME "`" + base_option_name +
													 (subIndex > 1 ? "_" + tos(subIndex) : "") +
													 "' contains a reserved character and will be ignored. "
													 "Please remove all colons to resolve this issue.");
				value.clear();
				continue;
			}
		}

		// special case to mimic old sc_serv conf file behaviour. In old sc_serv, the conf variable "unique" can
		// be subsituted for $ in any value related to a filename
		if (base_option_name == "unique")
		{
			unique = value;
		}

		if ((base_option_name == "logfile") ||
			(base_option_name == "adtestfile") ||
			(base_option_name == "adtestfile2") ||
			(base_option_name == "adtestfile3") ||
			(base_option_name == "adtestfile4") ||
			(base_option_name == "streamadtestfile") ||
			(base_option_name == "streamadtestfile2") ||
			(base_option_name == "streamadtestfile3") ||
			(base_option_name == "streamadtestfile4") ||
			(base_option_name == "introfile") ||
			(base_option_name == "streamintrofile") ||
			(base_option_name == "backupfile") ||
			(base_option_name == "streambackupfile") ||
			(base_option_name == "banfile") ||
			(base_option_name == "streambanfile") ||
			(base_option_name == "ripfile") ||
			(base_option_name == "streamripfile") ||
			(base_option_name == "agentfile") ||
			(base_option_name == "artworkfile") ||
			(base_option_name == "streamagentfile") ||
			(base_option_name == "streamartworkfile") ||
			(base_option_name == "include") ||
			(base_option_name == "w3clog") ||
			(base_option_name == "streamw3clog") ||
			(base_option_name == "portbase"))
		{
			if (unique != unique_matching_token)
			{
				utf8::size_type pos = value.find(unique_matching_token);
				while (pos != utf8::npos)
				{
					value.replace(pos, 1, unique);
					pos = value.find(unique_matching_token);
				}
			}

			// attempt to convert \ to / and vice versa as needed for cross-platform sharing of configuration files
			fileUtil::convertOSFilePathDelimiter(value);
		}

		if (base_option_name == "include")
		{
			// get the current folder so we can load from include=common.conf if using full paths, etc
			uniFile::filenameType currentPath = fileUtil::onlyPath(filename);

			// this will handle wildcard matching as applicable to what has been set in the 'include'
			// value so will allow us to have individual configs via 'include=./stream/stream_*.conf'
			#ifdef _WIN32
			vector<wstring> fileList = fileUtil::directoryFileList(value.toWString(), currentPath.toWString(), true, true);
			#else
			vector<string> fileList = fileUtil::directoryFileList(value.hideAsString(), currentPath.hideAsString());
			#endif
			if (!fileList.empty())
			{
				#ifdef _WIN32
				for (vector<wstring>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
				#else
				for (vector<string>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
				#endif
				{
					#ifdef _WIN32
					utf32 u32file(*i);
					utf8 u8f(u32file.toUtf8());
					_load(u8f, unique);
					#else
					_load(*i, unique);
					#endif
				}
			}
		}
		else
		{
			(this->*((*i).second.m_assignFunc))(value, subIndex);
		}
	}

	// close the config file
	::fclose(conf);

no_read_conf:
	return loaded;
}

bool config::editConfigFileEntry(size_t sid, const uniFile::filenameType &filename,
								 const uniString::utf8 &authhash,
								 const uniString::utf8 &param, const bool add,
								 bool &handled, bool &idHandled,
								 const bool parent = false) throw()
{
	bool loaded = true;
	FILE *conf = NULL, *newconf = NULL;
	char buffer[4096] = {0};
	char copyBuffer[4096] = {0};
 	char *pbuffer = buffer;
	int l = 0;	// line counter

	// create a backup of the current config file and then use that
	uniFile::filenameType fn(filename + ".backup");
	uniFile::unlink(fn);
	#ifdef _WIN32
	if (!::MoveFileW(filename.toWString().c_str(), fn.toWString().c_str()))
	#else
	if (::rename(filename.hideAsString().c_str(), fn.hideAsString().c_str()))
	#endif
	{
		loaded = false;
		ELOG(LOGNAME "Could not backup original file `" + filename + "'");
		goto no_write_conf;
	}

	conf = uniFile::fopen(filename+".backup","rb");
	if (!conf)
	{
		loaded = false;
		ELOG(LOGNAME "Could not open `" + filename + "'(" + errMessage().hideAsString() + ")");
		goto no_write_conf;
	}

	newconf = uniFile::fopen(filename,"wb");
	if (!newconf)
	{
		loaded = false;
		ELOG(LOGNAME "Could not open `" + filename+".backup' (" + errMessage().hideAsString() + ")");
		goto no_write_conf;
	}

	// parse each line of the file
	while (true)
	{
		size_t subIndex = DEFAULT_CLIENT_STREAM_ID; // for items of form xxxxxx_#  (multi-options)

		if (!fgets(buffer, sizeof(buffer), conf))
		{
			break; // get a line
		}
		memcpy(copyBuffer, buffer, sizeof(copyBuffer));

		size_t offset = strlen(buffer)-1;
		while (buffer[offset]=='\n' || buffer[offset]=='\r')
		{
			buffer[offset]='\0'; // get rid of cr/lf
		}
		++l; // increment line counter

		pbuffer = buffer;
		while (pbuffer && (*pbuffer == ' ' || *pbuffer == '\t'))
		{
			++pbuffer; // remove trailing whitespace
		}

		if (!pbuffer || !*pbuffer || *pbuffer == ';' || *pbuffer == '[' || *pbuffer == '#')
		{
			fwrite(copyBuffer,1,strlen(copyBuffer),newconf);
			continue; // blank lines or comments skipped
		}

		char *tok = pbuffer;
		while (pbuffer && *pbuffer && *pbuffer != '=' && *pbuffer != '\r')
		{
			++pbuffer; // look for = sign
		}
		if (!pbuffer || !*pbuffer) 
		{
			fwrite(copyBuffer,1,strlen(copyBuffer),newconf);
			continue;
		}
		*pbuffer++=0;

		// skip utf-8 BOM
		if ((strlen(tok) > 2) &&
			(((unsigned char*)tok)[0] == 0xef) &&
			(((unsigned char*)tok)[1] == 0xbb) &&
			(((unsigned char*)tok)[2] == 0xbf))
		{
			tok += 3;
		}

		utf8 stok = tok;
		// see if it's a multi option and set the subIndex value
		const vector<utf8> tokens = tokenizer(stok, '_');
		if (tokens.size() == 2)
		{
			stok = tokens[0];
			subIndex = atoi((const char *)stripWhitespace(tokens[1]).c_str());
		}
		stok = stripWhitespace(stok); // cleanup some more whitespace just in case

		// look for the base option name in the option map
		utf8 base_option_name = toLower(stok);
		optMap_t::const_iterator i = m_optMap.find(base_option_name);
		if (i == m_optMap.end())
		{
			fwrite(copyBuffer,1,strlen(copyBuffer),newconf);
			continue;
		}

		// move tok to the value
		tok = pbuffer;
		while (tok && *tok && *tok == ' ')
		{
			++tok;
		}

		utf8 value = stripWhitespace(utf8(tok));

		// this will attempt to check if streamid is present, otherwise we need to add otherwise
		// when the DNAS is restarted then it will not find the authhash despite being in there
		if (base_option_name == "streamid" && subIndex == sid && add == true)
		{
			idHandled = true;
		}

		// this will attempt to update a dummy entry in the config or remove depending on the mode
		if (base_option_name == "streamauthhash" && subIndex == sid)
		{
			// attempt to match the line breaks already in the file so it's consistent
			if (add == true)
			{
				// default to the current platforms encoding so it'll at least have a line break
				utf8 readEol = eol();
				if (copyBuffer[0])
				{
					// determine if just \n or \r\n
					size_t offset = strlen(copyBuffer)-1;
					if (copyBuffer[offset] == '\n')
					{
						if (copyBuffer[offset-1] == '\r')
						{
							readEol = "\r\n";
						}
						else
						{
							readEol = "\n";
						}
					}
					else if (copyBuffer[offset] == '\r')
					{
						readEol = "\r";
					}
				}

				uniString::utf8 newEntry("streamauthhash_"+tos(sid)+"="+authhash+readEol);
				fwrite(newEntry.c_str(),1,newEntry.size(),newconf);
			}
			handled = true;
		}
		// this will attempt to update a dummy entry in the config or remove depending on the mode
		else if (base_option_name == authhash && !sid)
		{
			// attempt to match the line breaks already in the file so it's consistent
			if (add == true)
			{
				// default to the current platforms encoding so it'll at least have a line break
				utf8 readEol = eol();
				if (copyBuffer[0])
				{
					// determine if just \n or \r\n
					size_t offset = strlen(copyBuffer)-1;
					if (copyBuffer[offset] == '\n')
					{
						if (copyBuffer[offset-1] == '\r')
						{
							readEol = "\r\n";
						}
						else
						{
							readEol = "\n";
						}
					}
					else if (copyBuffer[offset] == '\r')
					{
						readEol = "\r";
					}
				}

				if (param == "1")
				{
					uniString::utf8 newEntry(authhash+"="+param+eol());
					fwrite(newEntry.c_str(),1,newEntry.size(),newconf);
				}
			}
			handled = true;
		}
		else
		{
			fwrite(copyBuffer,1,strlen(copyBuffer),newconf);
		}

		if (base_option_name == "include")
		{
			// get the current folder so we can load from include=common.conf if using full paths, etc
			uniFile::filenameType currentPath = fileUtil::onlyPath(filename);

			// this will handle wildcard matching as applicable to what has been set in the 'include'
			// value so will allow us to have individual configs via 'include=./stream/stream_*.conf'
			#ifdef _WIN32
			vector<wstring> fileList = fileUtil::directoryFileList(value.toWString(), currentPath.toWString(), true, true);
			#else
			vector<string> fileList = fileUtil::directoryFileList(value.hideAsString(), currentPath.hideAsString());
			#endif
			if (!fileList.empty())
			{
				#ifdef _WIN32
				for (vector<wstring>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
				#else
				for (vector<string>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
				#endif
				{
					#ifdef _WIN32
					utf32 u32file(*i);
					utf8 u8f(u32file.toUtf8());
					editConfigFileEntry(sid, u8f, authhash, param, add, handled, idHandled);
					#else
					editConfigFileEntry(sid, *i, authhash, param, add, handled, idHandled);
					#endif
				}
			}
		}
	}

	// only save back to the first config file if nothing was done in any included files
	if (parent == true && add == true && (handled == false || idHandled == false))
	{
		if (sid && idHandled == false)
		{
			// look at and append a newline if there isn't one already at the config's eof
			if (copyBuffer[0])
			{
				size_t len = strlen(copyBuffer)-1;
				if (copyBuffer[len] != '\r' && copyBuffer[len] != '\n')
				{
					fwrite(eol().c_str(),1,eol().size(),newconf);
				}
			}
			idHandled = true;
		}

		if (sid && handled == false)
		{
			// look at and append a newline if there isn't one already at the config's eof
			if (copyBuffer[0])
			{
				size_t len = strlen(copyBuffer)-1;
				if (copyBuffer[len] != '\r' && copyBuffer[len] != '\n')
				{
					fwrite(eol().c_str(),1,eol().size(),newconf);
				}
			}
			uniString::utf8 newEntry("streamauthhash_"+tos(sid)+"="+authhash+eol());
			fwrite(newEntry.c_str(),1,newEntry.size(),newconf);
			handled = true;
		}

		// for anything else, if it's at the default then no need to add, just remove it
		if (!sid && handled == false && (param == "1"))
		{
			// look at and append a newline if there isn't one already at the config's eof
			if (copyBuffer[0])
			{
				size_t len = strlen(copyBuffer)-1;
				if (copyBuffer[len] != '\r' && copyBuffer[len] != '\n')
				{
					fwrite(eol().c_str(),1,eol().size(),newconf);
				}
			}
			uniString::utf8 newEntry(authhash+"="+param+eol());
			fwrite(newEntry.c_str(),1,newEntry.size(),newconf);
			handled = true;
		}
	}

	// close the config file
	::fclose(conf);
	::fclose(newconf);

no_write_conf:
	return loaded;
}

/////////////////////////////////////////////////
///// service template interface methods ////////
/////////////////////////////////////////////////

const vector<utf8> config::fromArgs(const vector<utf8> &cl) throw()
{
	vector<utf8> result;
	// only attempt to load a passed configuration file otherwise beforehand
	// this would cause sc_serv to load and appear to do nothing so make sure
	// that we let the defaults stay in effect so it will do work correctly
	if (!cl.empty())
	{
		load(cl.front());
		vector<utf8>::const_iterator i = cl.begin();
		++i;
		while (i != cl.end())
		{
			result.push_back(*(i++));
		}
	}

	#ifdef _WIN32
	if (_logFile() == DEFAULT_LOG)
	{
		// this will fill in the default log path as required
		wchar_t m_fileName[MAX_PATH] = {0};
		ExpandEnvironmentStringsW(DEFAULT_LOGW, m_fileName, MAX_PATH);
		assign_logFile(utf32(m_fileName).toUtf8(), DEFAULT_CLIENT_STREAM_ID);
	}
	#endif

	return result;
}

bool config::getConsoleLogging() const throw()
{
	stackLock sml(m_lock);
	return (!sDaemon ? _screenLog() && _log() : false);
}

const uniFile::filenameType config::getFileLog() const throw()
{
	stackLock sml(m_lock);

	static const uniFile::filenameType empty;
	return (_log() ? _logFile() : empty);
}

#ifdef _WIN32
utf8 config::getSystemLogConfigString() throw() { return AOL_logger::systemLogger_element::panicConfiguration(); }
#else
utf8 config::getSystemLogConfigString() throw() { return "";}
#endif

utf8 config::getVersionBuildStrings() throw()
{
	static utf8 version = "";
	if (version.empty())
	{
#ifdef _WIN32
		getVersionInfo(version);
#else
		for (int x = 0; x < VENT; ++x)
		{
			if (x)
			{
				version += ".";
			}
			version += tos(PRODUCTVERSION[x]);
		}
#endif
#ifdef LICENCE_FREE
        version += " no-licence-check";
#endif
	}
	return version;
}

utf8 config::streamConfig::urlObj::parse(const utf8 &in_url, utf8 &out_server, u_short &out_port, utf8 &out_path) throw(exception)
{
	utf8 url(in_url);

	// quick out
	if (in_url.empty())
	{
		out_server.clear();
		out_port = 0;
		out_path.clear();
		return in_url;
	}
	/////////////

	utf8 server;
	utf8 path("/");
	u_short port = 80;

	if (url.empty())
	{
		throwEx<runtime_error>(LOGNAME "Parse error in url (" + url + ")");
	}

	url = stripHTTPprefix(url);

	utf8::size_type pos = url.find(utf8(":")),
					pos2 = url.find(utf8("/"));

	if ((pos != utf8::npos) && ((pos2 == utf8::npos) || (pos < pos2)))
	{
		// port
		server = url.substr(0, pos);
		url.erase(0,pos + 1);
		pos2 = url.find(utf8("/"));
		port = utf8(url.substr(0, pos2)).toInt();
		url.erase(0,pos2);
	}
	else
	{
		server = url.substr(0,pos2);
		url.erase(0,pos2);
	}

	if (!url.empty())
	{
		path = url;
	}
	out_server = server;
	out_port = port;
	out_path = path;
	return in_url;
}

uniString::utf8 config::getCrossDomainFile(const bool compressed) throw()
{
	if (m_crossdomainStr.empty())
	{
		utf8 body = loadLocalFile(fileUtil::getFullFilePath(flashPolicyFile()));
		if (body.empty())
		{
			utf8 ports = tos(portBase());
			if (g_legacyPort >= 1 && g_legacyPort <= 65535)
			{
				ports += "," + tos(g_legacyPort);
			}
			if (!m_usedAlternatePorts.empty())
			{
				ports += m_usedAlternatePorts;
			}

			body = "<?xml version=\"1.0\"?>\n<!DOCTYPE cross-domain-policy SYSTEM "
				   "\"http://www.adobe.com/xml/dtds/cross-domain-policy.dtd\">\n"
				   "<cross-domain-policy>\n"
				   "<allow-access-from domain=\"*\" to-ports=\""+ports+"\"/>\n"
				   "<allow-http-request-headers-from domain=\"*\" headers=\"*\"/>\n"
				   "</cross-domain-policy>";
		}

		m_crossdomainStrGZ = m_crossdomainStr = body;

		if (!compressData(m_crossdomainStrGZ))
		{
			m_crossdomainStrGZ.clear();
		}
	}

	// default to returning the non-compressed version
	return (compressed ? (!m_crossdomainStrGZ.empty() ? m_crossdomainStrGZ : m_crossdomainStr) : m_crossdomainStr);
}

uniString::utf8 config::getShoutcastSWF(const bool compressed) throw()
{
	if (m_shoutcastSWFStr.empty())
	{
		utf8 body = loadLocalFile(gStartupDirectory + "shoutcast.swf");
		if (body.empty())
		{
			body = MSG_HTTP404;
		}

		m_shoutcastSWFStrGZ = m_shoutcastSWFStr = body;

		if (!compressData(m_shoutcastSWFStrGZ))
		{
			m_shoutcastSWFStrGZ.clear();
		}
	}

	// default to returning the non-compressed version
	return (compressed ? (!m_shoutcastSWFStrGZ.empty() ? m_shoutcastSWFStrGZ : m_shoutcastSWFStr) : m_shoutcastSWFStr);
}

uniString::utf8 config::getIndexCSS(const bool compressed) throw()
{
	if (m_styleCustomStr.empty())
	{
		utf8 body = loadLocalFile(fileUtil::getFullFilePath(adminCSSFile()));
		if (!body.empty())
		{
			m_styleCustomStrGZ = m_styleCustomStr = body;

			if (!compressData(m_styleCustomStrGZ))
			{
				m_styleCustomStrGZ.clear();
			}
		}
	}

	// default to returning the non-compressed version
	return (compressed ? (!m_styleCustomStrGZ.empty() ? m_styleCustomStrGZ : m_styleCustomStr) : m_styleCustomStr);
}

const uniString::utf8 config::getStreamHideStats(const size_t streamID) const
{
	if (!streamID)
	{
		return hideStats();
	}
	else
	{
		uniString::utf8 hide = stream_hideStats(streamID);
		if (!read_stream_hideStats(streamID) || hide.empty())
		{
			hide = hideStats();
		}
		return hide;
	}
}

const uniString::utf8 config::getStreamRedirectURL(const size_t streamID, const bool isStats,
												   const bool homeSet, const bool compress,
												   const bool force) const throw()
{
	utf8 url;
	// check if hiding of public pages is enabled with some
	// specific handling as required for 'all' or 'stats'.
	bool all = (getStreamHideStats(streamID) == "all"),
		 //none = !(getStreamHideStats(streamID) == "none"),
		 stats = ((getStreamHideStats(streamID) == "stats") && isStats);

	if (all || stats || force)
	{
		// if no streamid then look at global redirect option only
		if (!streamID)
		{
			// if not set then we look at the streamurl from source
			// and if that is not valid then and return a 403 error
			if (!redirectUrl().empty())
			{
				url = redirect(redirectUrl(), compress);
			}
			else
			{
				// but we check if it's for a stats only hide on a
				// stats method and then redirect to /index.html
				if (stats && !force)
				{
					url = redirect("index.html", compress);
				}
				else
				{
					url = MSG_HTTP403;
				}
			}
		}
		else
		{
			// look for a stream specific
			utf8 surl = stream_redirectUrl(streamID);
			if (!read_stream_redirectUrl(streamID) || surl.empty())
			{
				// see if it's a stats page and redirect to /index.html
				if (stats && !force)
				{
					url = redirect("index.html", compress);
				}
				else
				{
					// otherwise attempt to use the streamurl (if available)
					if (homeSet && !force)
					{
						url = redirect("home.html?sid="+tos(streamID), compress);
					}
					else
					{
						// and if not then try the global redirect
						if (!redirectUrl().empty())
						{
							url = redirect(redirectUrl(), compress);
						}
						// before reverting to a 403 error
						else
						{
							url = MSG_HTTP403;
						}
					}
				}
			}
			else
			{
				url = redirect(surl, compress);
			}
		}
	}

	return url;
}

void config::setOption(uniString::utf8 key, uniString::utf8 value) throw(exception)
{
	stackLock sml(m_lock);

	size_t subIndex = DEFAULT_CLIENT_STREAM_ID; // for items of form xxxxxx_#  (multi-options)
	uniString::utf8 stok = key;

	// see if it's a multi option and set the subIndex value
	const vector<utf8> tokens = tokenizer(stok, '_');
	if (tokens.size() == 2)
	{
		stok = tokens[0];
		subIndex = atoi((const char *)stripWhitespace(tokens[1]).c_str());
	}
	stok = stringUtil::stripWhitespace(stok); // cleanup some more whitespace just in case

	// look for the base option name in the option map
	uniString::utf8 base_option_name = stringUtil::toLower(stok);

	std::map<uniString::utf8, accessor_t>::const_iterator mi = m_optMap.find(base_option_name);
	if (mi == m_optMap.end())
	{
		throw std::runtime_error("Unknown option " + base_option_name.hideAsString());
	}

	// first see if the value has changed. If not then we don't worry about it
	if ((this->*(*mi).second.m_fetchFunc)(subIndex, 0) != value)
	{
		if ((base_option_name == "log") || (base_option_name == "screenlog"))
		{
			m_deferredOptions[base_option_name] = value;
		}
		else
		{
			// tweak things as needed with checking the type so we can use the sid or not
			(this->*(*mi).second.m_assignFunc)(value, subIndex);
		}
	}
}
