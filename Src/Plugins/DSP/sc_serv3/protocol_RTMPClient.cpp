#if 0
#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_RTMPClient.h"
#include "ripList.h"
#include "stats.h"
#include "streamData.h"
#include "amf.h"
#include "w3cLog.h"
#include "log.h"
#include "global.h"
#include <iomanip>

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define C1_S1_C2_S2_PACKET_SIZE 1536
#define DEFAULT_CHUNK_SIZE 128

#define RTMP_MSG__SET_CHUNK_SIZE				1
#define RTMP_MSG__ABORT							2
#define RTMP_MSG__ACKNOWLEDGEMENT				3
#define RTMP_MSG__USER_CONTROL_MESSAGE			4
#define RTMP_MSG__WINDOW_ACKNOWLEDGEMENT_SIZE	5
#define RTMP_MSG__SET_PEER_BANDWIDTH			6
#define RTMP_MSG__AUDIO_DATA					8
#define RTMP_MSG__VIDEO_DATA					9
#define RTMP_MSG__DATA_AMF3						15
#define RTMP_MSG__COMMAND_AMF3					17
#define RTMP_MSG__DATA_AMF0						18
#define RTMP_MSG__COMMAND_AMF0					20

// user control messages
#define RTMP_UCM_STREAM_BEGIN	0
#define RTMP_UCM_STREAM_EOF		1
#define RTMP_UCM_STREAM_DRY		2
#define RTMP_UCM_SET_BUFFER		3
#define RTMP_UCM_STREAM_IS_RECORDED	4
#define RTMP_UCM_PING_REQUEST	6
#define RTMP_UCM_PING_RESPONSE	7
/////////////////////////

#define RTMP_PEER_BANDWIDTH_HARD	0
#define RTMP_PEER_BANDWIDTH_SOFT	1
#define RTMP_PEER_BANDWIDTH_DYNAMIC	2

#define DEFAULT_SERVER_WINDOW 0x2625a0 //(16 * 1024)

#define DEBUG_LOG(x) { if (gOptions.RTMPClientDebug()) DLOG((x)); }

#ifdef _WIN32
#define TIMEFUNC ::timeGetTime
#else
#include <sys/time.h>
static unsigned long TIMEFUNC() throw()
{
	struct timeval tp;
	::gettimeofday(&tp,NULL);
	return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
#endif

static void createRTMPMsg(__uint8 msgType,long timestamp,int outboundChunkSize,const __uint8 *payload,size_t payloadSize,vector<__uint8> &result,
							int chunkStreamID = 2,int msgStreamID = 0);
static void createRTMPMsg(const protocol_RTMPClient::message &msg,long timestamp,int outboundChunkSize,vector<__uint8> &result);

//void getRTMPMsgInfo(const vector<__uint8> &msg,__uint8 &msgType,int &payloadLength,long &timestamp,int &streamID,int &payloadOffset);
static size_t decode4ByteValue(const __uint8 *data,size_t offset = 0) throw();

static utf8 prettyPrintMessage(const protocol_RTMPClient::message &msg) throw()
{
	utf8 EOL(eol());

	utf8 result(EOL);
	result += "msg type=" + tos((int)msg.m_messageType) + EOL;

	try
	{
		switch(msg.m_messageType)
		{
			case RTMP_MSG__SET_CHUNK_SIZE:
				result += "chunk size=" + tos(decode4ByteValue(&(msg.m_messageData[0]))) + EOL;
			break;

			case RTMP_MSG__ABORT:
				result += "chunk stream id=" + tos(decode4ByteValue(&(msg.m_messageData[0]))) + EOL;
			break;

			case RTMP_MSG__ACKNOWLEDGEMENT:
				result += "sequence number=" + tos(decode4ByteValue(&(msg.m_messageData[0]))) + EOL;
			break;

			case RTMP_MSG__USER_CONTROL_MESSAGE:
			break;

			case RTMP_MSG__WINDOW_ACKNOWLEDGEMENT_SIZE:
				result += "acknowledgement window size=" + tos(decode4ByteValue(&(msg.m_messageData[0]))) + EOL;
			break;

			case RTMP_MSG__SET_PEER_BANDWIDTH:
				result += "acknowledgement window size=" + tos(decode4ByteValue(&(msg.m_messageData[0]))) + EOL;
				result += "limit type=" + tos(msg.m_messageData[4]) + EOL;
			break;

			case RTMP_MSG__DATA_AMF0:
			case RTMP_MSG__COMMAND_AMF0:
			{
				AMFEncoding amf0;
				amf0.loadFromBitstream((const char *)&(msg.m_messageData[0]),(int)msg.m_messageData.size(),"");
				result += amf0.prettyPrint();
			}
			break;

			case RTMP_MSG__DATA_AMF3:
			case RTMP_MSG__COMMAND_AMF3:
			{
				AMFEncoding amf3(3);
				amf3.loadFromBitstream((const char *)&(msg.m_messageData[0]),(int)msg.m_messageData.size(),"");
				result += amf3.prettyPrint();
			}
			break;
		}
	}
	catch(const exception &ex)
	{
		result += string("Exception: ") + ex.what() + EOL;
		DEBUG_LOG(msg.packetDump());
	}	
	catch(...)
	{
		result += "Exception: <unknown>" + EOL;
		DEBUG_LOG(msg.packetDump());
	}

	return result;
}

#define TEST_FILE "C:\\Documents and Settings\\nradisch\\My Documents\\programming\\shoutcast\\current\\sc_serv2\\test.aac"
FILE *fff = 0;

utf8 protocol_RTMPClient::message::packetDump() const throw()
{
	ostringstream o;

	int x = 0;
	for(std::vector<__uint8>::const_iterator i = m_messageData.begin(); i != m_messageData.end(); ++i)
	{
		if ((x++) % 16 == 0) o << stringUtil::eol();
		o << setw(2) << hex << (int)(*i) << " ";
	}
	return o.str();
}

protocol_RTMPClient::protocol_RTMPClient(socketOps::tSOCKET s,const utf8 &hostName,const utf8 &addr,int port,__uint8 C0)throw(exception)
	:m_socket(s),m_clientHostName(hostName),m_clientAddr(addr),m_clientPort(port),m_clientLogString(dstAddrLogString(hostName,port)),
	m_C0(C0),
	m_S0(3),
	m_inDataBuffer(0),
	m_lastActivityTime(::time(0)),m_startTime(::time(0)),
	m_lastInboundMessageStreamID(-1),
	m_lastInboundMessageLength(-1),
	m_lastInboundMessageTypeID(-1),
	m_lastInboundTimestamp(-1),
	m_windowSizeFromClient(-1),
	m_bufferSizeFromClient(-1),
	m_lastTitleTime(::time(0)),
	m_bytesSentForCurrentTitle(0),
	m_totalBytesSent(0),
	m_objectEncodingMode(0),
	m_removeClientFromStats(false),
	//m_state(&protocol_RTMPClient::state_AttachToStream),
	m_state(&protocol_RTMPClient::state_SendS0),
	m_streamData(0)
{
#ifdef TEST_FILE
	if (fff) ::fclose(fff);
	fff = 0;
	fff = ::fopen(TEST_FILE,"rb");
#endif

	DEBUG_LOG(__FUNCTION__);

	m_inDataBufferMax = 16 * 1024;
	m_inDataBuffer = new __uint8[m_inDataBufferMax];
	m_inDataBufferAmt = 0;

	// intialize s1 to all zeros
	m_S1orS2.resize(C1_S1_C2_S2_PACKET_SIZE,0);
	memset(&(m_S1orS2[0]),0,m_S1orS2.size());

	// set base time
	m_serverBaseTime = TIMEFUNC();
	__uint32 sbt = htonl(m_serverBaseTime);
	memcpy(&(m_S1orS2[0]),&sbt,4);

	// initialize from 10 bytes of random section to random stuff. Faster than
	// doing entire 1528 byte block and still valid
	for(int x = 0; x < 10; ++x)
	{
		m_S1orS2[8+x] = rand();
	}
	m_outDataPtr = &m_S0;
	m_outDataSize = 1;
	m_inboundChunkSize = DEFAULT_CHUNK_SIZE;
	m_outboundChunkSize = DEFAULT_CHUNK_SIZE;
}

protocol_RTMPClient::~protocol_RTMPClient() throw()
{
#ifdef TEST_FILE
	if (fff) ::fclose(fff);
	fff = 0;
#endif
	DEBUG_LOG(__FUNCTION__);

	try
	{
		/*ILOG(m_clientLogString + " SHOUTcast 1 client connection closed (" +
			 tos(::time(0) - m_startTime).c_str() + " seconds). " +
			 mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"user-agent",utf8("")) +
			 " [Bytes: " + tos(m_totalBytesSent).c_str() + "]");*/

		if (m_removeClientFromStats)
			stats::removeClient(m_streamID,this);
		if (m_streamData)
		{
			m_streamData->abandonRTMPLimitTrigger(&m_limitTrigger);
			streamData::streamClientLost(m_streamData);
			m_streamData = 0;

			logW3C();
		}
	}
	catch(const exception &ex) 
	{
		ELOG(ex.what());
	}

	delete [] m_inDataBuffer;
	m_streamData = 0;
	socketOps::forgetTCPSocket(m_socket);
}

runnable::timeSliceResult protocol_RTMPClient::timeSlice() throw(exception)
{
	size_t listenerTime = gOptions.stream_listenerTime(DEFAULT_SHOUTCAST_SOURCE_STREAM);
	if(!gOptions.read_stream_listenerTime(DEFAULT_SHOUTCAST_SOURCE_STREAM))
	{
		listenerTime = gOptions.listenerTime();
	}

	listenerTime *= 60; // convert to seconds
	bool timesUp = (listenerTime && ((::time(0) - m_startTime) > (int)listenerTime));

	if (m_kickNextRound || timesUp || (m_streamData && m_streamData->isDead()))
	{
		if (timesUp)
			{ ILOG(m_clientLogString + " listener time exceeded.");}
		else if (m_kickNextRound)
			{ ILOG(m_clientLogString + " kicked");}
		timeSliceResult result;
		result.m_done = true;
		return result;
	}

	return (this->*m_state)();
}

//////////////////////////////////////////////////////////////////////////////////
///////////////////// Initial Handshake States //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

runnable::timeSliceResult protocol_RTMPClient::state_Send() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	timeSliceResult result;

	long to = sendDataBuffer(m_socket,m_outDataPtr,m_outDataSize,m_lastActivityTime,m_clientLogString);
	if (to == 0)
	{ // done
		m_state = m_nextState;
		result.m_runImmediately = true;
	}
	else
	{ // some more
		result.m_writeSet.insert(m_socket);
		result.m_timeout.tv_sec = to;
	}
	return result;
}

