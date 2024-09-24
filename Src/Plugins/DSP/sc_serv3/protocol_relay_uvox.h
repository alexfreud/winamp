#pragma once
#ifndef protocol_relay_uvox_H_
#define protocol_relay_uvox_H_

#include "threadedRunner.h"
#include "streamData.h"
#include "uvox2Common.h"

class protocol_relay_uvox: public runnable
{
private:
	const bool				m_backup;	// used to change log output depending on relay or backup usage
	bool					m_denied;			// used to prevent source disconnected messages e.g. for failed passwords

	short unsigned int		m_remainderSize;
	__uint8 *				m_remainder;

	const uniString::utf8	m_srcAddrName;
	const uniString::utf8	m_srcAddrNumeric;
	const uniString::utf8	m_srcURLpart;
	const uniString::utf8	m_srcLogString;

	///////////// for outgoing data //////////////////////////////
	__uint8 *				m_outData;
	const __uint8 *			m_outBuffer; // for outgoing data lines
	int						m_outBufferSize;

	const config::streamConfig m_originalRelayInfo;

	///////////// incoming data ////////////////////////////////////
	std::vector<__uint8>	m_inBuffer;
	////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////	
	//// data structures for assembling cached metadata
	typedef std::vector<__uint8> metadataFragment_t;
	struct metadataFragmentEntry_t
	{
		metadataFragment_t m_fragment;
		bool m_isValid;

		void clear() throw() {m_isValid = false; m_fragment.clear(); }
		metadataFragmentEntry_t() throw():m_isValid(false){}
	};

	typedef metadataFragmentEntry_t metadataFragmentCollection_t[MAX_METADATA_FRAGMENTS];
	#pragma pack(push, 1)
	struct metadataEntry_t
	{
		__uint16 m_expectedFragments;
		__uint16 m_receivedFragments;
		metadataFragmentCollection_t m_fragments;

		void clear() throw()
		{
			for (int x = 0; x < MAX_METADATA_FRAGMENTS; ++x)
			{
				m_fragments[x].clear();
			}
			m_receivedFragments = 0;
		}
		metadataEntry_t() throw() : m_expectedFragments(0), m_receivedFragments(0) { }
	};
	#pragma pack(pop)

	typedef __uint32 assemblyTableIndex_t;
	static assemblyTableIndex_t makeAssemblyTableIndex(__uint16 voxMsgType,__uint16 metadataID) throw() 
	{
		return ((((assemblyTableIndex_t)voxMsgType) << 16) | metadataID);
	}

	typedef std::map<assemblyTableIndex_t,metadataEntry_t> metadataAssemblyTable_t;
	metadataAssemblyTable_t	m_metadataAssemblyTable;
	/////////////////////////////////////////////////////

	streamData *m_streamData;

	typedef void (protocol_relay_uvox::*state_t)();

	void state_GetPacket() throw(std::exception);
	void state_SendBuffer() throw(std::exception);
	void state_GetStreamData() throw(std::exception);
	void state_Fail() throw(std::exception);
	void state_CloseConnection() throw(std::exception);

	state_t	m_state;
	state_t m_nextState;

	streamData::uvoxConfigData_t m_configData;

	void cleanup();

	template<typename T> void loadAndSendMsg(const T &msg, int type,state_t nextState) throw();

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_relay_uvox"; }

public:
	protocol_relay_uvox(const socketOps::tSOCKET s, const config::streamConfig &originalRelayInfo,
						const uniString::utf8 &srcAddrName, const uniString::utf8 &srcAddrNumeric,
						const int srcPort, const uniString::utf8 &srcURLpart,
						const httpHeaderMap_t &httpHeaders, const int originalBitrate = 0,
						const uniString::utf8& originalMimeType = "", const bool backup = false) throw(std::runtime_error);
	~protocol_relay_uvox() throw();
};

#endif
