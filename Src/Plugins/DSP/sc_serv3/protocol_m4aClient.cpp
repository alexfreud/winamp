#if 0
#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_m4aClient.h"
#include "ripList.h"
#include "stats.h"
#include "streamData.h"
#include "w3cLog.h"
#include "global.h"
#include "bandwidth.h"
#include "MP3Header.h"
#include "ADTSHeader.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define DEBUG_LOG(x) { if (gOptions.m4aClientDebug()) DLOG((x)); }
#define AD_DEBUG_LOG(x) { if (gOptions.adMetricsDebug()) DLOG((x)); }

protocol_m4aClient::protocol_m4aClient(const socketOps::tSOCKET s, const streamData::streamID_t streamID,
									   const utf8 &hostName, const utf8 &addr, const u_short port,
									   const utf8 &userAgent, const utf8 &XFF, const utf8 &referer,
									   const bool headRequest) throw(exception)
	: protocol_shoutcastClient(s, port, streamData::M4A, hostName,
							   (streamID <= 0 || streamID > INT_MAX ? DEFAULT_CLIENT_STREAM_ID : streamID),
							   stats::getNewClientId(), userAgent, referer, addr, XFF, headRequest),

	  m_state(&protocol_m4aClient::state_AttachToStream), m_nextState(0)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);
}

protocol_m4aClient::~protocol_m4aClient() throw()
{
	cleanup("M4A", gOptions.m4aClientDebug());
}

//////////////////////////////////////////////////////////////////////////////

void protocol_m4aClient::timeSlice() throw(exception)
{
	int ret = doTimeSlice(result);
	if (ret == 1)
	{
		m_state = &protocol_m4aClient::state_Stream;
		return;
	}
	else if (ret == 2)
	{
		return;
	}

	(this->*m_state)();
}

void protocol_m4aClient::state_Close() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	m_result.done();
}

void protocol_m4aClient::state_SendText() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif

	if (sendText())
	{
		m_metaIntervalCounter = 0;
		m_state = m_nextState;
	}
}

// find the appropriate stream and try to attach to it
void protocol_m4aClient::state_AttachToStream() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	m_streamData = streamData::accessStream(m_streamID);
	if (!m_streamData)
	{
		if (processReject("M4A", bandWidth::CLIENT_M4A_SENT, MSG_ICY_HTTP401,
						  MSG_ICY_HTTP401_LEN, &read_bitrate, &dataType))
		{
			goto fall_through;
		}

		m_state = &protocol_m4aClient::state_SendText;
		m_nextState = &protocol_m4aClient::state_Close;
	}
	else
	{
fall_through:
		const utf8 movedUrl = gOptions.stream_movedUrl(m_streamID);
		if (movedUrl.empty())
		{
			const int add = processAdd("FLV", bandWidth::CLIENT_M4A_SENT,
									   MSG_ICY_HTTP401, MSG_ICY_HTTP401_LEN, movedUrl,
									   (m_streamData ? m_streamData->streamBackupServer() : ""));
			if (add != 1)
			{
				m_state = &protocol_m4aClient::state_SendText;
				m_nextState = &protocol_m4aClient::state_Close;
			}
			else
			{
				const bool isPodcast = (!m_streamData && (gOptions.getBackupLoop(m_streamID) == 1));
				m_OKResponse = MSG_ICY_HTTP200 + "Content-Type:audio/mp4\r\n";

				if (!m_streamData)
				{
					utf8 path = getStreamPath(m_streamID);
					if (!path.empty() && path.find(utf8("/")) == 0)
					{
						path = path.substr(1);
					}

					if (isPodcast)
					{
						m_OKResponse += "Content-Disposition:attachment;filename=\"" + path + "\"\r\n"
										"Content-Length:" + tos(m_backupFile.size()) + "\r\n";
					}
				}

				if (gOptions.clacks())
				{
					m_OKResponse += "X-Clacks-Overhead:GNU Terry Pratchett\r\n";
				}
				m_OKResponse += "\r\n";

				DEBUG_LOG(m_clientLogString + "Sending [" + eol() + stripWhitespace(m_OKResponse) + eol() + "]");
				m_outBuffer = m_OKResponse.c_str();
				bandWidth::updateAmount(bandWidth::CLIENT_M4A_SENT, (m_outBufferSize = (int)m_OKResponse.size()));
				m_state = &protocol_m4aClient::state_SendText;
				if (!m_headRequest)
				{
					m_nextState = &protocol_m4aClient::state_InitiateStream;
				}
				else
				{
					m_removeClientFromStats = false;
					m_ignoreDisconnect = true;
					m_nextState = &protocol_m4aClient::state_Close;
				}
				m_result.write();
				m_result.timeoutSID(m_streamID);
				m_result.run();

				// when the client is added, we get back the unique id of the connection
				// but we now check for being > 0 as we need to filter out some of the
				// YP connections from being counted as valid clients for stats, etc
				reportNewListener("M4A");
			}
		}
		else
		{
			// if we get to here then we attempt to redirect the clients to the moved url
			// which is useful if the stream has moved hosting or it has been deprecated.
			streamMovedOrRejected("M4A", bandWidth::CLIENT_M4A_SENT, movedUrl, 2);
			m_state = &protocol_m4aClient::state_SendText;
			m_nextState = &protocol_m4aClient::state_Close;
		}
	}
}