runnable::timeSliceResult protocol_RTMPClient::state_RecvFixedAmt() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	timeSliceResult result;

	assert(m_inDataRequested <= m_inDataBufferMax);

	size_t amt_left = m_inDataRequested - m_inDataBufferAmt;
	size_t amt_left2 = amt_left;

	long to = getSocketData(m_socket,m_inDataBuffer,m_inDataBufferAmt,amt_left,m_lastActivityTime,m_clientLogString);
	m_inDataBufferAmt += (amt_left2 - amt_left);

	if (to == 0)
	{ // got data
		m_state = m_nextState;
		result.m_runImmediately = true;
	}
	else
	{ // wait some more
		result.m_readSet.insert(m_socket);
		result.m_timeout.tv_sec = to;
	}
	return result;
}

runnable::timeSliceResult protocol_RTMPClient::state_RecvMsg() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	timeSliceResult result;

	m_inMsg.clear();

	long to = getMsg(m_inMsg);

	if (to == 0)
	{
		m_state = m_nextState;
		result.m_runImmediately = true;
		DEBUG_LOG(string(__FUNCTION__) + " received msg" + prettyPrintMessage(m_inMsg));
	}
	else
	{
		result.m_readSet.insert(m_socket);
		result.m_timeout.tv_sec = to;
	}

	return result;
}

#define NEXT_STATE timeSliceResult result; result.m_runImmediately = true; return result;

runnable::timeSliceResult protocol_RTMPClient::state_SendS0() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_outDataPtr = &m_S0;
	m_outDataSize = 1;
	m_state = &protocol_RTMPClient::state_Send;
	m_nextState = &protocol_RTMPClient::state_SendS1;
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_SendS1() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_outDataPtr = &m_S1orS2[0];
	m_outDataSize = m_S1orS2.size();
	m_state = &protocol_RTMPClient::state_Send;
	m_nextState = &protocol_RTMPClient::state_WaitForC1;
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_WaitForC1() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_inDataBufferAmt = 0;
	m_inDataRequested = C1_S1_C2_S2_PACKET_SIZE;
	m_state = &protocol_RTMPClient::state_RecvFixedAmt;
	m_nextState = &protocol_RTMPClient::state_GotC1;
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_GotC1() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_C1ReadTime = TIMEFUNC() - m_serverBaseTime;
	m_C1.clear();
	m_C1.insert(m_C1.end(),m_inDataBuffer,m_inDataBuffer + C1_S1_C2_S2_PACKET_SIZE);
	m_state = &protocol_RTMPClient::state_SendS2;
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_SendS2() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_S1orS2 = m_C1;
	__uint32 t = htonl(m_C1ReadTime);
	memcpy(&(m_S1orS2[4]),&t,4);
	m_outDataPtr = &m_S1orS2[0];
	m_outDataSize = m_S1orS2.size();
	m_state = &protocol_RTMPClient::state_Send;
	m_nextState = &protocol_RTMPClient::state_WaitForC2;
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_WaitForC2() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_inDataBufferAmt = 0;
	m_inDataRequested = C1_S1_C2_S2_PACKET_SIZE;
	m_state = &protocol_RTMPClient::state_RecvFixedAmt;
	m_nextState = &protocol_RTMPClient::state_GotC2;
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_GotC2() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_C2.clear();
	m_C2.insert(m_C2.end(),m_inDataBuffer,m_inDataBuffer + C1_S1_C2_S2_PACKET_SIZE);
	m_state = &protocol_RTMPClient::state_WaitForMessage;
	NEXT_STATE	
}

