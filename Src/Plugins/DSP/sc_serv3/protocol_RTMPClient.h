#pragma once
#if 0
#ifndef protocol_RTMPClient_H_
#define protocol_RTMPClient_H_

#include "threadedRunner.h"
#include "streamData.h"
#include <map>
#include <vector>

class streamData;

class AMFEncoding;

class protocol_RTMPClient: public runnable, public clientProtocol
{
public:
	class message
	{
	public:
		int	m_chunkStreamID;
		int m_messageStreamID;
		__uint8 m_messageType;
		std::vector<__uint8> m_messageData;

		void clear() throw() { m_messageData.clear(); }
		uniString::utf8 packetDump() const throw();
	};

private:
	// the limit trigger is used when the client readPtr butts up against the write ptr.
	// In this case we can't select against the socket, because it will always be ready and we'll
	// end up in a CPU intenstive spin loop. Instead, we create a signal and give it to the streamData
	// object with the value of our readPtr. The streamData object will signal the trigger when there
	// is data available
	pipeDrivenSignal<AOL_namespace::mutex>		m_limitTrigger;

	socketOps::tSOCKET		m_socket;
	const uniString::utf8	m_clientHostName;
	const uniString::utf8	m_clientAddr;
	const int				m_clientPort;
	uniString::utf8			m_clientLogString;
	const __uint8			m_C0;	// C0 packet sent from client. Has version number
	const __uint8			m_S0;	// my response
	std::vector<__uint8>	m_C1;	// C1 packet from client
	std::vector<__uint8>	m_C2;	// C2 packet from client
	std::vector<__uint8>	m_S1orS2;	// S1 or S2 packet from me

	__uint8			*m_inDataBuffer;
	size_t			m_inDataBufferMax; // max allocated
	size_t			m_inDataBufferAmt; // amt in buffer
	size_t			m_inDataRequested; // amt we want to read for a fixed read (C1 or C2)
	message			m_inMsg;

	const __uint8	*m_outDataPtr; // for outgoing text lines
	size_t			m_outDataSize;
	std::vector<__uint8>	m_outDataBuffer;

	__uint32		m_serverBaseTime;	// for RTMP protocol
	__uint32		m_C1ReadTime;

	time_t					m_lastActivityTime;

	time_t					m_startTime;	// when connection started, used to implement listenerTime

	// inbound state
	size_t							m_inboundChunkSize;
	int								m_lastInboundMessageStreamID;
	int								m_lastInboundMessageLength;
	int								m_lastInboundMessageTypeID;
	long							m_lastInboundTimestamp;
	size_t							m_windowSizeFromClient;
	int								m_bufferSizeFromClient;

	// outbound state
	size_t							m_outboundChunkSize;

	long getMsg(message &msg) throw(std::exception);

	//// stats for w3c logs
	time_t					m_lastTitleTime;	// time of last title update used for w3c logs
	__uint64				m_bytesSentForCurrentTitle; // bytes sent since last title change. used for w3c logs
	__uint64				m_totalBytesSent;	// total bytes sent whilst the connection is open
	uniString::utf8			m_lastSentMetadata; // no need to send duplicates. Also used for w3C tracking

	int			m_objectEncodingMode;
	///////////////////////////

	////////// HTTP message buffer for 302 redirect
//	uniString::utf8			m_redirectResponse;
	//////////////////////////////////////////

//	const protocol_HTTPStyle::HTTPRequestInfo	m_HTTPRequestInfo; // the received request and headers
//	uniString::utf8			m_ICYOKResponse;
//	std::vector<__uint8>	m_ICYMetadata; // metadata buffer

	streamData::streamID_t	m_streamID;		// stream ID for this connection

	streamData::ringBufferAccess_t	m_readPtr;	// pointer into ring buffer
	int								m_underruns; // client too slow
	bool							m_removeClientFromStats; // do we have to do a stats::removeClient

	bool							m_sendMetadata;
	size_t							m_metaInterval;
	size_t							m_metaIntervalCounter; // counter for metadata updates

	std::vector<__uint8>			m_introFile;
	size_t							m_introFileOffset;
	std::vector<__uint8>			m_backupFile;
	size_t							m_backupFileOffset;

	void aquireIntroFile() throw();
	void aquireBackupFile() throw();

	streamData::ringBufferAccess_t resetReadPtr() throw(); // return distance between read and write pointer
	void sendICYMetadata(const uniString::utf8 &md) throw();

	void logW3C() throw();
	void setW3CState() throw();

	typedef runnable::timeSliceResult (protocol_RTMPClient::*state_t)(); // throw(std::exception);

	state_t	m_state;
	state_t m_nextState;

	streamData	*m_streamData;

	bool chunkSequenceComplete(const __uint8 *inBuffer,size_t amtInBuffer,size_t &amtToRemoveFromInBuffer,size_t expectedChunkSize,
								std::vector<__uint8> &msg,__uint8 &msgType,int &chunkStreamID,int &messageStreamID,const uniString::utf8 &logMsgPrefix) throw(std::exception);

	runnable::timeSliceResult state_Send() throw(std::exception);
	runnable::timeSliceResult state_RecvFixedAmt() throw(std::exception);
	runnable::timeSliceResult state_RecvMsg() throw(std::exception);

	runnable::timeSliceResult state_WaitForC1() throw(std::exception);
	runnable::timeSliceResult state_GotC1() throw(std::exception);
	runnable::timeSliceResult state_WaitForC2() throw(std::exception);
	runnable::timeSliceResult state_GotC2() throw(std::exception);
	runnable::timeSliceResult state_SendS0() throw(std::exception);
	runnable::timeSliceResult state_SendS1() throw(std::exception);
	runnable::timeSliceResult state_SendS2() throw(std::exception);

	runnable::timeSliceResult state_WaitForMessage() throw(std::exception);
	runnable::timeSliceResult state_GotMessage() throw(std::exception);

	runnable::timeSliceResult state_SendAudio() throw(std::exception);

	runnable::timeSliceResult handle_AMF0_message() throw(std::exception);
	runnable::timeSliceResult handle_AMF0_connect(const AMFEncoding &amf) throw(std::exception);
	runnable::timeSliceResult handle_AMF0_createStream(const AMFEncoding &amf) throw(std::exception);
	runnable::timeSliceResult handle_AMF0_play(const AMFEncoding &amf) throw(std::exception);

	runnable::timeSliceResult handle_USER_CONTROL_message() throw(std::exception);
	runnable::timeSliceResult handle_UCM_StreamBegin() throw(std::exception);

	runnable::timeSliceResult state_SendConnectResponse() throw(std::exception);

	runnable::timeSliceResult state_Close() throw(std::exception);

	virtual runnable::timeSliceResult timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_RTMPClient"; }

public:
	protocol_RTMPClient(socketOps::tSOCKET s,const uniString::utf8 &hostName,const uniString::utf8 &addr,int port,
						__uint8 C0 /* initial client packet */) throw(std::exception);
	virtual ~protocol_RTMPClient() throw();
};

#endif
#endif