void protocol_m4aClient::state_SendIntro() throw(exception)
{
	state_SendIntroFile();
	if (m_introFile.empty())
	{
		acquireIntroFile();
		if (m_introFile.empty())
		{
			m_state = &protocol_m4aClient::state_Stream;
		}
		else
		{
			m_state = &protocol_m4aClient::state_SendIntroFile;
		}
	}
}

void protocol_m4aClient::state_InitiateStream() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	resetReadPtr();

	if (!m_streamData || (m_streamData && m_introFile.empty()))
	{
		// send intro file if we have it
		acquireIntroFile();
		m_state = (m_introFile.empty() ? &protocol_m4aClient::state_Stream : &protocol_m4aClient::state_SendIntroFile);
	}
	else
	{
		m_state = &protocol_m4aClient::state_SendIntro;
	}

	setW3CState();

	m_result.run();
}

// handle state where we are sending intro files
void protocol_m4aClient::state_SendIntroFile() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	resetCommon();

	time_t cur_time;
	const bool debug = gOptions.m4aClientDebug();
	const int autoDumpTime = detectAutoDumpTimeout(cur_time, m_lastActivityTime, (m_clientLogString +
												   "Timeout waiting to send data"),
												   (!m_ignoreDisconnect && debug), m_streamID);

	if (m_shortSend.empty() && (m_frameCount > calculateFrameLimit(cur_time)))
	{
		if (calculateDelay((autoDumpTime - (int)(cur_time - m_lastActivityTime))))
		{
			// if we're at the limit then we're going to need to sit and spin
			// which will need to be a bit short of the required frame rate
			// so that we've got a bit of leeway on scheduling delays, etc
			return;
		}
	}

	int amt = (int)(m_introFile.size() - m_introFileOffset);
	if (amt == 0)
	{
		// we're done with the intro file
		m_introFile.clear();
		m_introFile.resize(0);
		m_introFileOffset = 0;
		m_lastActivityTime = ::time(NULL);
		m_state = &protocol_m4aClient::state_Stream;
		resetReadPtr();
		m_result.run();
	}
	else if (amt > 0)
	{
		const bool debug = gOptions.m4aClientDebug();
		checkListenerIsValid(debug);

		if (!m_lastSentMetadata.empty())
		{
			logW3C();
			m_lastSentMetadata.clear();
		}

		amt = min(amt, SEND_SIZE);

		int len = (int)amt,
			frames = 0;	// this will be uvox frames as we pre-process earlier to ensure
						// that the audio frames are frame-synced when this is created
		bool advert = false;
		doM4AFrameSync(m_streamData->streamUvoxDataType(), debug, m_introFileOffset,
					   (const char*)&m_introFile[0], cur_time, m_streamData->streamBitrate(),
					   m_streamData->streamSampleRate(), len, frames, advert);

		int rval = doSend(debug, cur_time, autoDumpTime);
		if (rval > 0)
		{
			bandWidth::updateAmount(bandWidth::CLIENT_M4A_SENT, rval);
			m_bytesSentForCurrentTitle += rval;
			m_totalBytesSent += rval;
			m_lastActivityTime = ::time(NULL);
			m_metaIntervalCounter += rval;
			// we adjust by the pre-M4A size
			m_introFileOffset += len;
		}

		updateFrameCount(frames, cur_time);

		handleShortSend(autoDumpTime, cur_time, rval);
	}
}