runnable::timeSliceResult protocol_RTMPClient::state_WaitForMessage() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_inDataBufferAmt = 0;
	m_state = &protocol_RTMPClient::state_RecvMsg;
	m_nextState = &protocol_RTMPClient::state_GotMessage;
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_GotMessage() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	switch(m_inMsg.m_messageType)
	{
		case RTMP_MSG__USER_CONTROL_MESSAGE:
		return handle_USER_CONTROL_message();

		case RTMP_MSG__WINDOW_ACKNOWLEDGEMENT_SIZE:
		{
			if (m_inMsg.m_messageData.size() != 4)
				throwEx<runtime_error>(m_clientLogString + " Bad payload size for Window Acknowledgement message, got " + tos(m_inMsg.m_messageData.size()) + " expected 4.");
			m_windowSizeFromClient = decode4ByteValue(&(m_inMsg.m_messageData[0]),0);
			
			DEBUG_LOG(m_clientLogString + " WAS from client is " + tos(m_windowSizeFromClient));
			m_state = &protocol_RTMPClient::state_WaitForMessage;
			NEXT_STATE
		}
		break;

		case RTMP_MSG__COMMAND_AMF0:
		return handle_AMF0_message();

		default:
			throwEx<runtime_error>(m_clientLogString + " " + __FUNCTION__ + " cannot dispatch message type " + tos((int)m_inMsg.m_messageType));
	}	
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::handle_USER_CONTROL_message() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	if (m_inMsg.m_messageData.size() < 2)
		throwEx<runtime_error>(m_clientLogString + " User control message has insufficient data.");

	__uint16 t = ntohs(*(const __uint16 *)(&m_inMsg.m_messageData[0]));
	switch(t)
	{
		case RTMP_UCM_SET_BUFFER:
		{
			if (m_inMsg.m_messageData.size() < 10)
				throwEx<runtime_error>(m_clientLogString + " Set Buffer user control message has insufficient data.");
			m_bufferSizeFromClient = ntohl(*(const __uint32*)&(m_inMsg.m_messageData[6]));
			DEBUG_LOG(m_clientLogString + " Buffer size from client is " + tos(m_bufferSizeFromClient) + " milliseconds.");
			m_state = &protocol_RTMPClient::state_WaitForMessage;
			NEXT_STATE
		}
		break;

		case RTMP_UCM_STREAM_BEGIN:
			//return handle_UCM_StreamBegin();
		break;
	}

	throwEx<runtime_error>(m_clientLogString + " User control message type " + tos(t) + " is not supported.");
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::handle_UCM_StreamBegin() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::handle_AMF0_message() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	AMFEncoding amf;
	amf.loadFromBitstream((const char *)&(m_inMsg.m_messageData[0]),(int)m_inMsg.m_messageData.size(),m_clientLogString);
	const AMFVal &v0 = amf.getValue(0);
	if (v0.getString() == "connect")
		return handle_AMF0_connect(amf);
	if (v0.getString() == "createStream")
		return handle_AMF0_createStream(amf);
	if (v0.getString() == "play")
		return handle_AMF0_play(amf);

	throwEx<runtime_error>(m_clientLogString + " " + __FUNCTION__ + " Unknown AMF0 message " + v0.getString());
	NEXT_STATE			
}

runnable::timeSliceResult protocol_RTMPClient::handle_AMF0_connect(const AMFEncoding &amf) throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

//	const AMFVal &v0 = amf.getValue(0);
	const AMFVal &v1 = amf.getValue(1);
	const AMFVal &v2 = amf.getValue(2);

	if (v1.getNumber() != 1)
		throwEx<runtime_error>(m_clientLogString + " Unexpected transaction number. Wanted 1, got " + tos(v1.getNumber()));
	const AMFObject &o = v2.getObject();

	const AMFVal *pv = o.getProperty("tcUrl");
	if (!pv) throwEx<runtime_error>(m_clientLogString + " Connect command has no tcUrl value.");
	utf8 url = pv->getString(); // use this value to create stream accessor

	pv = o.getProperty("objectEncoding");
	m_objectEncodingMode = (pv ? (int)pv->getNumber() : 0);

	m_state = &protocol_RTMPClient::state_SendConnectResponse;
	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::handle_AMF0_createStream(const AMFEncoding &amf) throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

//	const AMFVal &v0 = amf.getValue(0);
	const AMFVal &v1 = amf.getValue(1);

	double transaction = v1.getNumber();

	AMFEncoding amf0;
	AMFObject obj;
	amf0.appendValue(AMFVal(utf8("_result"))); // or "_error"
	amf0.appendValue(AMFVal((double)transaction));
	amf0.appendValue(AMFVal());
	amf0.appendValue(AMFVal((double)1)); //1234)); /// stream ID for client

	vector<__uint8> resp;
	amf0.serialize(resp,m_clientLogString);
	message msg(m_inMsg);
	msg.m_messageData = resp;
	msg.m_messageType = RTMP_MSG__COMMAND_AMF0;
	createRTMPMsg(msg,TIMEFUNC() - m_serverBaseTime,(int)m_outboundChunkSize,m_outDataBuffer);
	//createRTMPMsg(RTMP_MSG__COMMAND_AMF0,m_outboundChunkSize,&(resp[0]),resp.size(),m_outDataBuffer);

	m_outDataPtr = &m_outDataBuffer[0];
	m_outDataSize = m_outDataBuffer.size();
	m_state = &protocol_RTMPClient::state_Send;
	m_nextState = &protocol_RTMPClient::state_WaitForMessage;

	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::handle_AMF0_play(const AMFEncoding &amf) throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	AMFEncoding amf0;
	amf0.appendValue(AMFVal(utf8("onStatus"))); // or "_error"
	amf0.appendValue(AMFVal(utf8("NetStream.Play.Start")));

	vector<__uint8> resp;
	amf0.serialize(resp,m_clientLogString);
	long ttt = TIMEFUNC();

	createRTMPMsg(RTMP_MSG__COMMAND_AMF0,ttt - m_serverBaseTime,(int)m_outboundChunkSize,&(resp[0]),resp.size(),m_outDataBuffer,m_inMsg.m_chunkStreamID,m_inMsg.m_messageStreamID);

	amf0.clear();
	amf0.appendValue(AMFVal(utf8("onHeaderData")));
	AMFEMCAArray amfA;
	amfA.addProperty("protocol",new AMFVal(utf8("ICY")));
	amfA.addProperty("content-type",new AMFVal(utf8("audio/aacp")));
	amf0.appendValue(AMFVal(amfA));
	resp.clear();
	amf0.serialize(resp,m_clientLogString);
	createRTMPMsg(RTMP_MSG__DATA_AMF0,ttt - m_serverBaseTime,(int)m_outboundChunkSize,&(resp[0]),resp.size(),m_outDataBuffer,m_inMsg.m_chunkStreamID,m_inMsg.m_messageStreamID);

	m_outDataPtr = &m_outDataBuffer[0];
	m_outDataSize = m_outDataBuffer.size();
	m_state = &protocol_RTMPClient::state_Send;
	m_nextState = &protocol_RTMPClient::state_SendAudio;

	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_SendConnectResponse() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	m_outDataBuffer.clear();

	__uint8 data[5];

	long ttt = TIMEFUNC();
	m_serverBaseTime = ttt;

	(*(__uint32*)data) = htonl(DEFAULT_SERVER_WINDOW);
	createRTMPMsg(RTMP_MSG__WINDOW_ACKNOWLEDGEMENT_SIZE,ttt - m_serverBaseTime,(int)m_outboundChunkSize,data,4,m_outDataBuffer);

	data[4] = RTMP_PEER_BANDWIDTH_DYNAMIC; //_SOFT;
	createRTMPMsg(RTMP_MSG__SET_PEER_BANDWIDTH,ttt - m_serverBaseTime,(int)m_outboundChunkSize,data,5,m_outDataBuffer);

	__uint8 streamBeginData[6] = {0,0,0,0,0,0};

	createRTMPMsg(RTMP_MSG__USER_CONTROL_MESSAGE,ttt - m_serverBaseTime,(int)m_outboundChunkSize,streamBeginData,6,m_outDataBuffer);

	__int32 chunkSize = htonl((int)m_outboundChunkSize);
	createRTMPMsg(RTMP_MSG__SET_CHUNK_SIZE,ttt - m_serverBaseTime,(int)m_outboundChunkSize,(const __uint8 *)&chunkSize,sizeof(chunkSize),m_outDataBuffer);

	AMFEncoding amf; //(m_objectEncodingMode > 1 ? 3 : 0);
	AMFObject obj;
	amf.appendValue(AMFVal(utf8("_result"))); // or "_error"
	amf.appendValue(AMFVal((double)1.0));
	obj.addProperty("fmsVer",new AMFVal(utf8("FMS/3,5,3,824a"))); //new AMFVal(utf8("sc_serv " + version.first + " " + version.second)));
	obj.addProperty("capabilities",new AMFVal((double)127)); //31)); // ????
	obj.addProperty("mode",new AMFVal((double)1));
	amf.appendValue(AMFVal(obj));
	obj.clearProperties();
	obj.addProperty("level",new AMFVal(utf8("status")));
	obj.addProperty("code",new AMFVal(utf8("NetConnection.Connect.Success")));
	obj.addProperty("description",new AMFVal(utf8("Connection succeeded.")));
	obj.addProperty("clientid",new AMFVal((double)795525197.0));
	obj.addProperty("objectEncoding",new AMFVal((double)m_objectEncodingMode));
	AMFEMCAArray arry;
	arry.addProperty("version",new AMFVal(utf8("FMS/3,5,3,824a")));
	obj.addProperty("data",new AMFVal(arry));

	amf.appendValue(AMFVal(obj));
	vector<__uint8> resp;
	amf.serialize(resp,m_clientLogString);
	createRTMPMsg(RTMP_MSG__COMMAND_AMF0,ttt - m_serverBaseTime,(int)m_outboundChunkSize,&(resp[0]),resp.size(),m_outDataBuffer,m_inMsg.m_chunkStreamID,m_inMsg.m_messageStreamID);

	m_outDataPtr = &m_outDataBuffer[0];
	m_outDataSize = m_outDataBuffer.size();
	m_state = &protocol_RTMPClient::state_Send;
	m_nextState = &protocol_RTMPClient::state_WaitForMessage;
	//m_nextState = &protocol_RTMPClient::state_SendAudio;

	NEXT_STATE
}

