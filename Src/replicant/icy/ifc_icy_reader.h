/*
 *  ifc_icy_reader.h
 *  shoutcast_player
 *
 *  Created by Ben Allison on 2/1/08.
 *  Copyright 2008 Nullsoft, Inc. All rights reserved.
 *
 */
 #pragma once
#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "foundation/error.h"

class ifc_icy_reader : public Wasabi2::Dispatchable
{
protected:
	ifc_icy_reader() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_icy_reader() {}
public:
	size_t BytesBuffered() { return ICYReader_BytesBuffered(); }
	int Read(void *buffer, size_t length, size_t *readLength) { return ICYReader_Read(buffer, length, readLength); }
	int Peek(void *buffer, size_t length, size_t *readLength) { return ICYReader_Peek(buffer, length, readLength); }
	int IsClosed() { return ICYReader_IsClosed(); }
	int Run() { return ICYReader_Run(); }
	
	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual size_t WASABICALL ICYReader_BytesBuffered()=0;
	virtual int WASABICALL ICYReader_Read(void *buffer, size_t length, size_t *readLength)=0;
	virtual int WASABICALL ICYReader_Peek(void *buffer, size_t length, size_t *readLength)=0;
	virtual int WASABICALL ICYReader_IsClosed()=0;
	virtual int WASABICALL ICYReader_Run()=0;
	
};