// handle state where we are sending backup files
void protocol_m4aClient::state_SendBackupFile() throw(exception)
{
	DEBUG_LOG(m_clientLogString + __FUNCTION__);

	resetCommon();

	int amt = (int)(m_backupFile.size() - m_backupFileOffset);

	if (streamData::isSourceConnected(m_streamID))
	{
		if (m_streamData)
		{
			m_streamData->releaseStream();
		}
		m_streamData = streamData::accessStream(m_streamID);

		// we're done with the backup file
		m_backupFile.clear();
		m_backupFile.resize(0);
		m_backupFileOffset = 0;
		m_backupLoopTries = 0;
		m_lastActivityTime = ::time(NULL);
		m_state = &protocol_m4aClient::state_Stream;
		resetReadPtr();
		m_result.run();
	}
	else if (amt == 0)
	{
		const int backuploop = gOptions.getBackupLoop(m_streamID);
		if (!backuploop || (m_backupLoopTries <= backuploop))
		{
			// we're done with the backup file. get more data
			acquireBackupFile();
			if (!m_backupFile.empty())
			{
				++m_backupLoopTries;
				resetReadPtr();
				m_lastActivityTime = ::time(NULL);
				m_state = &protocol_m4aClient::state_Stream;
			}
		}
		else if (backuploop && (m_backupLoopTries < backuploop))
		{
			m_backupFileOffset = 0;
			m_backupFile.clear();
		}
		else
		{
			resetReadPtr();
			m_backupFileOffset = 0;
			m_lastActivityTime = ::time(NULL);
			m_state = &protocol_m4aClient::state_Stream;
		}

		m_result.run();
	}
	else if (amt > 0)
	{
		const bool debug = gOptions.m4aClientDebug();
		checkListenerIsValid(debug);

		time_t cur_time;
		const int autoDumpTime = detectAutoDumpTimeout(cur_time, m_lastActivityTime, (m_clientLogString +
													   "Timeout waiting to send data"),
													   (!m_ignoreDisconnect && debug), m_streamID);

		if (m_shortSend.empty() && (m_frameCount > calculateFrameLimit(cur_time)))
		{
			if (calculateDelay((autoDumpTime - (int)(cur_time - m_lastActivityTime))))
			{
				// if we're at the limit then we're going to need to sit and spin
				// which will need to be a bit short of the required frame rate
				// so that we've got a bit of leeway on scheduling delays, etc
				return;
			}
		}

		if (!m_lastSentMetadata.empty())
		{
			logW3C();
			m_lastSentMetadata.clear();
		}

		amt = min(amt, SEND_SIZE);

		int len = (int)amt, backup_type = MP3_DATA,
			frames = 0;	// this will be uvox frames as we pre-process earlier to ensure
						// that the audio frames are frame-synced when this is created
		if (!m_streamData)
		{
			utf8 backupFile = gOptions.stream_backupFile(m_streamID);
			if (!gOptions.read_stream_backupFile(m_streamID))
			{
				backupFile = gOptions.backupFile();
			}
			if (!backupFile.empty())
			{
				backup_type = ((backupFile.rfind((utf8)".aac") == utf8::npos) ? MP3_DATA : AACP_DATA);
			}
		}

		bool advert = false;
		doM4AFrameSync((m_streamData ? m_streamData->streamUvoxDataType() : backup_type),
					   debug, m_backupFileOffset, (const char*)&m_backupFile[0], cur_time,
					   (m_streamData ? m_streamData->streamBitrate() : 0),
					   (m_streamData ? m_streamData->streamSampleRate() : 0), len, frames, advert);

		int rval = doSend(debug, cur_time, autoDumpTime);
		if (rval > 0)
		{
			bandWidth::updateAmount(bandWidth::CLIENT_M4A_SENT, rval);
			m_bytesSentForCurrentTitle += rval;
			m_totalBytesSent += rval;
			m_lastActivityTime = ::time(NULL);
			m_metaIntervalCounter += rval;
			// we adjust by the pre-M4A size
			m_backupFileOffset += len;
		}

		updateFrameCount(frames, cur_time);

		handleShortSend(autoDumpTime, cur_time, rval);
	}
}