runnable::timeSliceResult protocol_RTMPClient::state_SendAudio() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

#ifdef TEST_FILE
	__uint8 buffer[1024];
	buffer[0] = 0xaf; //FLV audio header
	buffer[1] = 0x01;

	if (::fread(&(buffer[2]),1,1022,fff) != 1022)
		throwEx<runtime_error>("Test done!");

	m_outDataBuffer.clear();
	createRTMPMsg(RTMP_MSG__AUDIO_DATA,TIMEFUNC() - m_serverBaseTime,(int)m_outboundChunkSize,buffer,1024,m_outDataBuffer,m_inMsg.m_chunkStreamID,m_inMsg.m_messageStreamID);

	m_outDataPtr = &m_outDataBuffer[0];
	m_outDataSize = m_outDataBuffer.size();
	m_state = &protocol_RTMPClient::state_Send;
	m_nextState = &protocol_RTMPClient::state_SendAudio;

#endif	
	NEXT_STATE
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

static void encode3ByteValue(long v,__uint8 *data) throw()
{
	data[0] = (v >> 16) & 0xff;
	data[1] = (v >> 8) & 0xff;
	data[2] = (v & 0xff);
}

static void encode4ByteValue(long v,__uint8 *data) throw()
{
	data[0] = (v >> 24) & 0xff;
	data[1] = (v >> 16) & 0xff;
	data[2] = (v >> 8) & 0xff;
	data[3] = (v & 0xff);
}

static void chunkify(int chunkStreamID,int msgStreamID,__uint8 msgTypeID,long timestamp,int outboundChunkSize,const __uint8 *data,size_t dataSize,vector<__uint8> &result)
{
	assert(chunkStreamID >= 2);

	size_t chunkCount = 1;
	if (dataSize > 0)
		chunkCount = ((dataSize-1) / outboundChunkSize)+1;
	assert(chunkCount);

	for(size_t c = 0; c < chunkCount; ++c)
	{
		__uint8 header[18]; // largest possible header
		__uint8 *h = header;

		// size basic header
		if (chunkStreamID >=2 && chunkStreamID <= 63)
		{
			*(h++) = chunkStreamID;
		}
		else if (chunkStreamID >= 64 && chunkStreamID <= 319)
		{
			*(h++) = 0;
			*(h++) = (chunkStreamID - 64);
		}
		else
		{
			h[0] = 1;
			h[2] = ((chunkStreamID - 64) / 256);
			h[1] = (chunkStreamID - 64 - (h[2] * 256));
			h+= 3;
		}

		// fill in format bits and header
		if (c) 
		{
			header[0] |= 0xc0; // type 3
		}
		else
		{
			// type zero, full header
			encode3ByteValue(timestamp < 0x00ffffff ? timestamp : 0x00ffffff,h);
			h += 3;
			encode3ByteValue((int)dataSize,h);
			h += 3;
			*(h++) = msgTypeID;
			encode4ByteValue(msgStreamID,h);
			h += 4;
			if (timestamp >= 0x00ffffff)
			{
				encode4ByteValue(timestamp,h);
				h += 4;
			}		
		}

		// put header into result
		result.insert(result.end(),header,h);

		size_t payload_amt = min(dataSize,(size_t)outboundChunkSize);
		// put payload into result
		result.insert(result.end(),data,data + payload_amt);
		dataSize -= payload_amt;
		data += payload_amt;
	}

	assert(dataSize == 0);
}

static void createRTMPMsg(__uint8 msgType,long timestamp,int outboundChunkSize,const __uint8 *payload,size_t payloadSize,vector<__uint8> &result,
						int chunkStreamID,int msgStreamID)
{
	//chunkify(chunkStreamID,msgStreamID,msgType,TIMEFUNC(),outboundChunkSize,payload,payloadSize,result);
	protocol_RTMPClient::message msg;
	msg.m_chunkStreamID = chunkStreamID;
	msg.m_messageStreamID = msgStreamID;
	msg.m_messageType = msgType;
	msg.m_messageData.clear();
	msg.m_messageData.insert(msg.m_messageData.end(),payload,payload + payloadSize);
	createRTMPMsg(msg,timestamp,outboundChunkSize,result);

	#if 0
	// create a buffer with the message
	long t = TIMEFUNC();

	vector<__uint8> m;
	m.resize(11 + payloadSize);

	__uint8 *p = &m[0];
	memset(p,0,11);
	(*(__uint32*)p) = htonl(payloadSize);
	p[0] = msgType;
	(*(__uint32*)(&(p[4]))) = htonl(t);
	memmove(&(p[11]),payload,payloadSize);

	// chunkify
	chunkify(2 /* chunk stream id */,0 /* msg stream ID */ ,msgType,t,outboundChunkSize,p,11 + payloadSize,result);	
	#endif
}

