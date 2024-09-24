#ifndef NULLSOFT_IN_DSHOW_CSAMPLECB_H
#define NULLSOFT_IN_DSHOW_CSAMPLECB_H

class CSampleCB
{
public:
	virtual void sample_cb(LONGLONG starttime, LONGLONG endtime, IMediaSample *pSample) { }
	virtual void endofstream() { }}
;

#endif