// handle state where we are sending advert content
void protocol_m4aClient::state_SendAdverts() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif

	resetCommon();

	streamData::specialFileData *ad = m_adAccess.getAd(m_streamID, m_streamData->advertGroups, !!m_streamData->m_adTest);
	int amt = (ad ? (int)(ad->m_sc1Buffer.size() - m_adAccess.offset) : 0);

	while (true)
	{
		// DLOG ("amount remaining to be sent " + tos(amt));
		if (amt > 0)
		{
			break;
		}
		if (m_adAccess.anotherAd(m_streamID, m_streamData->advertGroups, !!m_streamData->m_adTest))
		{
			ad = m_adAccess.getAd(m_streamID, m_streamData->advertGroups, !!m_streamData->m_adTest);
			amt = (ad ? (int)(ad->m_sc1Buffer.size() - m_adAccess.offset) : 0);
			continue;
		}
		// no more adverts
		m_lastActivityTime = ::time(NULL);
		m_state = &protocol_m4aClient::state_Stream;
		m_adAccess.stopAdRun(m_streamID, m_streamData->advertGroups, !!m_streamData->m_adTest);
		// go to the latest point in the ring buffer.
		resetReadPtr();
		m_result.run();
		m_result.timeoutSID(m_streamID);

		ILOG(m_clientLogString + "Transitioning back to stream [Agent: `" +
			 m_userAgent + "', UID: " + tos(m_unique) + ", GRID: " + tos(m_group) + "]");
		return;
	}

	try
	{
		const bool debug = gOptions.m4aClientDebug();
		checkListenerIsValid(debug);

		time_t cur_time;
		const int autoDumpTime = detectAutoDumpTimeout(cur_time, m_lastActivityTime, (m_clientLogString +
													   "Timeout waiting to send data"),
													   (!m_ignoreDisconnect && debug), m_streamID);

		if (m_shortSend.empty() && (m_frameCount > calculateFrameLimit(cur_time)))
		{
			if (calculateDelay((autoDumpTime - (int)(cur_time - m_lastActivityTime))))
			{
				// if we're at the limit then we're going to need to sit and spin
				// which will need to be a bit short of the required frame rate
				// so that we've got a bit of leeway on scheduling delays, etc
				return;
			}
		}

		if (!m_lastSentMetadata.empty())
		{
			logW3C();
			m_lastSentMetadata.clear();
		}

		amt = min(amt, SEND_SIZE);

		int len = (int)amt,
			frames = 0;	// this will be uvox frames as we pre-process earlier to ensure
						// that the audio frames are frame-synced when this is created
		bool advert = false;
		doM4AFrameSync(m_streamData->streamUvoxDataType(), (int)m_adAccess.offset,
					   debug, (const char*)&ad->m_sc1Buffer[0], cur_time,
					   m_streamData->streamBitrate(), m_streamData->streamSampleRate(),
					   len, frames, advert);

		int rval = doSend(debug, cur_time, autoDumpTime);
		if (rval > 0)
		{
			bandWidth::updateAmount(bandWidth::CLIENT_M4A_SENT, rval);
			m_bytesSentForCurrentTitle += rval;
			m_totalBytesSent += rval;
			m_lastActivityTime = ::time(NULL);
			m_metaIntervalCounter += rval;
			// we adjust by the pre-M4A size
			m_adAccess.offset += len;

			m_nextState = &protocol_m4aClient::state_SendText;
		}

		updateFrameCount(frames, cur_time);

		handleShortSend(autoDumpTime, cur_time, rval);
	}
	catch (std::runtime_error)
	{
		m_adAccess.stopAdRun(m_streamID, m_streamData->advertGroups, !!m_streamData->m_adTest);
		throw;
	}
}