static void createRTMPMsg(const protocol_RTMPClient::message &msg,long timestamp,int outboundChunkSize,vector<__uint8> &result)
{
	utf8 s = prettyPrintMessage(msg);
	DEBUG_LOG(" send " + eol() + 
		"CSID=" + tos(msg.m_chunkStreamID) + eol() + 
		"MSID=" + tos(msg.m_messageStreamID) + eol() + 
		"Time=" + tos(timestamp) + eol() + 
		"MTYPE=" + tos((int)msg.m_messageType) + eol() +
		"MLEN=" + tos(msg.m_messageData.size()) + eol() +  
		s);
	chunkify(msg.m_chunkStreamID,msg.m_messageStreamID,msg.m_messageType,timestamp,outboundChunkSize,&(msg.m_messageData[0]),msg.m_messageData.size(),result);
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

static int decode3ByteValue(const __uint8 *data,size_t offset = 0) throw()
{
	int result = 0;
	result += data[offset];
	result <<= 8;
	result += data[offset + 1];
	result <<= 8;
	result += data[offset + 2];
	return result;
}

static size_t decode4ByteValue(const __uint8 *data,size_t offset) throw()
{
	size_t result = 0;
	result += data[offset];
	result <<= 8;
	result += data[offset + 1];
	result <<= 8;
	result += data[offset + 2];
	result <<= 8;
	result += data[offset + 3];
	return result;
}

#if 0
void getRTMPMsgInfo(const vector<__uint8> &msg,__uint8 &msgType,int &payloadLength,long &timestamp,int &streamID,int &payloadOffset)
{
	assert(msg.size() >= 11);
	const __uint8 *p = &msg[0];
	msgType = *(p++);
	payloadLength = decode3ByteValue(p);
	p+=3;
	timestamp = decode4ByteValue(p);
	p += 4;	
	streamID = decode3ByteValue(p);
	p += 3;
	payloadOffset = 11;
}
#endif	

// get chunk type from first byte in basic header
static int chunkType(const __uint8 *basicHeader) throw()
{
	return (((*basicHeader) & 0xc0) >> 6);
}

static int calculateBasicChunkHeaderSize(const __uint8 *basicHeader) throw()
{
	int b = ((*basicHeader) & 0x3f);
	switch(b)
	{
		case 0: return 2;
		case 1: return 3;
	}
	return 1;
}

// look at complete basic header and determine how many bytes to expect in
// the chunk header	
static int calculateChunkMsgHeaderSize(const __uint8 *basicHeader) throw()
{
	switch(chunkType(basicHeader))
	{
		case 0: return 11;
		case 1: return 7;
		case 2: return 3;
	}
	return 0;
}

// calculate the complete size of a chunk header
static int calculateCompleteChunkHeaderSize(const __uint8 *basicHeader) throw()
{
	return calculateChunkMsgHeaderSize(basicHeader) + calculateBasicChunkHeaderSize(basicHeader);
}

// look at a complete chunk header (basic and msg header) and see if we need to get
// an extended timestamp for the chunk
static bool chunkNeedsExtendedTimestamp(const __uint8 *basicHeader) throw()
{
	int f = chunkType(basicHeader);
	if (f == 3) return false;
	int s = calculateBasicChunkHeaderSize(basicHeader);
	// timestamp is always just after the basic header
	return ((basicHeader[s] == 0xff) && (basicHeader[s+1] == 0xff) && (basicHeader[s+2] == 0xff));
}

// get chunkstream ID from the basic header	
static int getChunkStreamIDFromBasicHeader(const __uint8 *basicHeader) throw()
{
	int b = ((*basicHeader) & 0x3f);
	switch(b)
	{
		case 0: return (basicHeader[1] + 64);
		case 1: return (basicHeader[2] * 256 + basicHeader[1] + 64);
	}
	return b;
}

static int calculateMessageSize(const __uint8 *basicHeader) throw()
{
	if (chunkType(basicHeader) <= 1)
	{
		int s = calculateBasicChunkHeaderSize(basicHeader);
		return decode3ByteValue(basicHeader + s + 3);
	}
	return -1;
}

static int calculateMessageTypeID(const __uint8 *basicHeader) throw()
{
	if (chunkType(basicHeader) <= 1)
	{
		int s = calculateBasicChunkHeaderSize(basicHeader);
		return basicHeader[s + 6];
	}
	return -1;
}

static int calculateMessageStreamID(const __uint8 *basicHeader) throw()
{
	if (chunkType(basicHeader) == 0)
	{
		int s = calculateBasicChunkHeaderSize(basicHeader);
		return decode3ByteValue(basicHeader + s + 7);
	}
	return -1;
}

static long calculateTimestamp(const __uint8 *basicHeader) throw()
{
	return (chunkType(basicHeader) < 3 ? decode3ByteValue(basicHeader + calculateBasicChunkHeaderSize(basicHeader)) : 0);
}

// looks in inBuffer for a complete message. If it finds one it fills in msg and returns true.
// whether true or false, data from head of inBuffer should be removed based on amtToRemoveFromInBuffer
bool protocol_RTMPClient::chunkSequenceComplete(const __uint8 *inBuffer,size_t amtInBuffer,size_t &amtToRemoveFromInBuffer,size_t expectedChunkSize,
							vector<__uint8> &msg,__uint8 &msgType,int &chunkStreamID,int &messageStreamID,const uniString::utf8 &logMsgPrefix) throw(std::exception)
{
	bool result = false;
	msg.clear();
	amtToRemoveFromInBuffer = 0;

	// walk through chunks
	const __uint8 *pBegin = inBuffer;
	const __uint8 *pEnd = inBuffer + amtInBuffer;
	const __uint8 *p = inBuffer;

	chunkStreamID = -1;
	messageStreamID = -1;
	int messageLength = -1;
	int messageTypeID = -1;
	int payloadDataSeen = 0;
	int chunksConsolidated = 0; // for debugging
	while(p != pEnd)
	{
		chunksConsolidated += 1;

		// do all calculations necessary to see if we have a complete chunk
		int tmp;

		int hs = calculateCompleteChunkHeaderSize(p);
		if ((p + hs) > pEnd)	break; // not enough data
		if (chunkNeedsExtendedTimestamp(p))	
			hs += 4;
		if ((p + hs) > pEnd)	break; // not enough data

		// calculate timestamp
		long tt = calculateTimestamp(p);
		switch(chunkType(p))
		{
			case 0:
				m_lastInboundTimestamp = tt;
			break;

			case 1: case 2:
				m_lastInboundTimestamp += tt;
			break;
		}

		// gather and confirm chunk stream id
		tmp = getChunkStreamIDFromBasicHeader(p);
		if (chunkStreamID == -1)
			chunkStreamID = tmp;
		else if (chunkStreamID != tmp)
			throwEx<runtime_error>(logMsgPrefix + " expected chunk stream ID " + tos(chunkStreamID) + " but got " + tos(tmp) + " instead.");

		// gather and confirm message length for this chunk sequence
		tmp = calculateMessageSize(p);
		if ((tmp == -1) && (m_lastInboundMessageLength == -1))				throwEx<runtime_error>(logMsgPrefix + " No message length for chunk with chunk stream ID " + tos(chunkStreamID));
		if ((tmp != -1) && (messageLength != -1) && (tmp != messageLength))	throwEx<runtime_error>(logMsgPrefix + " mismatch message length for chunk with chunk stream ID " + tos(chunkStreamID) + " initially got " + tos(messageLength) + " then received " + tos(tmp));			
		if (tmp != -1)
			m_lastInboundMessageLength = messageLength = tmp;

		tmp = calculateMessageTypeID(p);
		if ((tmp == -1) && (m_lastInboundMessageTypeID == -1))				throwEx<runtime_error>(logMsgPrefix + " No message type ID for chunk with chunk stream ID " + tos(chunkStreamID));
		if ((tmp != -1) && (messageTypeID != -1) && (tmp != messageTypeID))	throwEx<runtime_error>(logMsgPrefix + " mismatch message type ID for chunk with chunk stream ID " + tos(chunkStreamID) + " initially got " + tos(messageTypeID) + " then received " + tos(tmp));
		if (tmp != -1)
			m_lastInboundMessageTypeID = messageTypeID = tmp;

		tmp = calculateMessageStreamID(p);
		if ((tmp == -1) && (m_lastInboundMessageStreamID == -1))				throwEx<runtime_error>(logMsgPrefix + " No message stream ID for chunk with chunk stream ID " + tos(chunkStreamID));
		if ((tmp != -1) && (messageStreamID != -1) && (tmp != messageStreamID))	throwEx<runtime_error>(logMsgPrefix + " mismatch message stream ID for chunk with stream ID " + tos(chunkStreamID) + " initially got " + tos(messageStreamID) + " then received " + tos(tmp));
		if (tmp != -1)
			m_lastInboundMessageStreamID = messageStreamID = tmp;

		// calculate data that should be in this chunk
		tmp = m_lastInboundMessageLength - payloadDataSeen;
		tmp = min(tmp,(int)expectedChunkSize);

		// see if we have enough
		if ((p + hs + tmp) > pEnd)
			break; // nope

		// yeah we do. Copy out data
		msg.insert(msg.end(),p+hs,p+hs+tmp);
		payloadDataSeen += tmp;
		p = p + hs + tmp;

		// are we all done?
		if (payloadDataSeen == m_lastInboundMessageLength)
		{
			// yes
			amtToRemoveFromInBuffer = p - pBegin;
			DEBUG_LOG(logMsgPrefix + " recv" + stringUtil::eol() + 
				" CSID=" + tos(chunkStreamID) + stringUtil::eol() +
				" MSID=" + tos(m_lastInboundMessageStreamID) + stringUtil::eol() +
				" Time=" + tos(m_lastInboundTimestamp) + stringUtil::eol() + 
				" MTYPE=" + tos(m_lastInboundMessageTypeID) + stringUtil::eol() +
				" LEN=" + tos(m_lastInboundMessageLength) + stringUtil::eol() + 
				" REMOVED=" + tos(amtToRemoveFromInBuffer));
					
			msgType = m_lastInboundMessageTypeID;
			messageStreamID = m_lastInboundMessageStreamID;
			return true;
		}
	}

	return result;
}

// return zero if get a msg, otherwise return timeout for read
long protocol_RTMPClient::getMsg(message &msg) throw(exception)
{
	size_t amt_left = m_inDataBufferMax - m_inDataBufferAmt;
	size_t amt_left2 = amt_left;

	long to = getSocketData(m_socket,m_inDataBuffer,m_inDataBufferAmt,amt_left,m_lastActivityTime,m_clientLogString);
	m_inDataBufferAmt += (amt_left2 - amt_left);

	size_t amtToRemoveFromInBuffer = 0;

	bool seqComplete = chunkSequenceComplete(m_inDataBuffer,m_inDataBufferAmt,amtToRemoveFromInBuffer,m_inboundChunkSize,msg.m_messageData,msg.m_messageType,msg.m_chunkStreamID,msg.m_messageStreamID,m_clientLogString);
	if (amtToRemoveFromInBuffer)
	{
		assert(m_inDataBufferAmt >= amtToRemoveFromInBuffer);
		if (m_inDataBufferAmt == amtToRemoveFromInBuffer)
		{
			m_inDataBufferAmt = 0;
			#ifndef NDEBUG
			memset(m_inDataBuffer,0,m_inDataBufferMax);
			#endif
		}
		else
		{
			memmove(m_inDataBuffer,m_inDataBuffer + amtToRemoveFromInBuffer,m_inDataBufferAmt - amtToRemoveFromInBuffer);
			m_inDataBufferAmt -= amtToRemoveFromInBuffer;
			#ifndef NDEBUG
			memset(m_inDataBuffer + m_inDataBufferAmt,0,m_inDataBufferMax - m_inDataBufferAmt);
			#endif
		}
	}
	if (seqComplete)
	{
		to = 0;
	}
	else
	{
		if (m_inDataBufferAmt == m_inDataBufferMax)
			throwEx<runtime_error>(m_clientLogString + " inbound data buffer exceeded");
		assert(to != 0); // ??? not sure
		if (to == 0) 
			to = 1;
	}

	return to;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

runnable::timeSliceResult protocol_RTMPClient::state_Close() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);
	timeSliceResult result;
	result.m_done = true;
	return result;
}

