#pragma once
#ifndef protocol_uvox2Source_H_
#define protocol_uvox2Source_H_

#include "threadedRunner.h"
#include "streamData.h"
#include "uvox2Common.h"
#include <map>

class streamData;

/*
	Runnable object that handles the uvox 2 and 2.1 source (broadcaster)
	protocol
*/

class protocol_uvox2Source: public runnable
{
private:
	int						m_srcStreamID;
	const u_short			m_srcPort;

	bool					m_flushCachedMetadata;	// flush cached metadata when source connects
	bool					m_denied;				// used to prevent source disconnected messages e.g. for failed passwords

	const uniString::utf8	m_srcAddr;
	uniString::utf8			m_srcLogString;

	///////////// for outgoing data //////////////////////////////
	__uint8					*m_outData;
	const __uint8			*m_outBuffer; // for outgoing data lines
	int						m_outBufferSize;
	///////////////////////////////////////////////////////////////

	/////////// temporary storage buffer for special file transfers (uvox 0x1050) /////////////////
	int						m_specialFileBytesExpected;
	std::vector<__uint8>	m_specialFileBytes;
	std::string				m_specialFileType;

	///////////// incoming data ////////////////////////////////////
	std::vector<__uint8>	m_inBuffer;
	////////////////////////////////////////////////////////////

	streamData				*m_streamData; // associated stream object

	///// source information
	uniString::utf8	m_srcUserID;
	uniString::utf8	m_cipherKey; // for uvox 2.1
	streamData::uvoxConfigData_t	m_configData;

	///////////////////////////////////////////////////////////////////////	
	//// data structures for assembling cached metadata
	typedef std::vector<__uint8>	metadataFragment_t;
	struct metadataFragmentEntry_t
	{
		metadataFragment_t	m_fragment;
		bool				m_isValid;

		void clear() throw()
		{
			m_isValid = false;
			m_fragment.clear();
		}
		metadataFragmentEntry_t() throw() : m_isValid(false) {}
	};

	typedef metadataFragmentEntry_t metadataFragmentCollection_t[MAX_METADATA_FRAGMENTS];
	struct metadataEntry_t
	{
		metadataFragmentCollection_t	m_fragments;
		__uint16						m_expectedFragments;
		__uint16						m_receivedFragments;

		void clear() throw()
		{
			for (int x = 0; x < MAX_METADATA_FRAGMENTS; ++x)
			{
				m_fragments[x].clear();
			}
			m_receivedFragments = 0;
		}
		metadataEntry_t() throw() : m_expectedFragments(0), m_receivedFragments(0) {}
	};

	typedef __uint32 assemblyTableIndex_t;
	static assemblyTableIndex_t makeAssemblyTableIndex(__uint16 voxMsgType,__uint16 metadataID) throw() 
	{
		return ((((assemblyTableIndex_t)voxMsgType) << 16) | metadataID);
	}

	typedef std::map<assemblyTableIndex_t,metadataEntry_t> metadataAssemblyTable_t;
	std::map<__uint16,metadataAssemblyTable_t> m_metadataAssemblyTable;
	/////////////////////////////////////////////////////

	typedef void (protocol_uvox2Source::*state_t)();

	state_t	m_state;
	state_t m_nextState;

	__uint8					*m_remainder;
	short unsigned int		m_remainderSize;
    int                     m_loop; // hack until the read handler can process more than 1 packet at a time

	void state_SendBuffer() throw(std::exception);
	void state_ConfirmPasswordGet() throw(std::exception);
	void state_ConfirmPassword() throw(std::exception);
	void state_SendCrypto() throw(std::exception);
	void state_CloseConnection() throw(std::exception);
	void state_StreamConfiguration() throw(std::exception);
	void state_StreamConfigurationGet() throw(std::exception);
	void state_GetPacket() throw(std::exception);
	void state_StreamDataGet() throw(std::exception);
	void state_GetStreamData() throw(std::exception);

	template<typename T> void loadAndSendMsg(const T &msg,int type,state_t nextState) throw();

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_uvox2Source"; }

public:
    protocol_uvox2Source (microConnection &mc, const __uint8 *firstPacket, const int sizeOfFirstPacket) throw(exception);

	protocol_uvox2Source(const socketOps::tSOCKET s, const uniString::utf8 &addr, const u_short port,
						 const __uint8 *firstPacket, const int sizeOfFirstPacket) throw(std::exception);
	virtual ~protocol_uvox2Source() throw();
};

#endif