void protocol_m4aClient::state_Stream() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_clientLogString + __FUNCTION__);
#endif

	resetCommon();

	const time_t cur_time = ::time(NULL);
	const bool debug = gOptions.m4aClientDebug();
	const int autoDumpTime = gOptions.getAutoDumpTime(m_streamID); // don't want this value to change during this call
	if ((autoDumpTime > 0) && ((cur_time - m_lastActivityTime) >= autoDumpTime))
	{
		throwEx<runtime_error>((!m_ignoreDisconnect && debug ?
								(m_clientLogString + "Timeout waiting to send data (" +
								 tos(cur_time) + " " + tos(m_lastActivityTime) + " [" +
								 tos(cur_time - m_lastActivityTime) + "] )") : (utf8)""));
	}

	if (m_shortSend.empty() && (m_frameCount > calculateFrameLimit(cur_time)))
	{
		if (calculateDelay((autoDumpTime - (int)(cur_time - m_lastActivityTime))))
		{
			// if we're at the limit then we're going to need to sit and spin
			// which will need to be a bit short of the required frame rate
			// so that we've got a bit of leeway on scheduling delays, etc
			return;
		}
	}

	const streamData::ringBuffer_t rb = (m_streamData ? m_streamData->getSc1RingBuffer() : streamData::ringBuffer_t());
	streamData::ringBufferAccess_t amt = (m_streamData ? (rb.m_writePtr - m_readPtr) : 0);
	if ((amt > 0) && (amt > rb.m_data.size()))
	{
		// the pointers are too far apart. Underrun
		resetReadPtr(&amt);
	}

	std::vector<__uint8>& rem = getRemainder();
	const streamData::ringBufferAccess_t offset = (m_readPtr & rb.m_ptrMask);
	// clamp again so we don't read pass the end of the buffer
	//
	// if we've got more in remainder than what we're wanting
	// to send then we'll prioritise the remainder data first
	// before trying to acquire more new data to try to send.
	amt = min(amt, min((rb.m_data.size() - offset), (streamData::ringBufferAccess_t)max(0, (SEND_SIZE - rem.size()))));

	bool advert = false;
	int remainder = 0, len = (int)amt,
		frames = 0;	// this will be uvox frames as we pre-process earlier to ensure
					// that the audio frames are frame-synced when this is created
	if ((len > 0) || !rem.empty() || !m_shortSend.empty())
	{
		const std::vector<__uint8>::const_iterator pos = rb.m_data.begin();
		std::vector<__uint8>& tempBuf;
		// if we had anything left over then now we
		// need to copy it back into the buffer and
		// adjust the max data amount to be read in
		if (!rem.empty() && ((len + rem.size()) <= (BUF_SIZE * 4)))
		{
			tempBuf.insert(tempBuf.end(), rem.begin(), rem.end());
			tempBuf.insert(tempBuf.end(), pos + offset, pos + (offset + len));
			len += m_remainderSize;
		}
		else if (len > 0)
		{
			tempBuf.insert(tempBuf.end(), pos + offset, pos + (offset + len));
		}
		rem.clear();

		remainder = doM4AFrameSync((m_streamData ? m_streamData->streamUvoxDataType() : MP3_DATA),
								   debug, 0, cur_time, (m_streamData ? m_streamData->streamBitrate() : 0),
								   (m_streamData ? m_streamData->streamSampleRate() : 0),
								   len, frames, advert, true);
	}

	// if no data then we need to just go to adverts
	// otherwise we need to see what we've got and
	// if we can, then transition to adverts or spin
	// and wait for the current data to be sent and
	// then we should be able to re-process into ads
	if (m_output.empty())
	{
		// this will close the stream or go to backups as needed
		if (handleNoData("M4A", remainder, amt, autoDumpTime, cur_time))
		{
			m_state = &protocol_m4aClient::state_SendBackupFile;
		}
		else
		{
			if (processAdvertTrigger(advert))
			{
				m_state = &protocol_m4aClient::state_SendAdverts;
				return;
			}

			// at this point, we're in a bit of a weird state
			// as we've not been able to generate / retrieve
			// anything and if we don't slwo down things then
			// we need to delay things to allow it to catchup
			if (m_frameCount > calculateFrameLimit(cur_time))
			{
				// determine things based on the 'next' second when working out
				// the differential as that's how long we want to wait to run
				m_result.schedule((int)((__uint64)((cur_time + 1) * 1000) - m_result.m_currentTime));
			}
			else
			{
				// just go with an arbitrary value as we cannot
				// be certain when we're going to get data next
				// and we don't know the impact on playback etc
				m_result.schedule(100);
			}

			m_result.timeout((autoDumpTime - (int)(cur_time - m_lastActivityTime)));
			m_result.write();
		}
	}
	else
	{
		processTitleW3C();

		int rval = doSend(debug, cur_time, autoDumpTime);
		if (rval > 0)
		{
			bandWidth::updateAmount(bandWidth::CLIENT_M4A_SENT, rval);
			m_bytesSentForCurrentTitle += rval;
			m_totalBytesSent += rval;
			m_lastActivityTime = ::time(NULL);
			m_metaIntervalCounter += rval;

			if (processAdvertTrigger(advert))
			{
				m_state = &protocol_m4aClient::state_SendAdverts;
				remainder = 0;
			}
		}

		// as we're keeping a copy of the remainder / non-sent data
		// we need to move the readptr on so that it doesn't cause
		// the stream to effectively stick on the same point and in
		// turn then lead to resetReadPtr() being called (skipping)
		m_readPtr += amt;
		updateFrameCount(frames, cur_time);

		handleShortSend(autoDumpTime, cur_time, rval);

		// we only keep the remainder if
		// there's no advert to provide
		m_remainderSize = remainder;
	}
}

void protocol_m4aClient::return_403(void)
{
	protocol_shoutcastClient::return_403();
	m_state = &protocol_m4aClient::state_SendText;
	m_nextState = &protocol_m4aClient::state_Close;
}
#endif