void protocol_RTMPClient::logW3C() throw()
{
}

#if 0
///////////////////////////////////// W3C Logging //////////////////////////////////////////////

static utf8 titleFromMetadata(const utf8 &md) throw()
{
	utf8 title;

	utf8::size_type p1 = utf8::npos;
	utf8::size_type p2 = utf8::npos;

	p1 = md.find(utf8("itle='"));
	if (p1 != utf8::npos)
	{
		p1 += 6;
		p2 = md.find(utf8("';"),p1);
		if (p2 != utf8::npos)
		{
			title = md.substr(p1,p2-p1);
		}
	}
	return title;
}

// create W3C entry. Entries describe the duration a client has listened to a specific title.
// the entry is generated on a title change, or when the client disconnects
void protocol_RTMPClient::logW3C() throw()
{			
	if (gOptions.w3cEnable())
	{
		time_t t(::time(0));
		time_t durationOfTitle = t - m_lastTitleTime;
		int		bitrateOfTitle = (int)(durationOfTitle ? (8 * m_bytesSentForCurrentTitle) / durationOfTitle : 0);
		utf8 title = titleFromMetadata(m_lastSentMetadata);

		w3cLog::log(m_clientAddr,
					m_clientHostName,
					m_streamID,
					title,
					mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"user-agent",utf8("")),
					m_bytesSentForCurrentTitle,
					durationOfTitle,
					bitrateOfTitle);

		setW3CState();
	}
}

// setup tracking variables for W3C log
void protocol_RTMPClient::setW3CState() throw()
{
	m_lastTitleTime = ::time(0);
	m_bytesSentForCurrentTitle = 0;
}

//////////////////////////////////////////////////////////////////////////////

void protocol_RTMPClient::aquireIntroFile() throw()
{
	m_streamData->getIntroFile().getSc1Data(m_introFile);
	m_introFileOffset = 0;
}

void protocol_RTMPClient::aquireBackupFile() throw()
{
	m_streamData->getBackupFile().getSc1Data(m_backupFile);
	m_backupFileOffset = 0;
}

runnable::timeSliceResult protocol_RTMPClient::state_SendText() throw(exception)
{
	//DLOG(__FUNCTION__);

	timeSliceResult result;
	long to = sendHTTPStyleText(m_socket,m_outBuffer,m_outBufferSize,m_lastActivityTime,m_clientLogString);
	if (to == 0)
	{ // sent
		m_state = m_nextState;
		result.m_runImmediately = true;
	}
	else
	{ // try again
		result.m_writeSet.insert(m_socket);
		result.m_timeout.tv_sec = to;
	}	
	return result;
}

runnable::timeSliceResult protocol_RTMPClient::state_AttachToStream() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	timeSliceResult result;

	assert(!m_streamData);

	m_streamID = DEFAULT_CLIENT_STREAM_ID;
	utf8::size_type pos = m_HTTPRequestInfo.m_url.find(utf8("/stream/"));
	if (pos != utf8::npos)
		m_streamID = atoi((const char *)m_HTTPRequestInfo.m_url.substr(pos + 8).c_str());

	map<int,config::streamConfig> stream_configs = gOptions.getStreamConfigs();
	for(map<int,config::streamConfig>::const_iterator i = stream_configs.begin(); i != stream_configs.end(); ++i)
	{
		if ((*i).second.m_urlPath == m_HTTPRequestInfo.m_url)
		{
			m_streamID = (*i).first;
			break;
		}

		pos = m_HTTPRequestInfo.m_url.find((*i).second.m_urlPath);
		if (pos != utf8::npos && pos == 0)
		{
			utf8 params = m_HTTPRequestInfo.m_url.substr(pos + (*i).second.m_urlPath.size());
			if(params.find(utf8(";")) == 0 || params.find(utf8("/")) == 0 || params.find(utf8("/;")) == 0)
			{
				m_streamID = (*i).first;
				break;
			}
		}
	}

	m_clientLogString = dstAddrLogString(m_clientHostName,m_clientPort,m_streamID);

	utf8 user_agent = toLower(mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"user-agent",utf8("")));

	bool isSourceActive = false;
	m_streamData = streamData::accessStream(m_streamID,isSourceActive);
	if (!m_streamData || !isSourceActive)
	{
		utf8 msg_icy401 = MSG_ICY401;
		msg_icy401.replace(msg_icy401.find(utf8("^")),1,gOptions.getVersionBuildStrings());
		m_outBuffer = msg_icy401.c_str();
		m_outBufferSize = strlen(m_outBuffer);

		m_state = &protocol_RTMPClient::state_SendText;
		m_nextState = &protocol_RTMPClient::state_Close;
		result.m_writeSet.insert(m_socket);
		result.m_timeout.tv_sec = gOptions.getAutoDumpSourceTime();
		ELOG(m_clientLogString + " SHOUTcast 1 client connection rejected. Stream not available. " + mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"user-agent",utf8("")));
	}
	else
	{
		// construct the response text
		if (!stats::addClient(m_streamID,this,m_clientAddr,m_clientPort,mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"user-agent",utf8("")),g_ripList.find(m_clientAddr)))
		{
			utf8 backup_server = m_streamData->streamBackupServer();
			if (backup_server.empty())
			{
				utf8 msg_icy503 = MSG_ICY503;
				msg_icy503.replace(msg_icy503.find(utf8("^")),1,gOptions.getVersionBuildStrings());
				m_outBuffer = msg_icy503.c_str();
				m_outBufferSize = strlen(m_outBuffer);

				ELOG(m_clientLogString + " SHOUTcast 1 client connection rejected. Max users reached. " + mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"user-agent",utf8("")));
			}
			else
			{
				m_redirectResponse = http302(backup_server);
				m_outBuffer = m_redirectResponse.c_str();
				m_outBufferSize = m_redirectResponse.size();
				WLOG(m_clientLogString + " SHOUTcast 1 client connection rejected. Max users reached. Redirecting to " + backup_server + ". " + mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"user-agent",utf8("")));
			}
			m_state = &protocol_RTMPClient::state_SendText;
			m_nextState = &protocol_RTMPClient::state_Close;
			result.m_writeSet.insert(m_socket);
			result.m_timeout.tv_sec = gOptions.getAutoDumpSourceTime();
		}
		else
		{
			m_removeClientFromStats = true;
			m_sendMetadata = mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"icy-metadata",false);

			m_metaInterval = gOptions.metaInterval();
			if (m_metaInterval < 256) m_metaInterval = 256; // clamp

			m_ICYOKResponse = MSG_ICY200;
			m_ICYOKResponse += "icy-name:" + m_streamData->streamName() + "\r\n";
			m_ICYOKResponse += "icy-genre:" + m_streamData->streamGenre() + "\r\n";
			m_ICYOKResponse += "icy-url:" + m_streamData->streamURL() + "\r\n";
			m_ICYOKResponse += "content-type:" + m_streamData->streamContentType() + "\r\n";

			if (isUserAgentRelay(user_agent) && (!m_streamData->allowPublicRelay()))
				m_ICYOKResponse += "icy-pub:0\r\n";
			else
				m_ICYOKResponse += "icy-pub:" + tos(m_streamData->streamPublic()) + "\r\n";
			if (m_sendMetadata)
				m_ICYOKResponse += "icy-metaint:" + tos(m_metaInterval) + "\r\n";
			m_ICYOKResponse += "icy-br:" + m_streamData->streamName() + "\r\n";
			m_ICYOKResponse += "\r\n";		

			DEBUG_LOG(m_clientLogString + " sending [" + m_ICYOKResponse + "]");
			m_outBuffer = m_ICYOKResponse.c_str();
			m_outBufferSize = strlen(m_outBuffer);
			m_state = &protocol_RTMPClient::state_SendText;
			m_nextState = &protocol_RTMPClient::state_InitiateStream;
			result.m_writeSet.insert(m_socket);
			result.m_timeout.tv_sec = gOptions.getAutoDumpSourceTime();
			ILOG(m_clientLogString + " SHOUTcast 1 client connection accepted. " + mapGet(m_HTTPRequestInfo.m_HTTPHeaders,"user-agent",utf8("")));
		}
	}

	return result;
	}

// set read pointer to a nice distance from the write pointer. Return that distance
streamData::ringBufferAccess_t protocol_RTMPClient::resetReadPtr() throw()
{
	m_readPtr = m_streamData->getSc1ClientStartPosition();
	return m_streamData->getSc1RingBuffer().m_writePtr - m_readPtr;
}

runnable::timeSliceResult protocol_RTMPClient::state_InitiateStream() throw(exception)
{
	DEBUG_LOG(__FUNCTION__);

	assert(m_streamData);

	m_metaIntervalCounter = 0;
	resetReadPtr();

	m_underruns = 0;

	// if we have an intro file send it, otherwise start streaming
	aquireIntroFile();

	m_state = (m_introFile.empty() ? &protocol_RTMPClient::state_Stream : &protocol_RTMPClient::state_SendIntroFile);

	setW3CState();

	timeSliceResult result;
	result.m_runImmediately = true;
	return result;
}

// construct the necessary metadata information and load into outbound buffers.
void protocol_RTMPClient::sendICYMetadata(const utf8 &md) throw()
{
	m_ICYMetadata.clear();
	m_ICYMetadata.push_back(1); // placeholder
	if (md != m_lastSentMetadata) // don't sent duplicates
	{
		m_ICYMetadata.insert(m_ICYMetadata.end(),md.begin(),md.end());
		if (!m_lastSentMetadata.empty())
			logW3C();
		m_lastSentMetadata = md;
	}
	unsigned int dlen = m_ICYMetadata.size();
	if (dlen == 1) dlen = 0;
	unsigned int l1=((dlen+15)&~15);
	m_ICYMetadata[0] = l1/16;
	unsigned int send_len = l1+1;
	m_ICYMetadata.insert(m_ICYMetadata.end(),send_len - m_ICYMetadata.size(),0);
	assert(m_ICYMetadata.size() == ((m_ICYMetadata[0] * 16)+1));
	m_metaIntervalCounter = 0;

	m_outBuffer = &m_ICYMetadata[0];
	m_outBufferSize = m_ICYMetadata.size();
	m_state = &protocol_RTMPClient::state_SendText;
}

// handle state where we are sending intro files	
runnable::timeSliceResult protocol_RTMPClient::state_SendIntroFile() throw(exception)
{
	assert(m_streamData);
	assert(!m_introFile.empty());

	timeSliceResult result;

	int autoDumpTime = gOptions.getAutoDumpSourceTime(); // don't want this value to change during this call

	m_limitTrigger.clear();

	size_t amt = m_introFile.size() - m_introFileOffset;

	if (amt == 0)
	{
		// we're done with the intro file
		m_introFile.clear();
		m_state = &protocol_RTMPClient::state_Stream;
		resetReadPtr();
		result.m_runImmediately = true;
	}
	else if ((m_metaIntervalCounter == m_metaInterval) && m_sendMetadata) // check to see if we have to send the metadata
	{
		sendICYMetadata("StreamTitle='';");

		m_nextState = &protocol_RTMPClient::state_SendIntroFile;
		result.m_writeSet.insert(m_socket);
		result.m_timeout.tv_sec = autoDumpTime;
	}
	else
	{
		// clamp amount to send if we are supporting metadata	
		if (m_sendMetadata)
		{
			amt = min(amt,(m_metaInterval - m_metaIntervalCounter));
		}

		// send
		time_t cur_time = ::time(0);
		if (autoDumpTime > 0 && (cur_time - m_lastActivityTime) >= autoDumpTime)
			throwEx<runtime_error>(m_clientLogString + " Timeout waiting to send data (" +
								   tos(cur_time) + " " + tos(m_lastActivityTime) + " [" + tos(cur_time - m_lastActivityTime) + "] )");

		if (!amt)
		{
			// nothing in the source
			result.m_runImmediately = true;
		}
		else
		{
			int rval = ::send(m_socket,(const char *)&(m_introFile[m_introFileOffset]),amt,0);
			if (rval == 0)
			{
				throwEx<runtime_error>(m_clientLogString + " Remote socket closed while sending data.");
			}
			else if (rval < 0)
			{
				rval = socketOps::errCode();
				if (rval != SOCKETOPS_WOULDBLOCK)
					throwEx<runtime_error>(((
											 #ifdef _WIN32
											 rval == WSAECONNABORTED || rval == WSAECONNRESET
											 #else
											 rval == ECONNABORTED || rval == ECONNRESET || rval == EPIPE
											 #endif
											) ? uniString::utf8("") : m_clientLogString + "Socket error while waiting to send data. " + socketErrString(rval)));
				result.m_timeout.tv_sec = (long)(autoDumpTime - (cur_time - m_lastActivityTime));
				result.m_writeSet.insert(m_socket);
			}
			else
			{
				m_bytesSentForCurrentTitle += rval;
				m_totalBytesSent += rval;
				m_lastActivityTime = ::time(NULL);
				m_metaIntervalCounter += rval;
				m_introFileOffset += rval;
				assert((!m_sendMetadata) || (m_metaIntervalCounter <= m_metaInterval));
				result.m_writeSet.insert(m_socket);
				result.m_timeout.tv_sec = autoDumpTime;
			}
		}
	}
	return result;
}

// handle state where we are sending backup files	
runnable::timeSliceResult protocol_RTMPClient::state_SendBackupFile() throw(exception)
{
	assert(m_streamData);
	assert(!m_backupFile.empty());

	timeSliceResult result;

	int autoDumpTime = gOptions.getAutoDumpSourceTime(); // don't want this value to change during this call

	m_limitTrigger.clear();

	size_t amt = m_backupFile.size() - m_backupFileOffset;

	if (streamData::isSourceConnected(m_streamData))
	{
		// we're done with the backup file
		m_backupFile.clear();
		resetReadPtr();
		m_state = &protocol_RTMPClient::state_Stream;

		result.m_runImmediately = true;
	}
	else if (amt == 0)
	{
		// we're done with the backup file. get more data
		aquireBackupFile();
		if (m_backupFile.empty())
		{ // it got cleared out from under us? Try and stream
			resetReadPtr();
			m_state = &protocol_RTMPClient::state_Stream;
		}

		result.m_runImmediately = true;
	}
	else if ((m_metaIntervalCounter == m_metaInterval) && m_sendMetadata) // check to see if we have to send the metadata
	{
		sendICYMetadata("StreamTitle='';");

		m_nextState = &protocol_RTMPClient::state_SendBackupFile;
		result.m_writeSet.insert(m_socket);
		result.m_timeout.tv_sec = autoDumpTime;
	}
	else
	{
		// clamp amount to send if we are supporting metadata	
		if (m_sendMetadata)
		{
			amt = min(amt,(m_metaInterval - m_metaIntervalCounter));
		}

		// send
		time_t cur_time = ::time(0);
		if (autoDumpTime > 0 && (cur_time - m_lastActivityTime) >= autoDumpTime)
			throwEx<runtime_error>(m_clientLogString + " Timeout waiting to send data (" +
								   tos(cur_time) + " " + tos(m_lastActivityTime) + " [" + tos(cur_time - m_lastActivityTime) + "] )");

		if (!amt)
		{
			// nothing in the source
			result.m_runImmediately = true;
		}
		else
		{
			int rval = ::send(m_socket,(const char *)&(m_backupFile[m_backupFileOffset]),amt,0);
			if (rval == 0)
			{
				throwEx<runtime_error>(m_clientLogString + " Remote socket closed while sending data.");
			}
			else if (rval < 0)
			{
				rval = socketOps::errCode();
				if (rval != SOCKETOPS_WOULDBLOCK)
					throwEx<runtime_error>(((
											 #ifdef _WIN32
											 rval == WSAECONNABORTED || rval == WSAECONNRESET
											 #else
											 rval == ECONNABORTED || rval == ECONNRESET || rval == EPIPE
											 #endif
											) ? uniString::utf8("") : m_clientLogString + "Socket error while waiting to send data. " + socketErrString(rval)));
				result.m_timeout.tv_sec = (long)(autoDumpTime - (cur_time - m_lastActivityTime));
				result.m_writeSet.insert(m_socket);
			}
			else
			{
				m_bytesSentForCurrentTitle += rval;
				m_totalBytesSent += rval;
				m_lastActivityTime = ::time(NULL);
				m_metaIntervalCounter += rval;
				m_backupFileOffset += rval;
				assert((!m_sendMetadata) || (m_metaIntervalCounter <= m_metaInterval));
				result.m_writeSet.insert(m_socket);
				result.m_timeout.tv_sec = autoDumpTime;
			}
		}
	}
	return result;
}

runnable::timeSliceResult protocol_RTMPClient::state_Stream() throw(exception)
{
	assert(m_streamData);

	timeSliceResult result;

	int autoDumpTime = gOptions.getAutoDumpSourceTime(); // don't want this value to change during this call

	m_limitTrigger.clear();

	// check to see if we have to send the metadata
	if ((m_metaIntervalCounter == m_metaInterval) && m_sendMetadata)
	{
		// send metadata
		sendICYMetadata(m_streamData->getSc1Metadata(m_readPtr).m_songTitle);

		m_nextState = &protocol_RTMPClient::state_Stream;
		result.m_writeSet.insert(m_socket);
		result.m_timeout.tv_sec = autoDumpTime;
	}
	else
	{
		streamData::ringBuffer_t rb = m_streamData->getSc1RingBuffer();

		streamData::ringBufferAccess_t amt = rb.m_writePtr - m_readPtr;
		if (amt > rb.m_bufferSize)
		{
			// the pointers are too far apart. Underrun
			m_underruns += 1;
			amt = resetReadPtr();
		}

		// clamp amount to send if we are supporting metadata	
		if (m_sendMetadata)
		{
			amt = min(amt,(streamData::ringBufferAccess_t)(m_metaInterval - m_metaIntervalCounter));
		}

		streamData::ringBufferAccess_t offset = m_readPtr & rb.m_ptrMask;
		// clamp again so we don't read passed end of buffer
		amt = min(amt,rb.m_bufferSize - offset);

		// send
		time_t cur_time = ::time(0);
		if (autoDumpTime > 0 && (cur_time - m_lastActivityTime) >= autoDumpTime)
			throwEx<runtime_error>(m_clientLogString + " Timeout waiting to send data (" +
								   tos(cur_time) + " " + tos(m_lastActivityTime) + " [" + tos(cur_time - m_lastActivityTime) + "] )");

		if (!amt)
		{
			// nothing in the source
			// If the source has gone away, and we have a backup file, send it.
			bool sendBackupFile = false;

			if (!streamData::isSourceConnected(m_streamData))
			{
				aquireBackupFile();
				sendBackupFile = !m_backupFile.empty();
			}
			if (sendBackupFile)
			{
				m_state = &protocol_RTMPClient::state_SendBackupFile;
				result.m_runImmediately = true;
			}
			else
			{	
				result.m_timeout.tv_sec = (long)(autoDumpTime - (cur_time - m_lastActivityTime));
				m_limitTrigger.clear();
				result.m_readSet.insert(m_limitTrigger.test());
				m_streamData->scheduleSc1LimitTrigger(&m_limitTrigger,m_readPtr);
			}
		}
		else
		{
			int rval = ::send(m_socket,(const char *)&(rb.m_data[offset]),amt,0);
			if (rval == 0)
			{
				throwEx<runtime_error>(m_clientLogString + " Remote socket closed while sending data.");
			}
			else if (rval < 0)
			{
				rval = socketOps::errCode();
				if (rval != SOCKETOPS_WOULDBLOCK)
					throwEx<runtime_error>(((
											 #ifdef _WIN32
											 rval == WSAECONNABORTED || rval == WSAECONNRESET
											 #else
											 rval == ECONNABORTED || rval == ECONNRESET || rval == EPIPE
											 #endif
											) ? uniString::utf8("") : m_clientLogString + "Socket error while waiting to send data. " + socketErrString(rval)));
				result.m_timeout.tv_sec = (long)(autoDumpTime - (cur_time - m_lastActivityTime));
				result.m_writeSet.insert(m_socket);
			}
			else
			{
				m_bytesSentForCurrentTitle += rval;
				m_totalBytesSent += rval;
				m_lastActivityTime = ::time(NULL);
				m_metaIntervalCounter += rval;
				assert((!m_sendMetadata) || (m_metaIntervalCounter <= m_metaInterval));
				m_readPtr += rval;
				result.m_writeSet.insert(m_socket);
				result.m_timeout.tv_sec = autoDumpTime;
			}
		}
	}
	return result;
}

#endif
#endif
