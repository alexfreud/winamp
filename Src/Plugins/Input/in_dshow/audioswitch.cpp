#include <windows.h>
#include <AtlBase.h>
#include <streams.h>
#include <strsafe.h>

#include <qnetwork.h>
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.
#include "audioswitch.h"

// Implements the CAudioSwitchRenderer class

CAudioSwitchRenderer::CAudioSwitchRenderer(REFCLSID RenderClass,  // CLSID for this renderer
        TCHAR *pName,          // Debug ONLY description
        LPUNKNOWN pUnk,        // Aggregated owner object
        HRESULT *phr) :        // General OLE return code

		CBaseFilter(pName, pUnk, &m_InterfaceLock, RenderClass),
		m_evComplete(TRUE),
		m_bAbort(FALSE),
		m_pPosition(NULL),
		m_ThreadSignal(TRUE),
		m_bStreaming(FALSE),
		m_bEOS(FALSE),
		m_bEOSDelivered(FALSE),
		m_dwAdvise(0),
		m_pQSink(NULL),
		m_bRepaintStatus(TRUE),
		m_SignalTime(0),
		m_bInReceive(FALSE),
		m_EndOfStreamTimer(0),
		m_inputSelected(0)
{
	for (int i = 0;i < 16;i++) m_pInputPin[i] = NULL;
	for (int i = 0;i < 16;i++) m_pMediaSample[i] = NULL;
	Ready();
#ifdef PERF
	m_idBaseStamp = MSR_REGISTER("BaseRenderer: sample time stamp");
	m_idBaseRenderTime = MSR_REGISTER("BaseRenderer: draw time (msec)");
	m_idBaseAccuracy = MSR_REGISTER("BaseRenderer: Accuracy (msec)");
#endif
}


// Delete the dynamically allocated IMediaPosition and IMediaSeeking helper
// object. The object is created when somebody queries us. These are standard
// control interfaces for seeking and setting start/stop positions and rates.
// We will probably also have made an input pin based on CAudioSwitchRendererInputPin
// that has to be deleted, it's created when an enumerator calls our GetPin

CAudioSwitchRenderer::~CAudioSwitchRenderer()
{
	ASSERT(m_bStreaming == FALSE);
	ASSERT(m_EndOfStreamTimer == 0);
	StopStreaming();
	ClearPendingSample();

	// Delete any IMediaPosition implementation

	if (m_pPosition)
	{
		delete m_pPosition;
		m_pPosition = NULL;
	}

	// Delete any input pin created

	for (int i = 0;i < 16;i++)
	{
		if (m_pInputPin[i])
		{
			delete m_pInputPin[i];
			m_pInputPin[i] = NULL;
		}
	}

	// Release any Quality sink

	ASSERT(m_pQSink == NULL);
}


// This returns the IMediaPosition and IMediaSeeking interfaces

HRESULT CAudioSwitchRenderer::GetMediaPositionInterface(REFIID riid, void **ppv)
{
	CAutoLock cRendererLock(&m_InterfaceLock);
	if (m_pPosition)
	{
		return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
	}

	HRESULT hr = NOERROR;

	// Create implementation of this dynamically since sometimes we may
	// never try and do a seek. The helper object implements a position
	// control interface (IMediaPosition) which in fact simply takes the
	// calls normally from the filter graph and passes them upstream

	m_pPosition = new CRendererPosPassThru(NAME("Renderer CPosPassThru"),
	                                       CBaseFilter::GetOwner(),
	                                       (HRESULT *) & hr,
	                                       GetPin(m_inputSelected));
	if (m_pPosition == NULL)
	{
		return E_OUTOFMEMORY;
	}

	if (FAILED(hr))
	{
		delete m_pPosition;
		m_pPosition = NULL;
		return E_NOINTERFACE;
	}
	return GetMediaPositionInterface(riid, ppv);
}


// Overriden to say what interfaces we support and where

STDMETHODIMP CAudioSwitchRenderer::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	// Do we have this interface

	if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking)
	{
		return GetMediaPositionInterface(riid, ppv);
	}
	else
	{
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}


// This is called whenever we change states, we have a manual reset event that
// is signalled whenever we don't won't the source filter thread to wait in us
// (such as in a stopped state) and likewise is not signalled whenever it can
// wait (during paused and running) this function sets or resets the thread
// event. The event is used to stop source filter threads waiting in Receive

HRESULT CAudioSwitchRenderer::SourceThreadCanWait(BOOL bCanWait)
{
	if (bCanWait == TRUE)
	{
		m_ThreadSignal.Reset();
	}
	else
	{
		m_ThreadSignal.Set();
	}
	return NOERROR;
}


#ifdef DEBUG 
// Dump the current renderer state to the debug terminal. The hardest part of
// the renderer is the window where we unlock everything to wait for a clock
// to signal it is time to draw or for the application to cancel everything
// by stopping the filter. If we get things wrong we can leave the thread in
// WaitForRenderTime with no way for it to ever get out and we will deadlock

void CAudioSwitchRenderer::DisplayRendererState()
{
	DbgLog((LOG_TIMING, 1, TEXT("\nTimed out in WaitForRenderTime")));

	// No way should this be signalled at this point

	BOOL bSignalled = m_ThreadSignal.Check();
	DbgLog((LOG_TIMING, 1, TEXT("Signal sanity check %d"), bSignalled));

	// Now output the current renderer state variables

	DbgLog((LOG_TIMING, 1, TEXT("Filter state %d"), m_State));

	DbgLog((LOG_TIMING, 1, TEXT("Abort flag %d"), m_bAbort));

	DbgLog((LOG_TIMING, 1, TEXT("Streaming flag %d"), m_bStreaming));

	DbgLog((LOG_TIMING, 1, TEXT("Clock advise link %d"), m_dwAdvise));

	DbgLog((LOG_TIMING, 1, TEXT("Current media sample %x"), m_pMediaSample[m_inputSelected]));

	DbgLog((LOG_TIMING, 1, TEXT("EOS signalled %d"), m_bEOS));

	DbgLog((LOG_TIMING, 1, TEXT("EOS delivered %d"), m_bEOSDelivered));

	DbgLog((LOG_TIMING, 1, TEXT("Repaint status %d"), m_bRepaintStatus));


	// Output the delayed end of stream timer information

	DbgLog((LOG_TIMING, 1, TEXT("End of stream timer %x"), m_EndOfStreamTimer));

	DbgLog((LOG_TIMING, 1, TEXT("Deliver time %s"), CDisp((LONGLONG)m_SignalTime)));


	// Should never timeout during a flushing state

	BOOL bFlushing = m_pInputPin[m_inputSelected]->IsFlushing();
	DbgLog((LOG_TIMING, 1, TEXT("Flushing sanity check %d"), bFlushing));

	// Display the time we were told to start at
	DbgLog((LOG_TIMING, 1, TEXT("Last run time %s"), CDisp((LONGLONG)m_tStart.m_time)));

	// Have we got a reference clock
	if (m_pClock == NULL) return ;

	// Get the current time from the wall clock

	CRefTime CurrentTime, StartTime, EndTime;
	m_pClock->GetTime((REFERENCE_TIME*) &CurrentTime);
	CRefTime Offset = CurrentTime - m_tStart;

	// Display the current time from the clock

	DbgLog((LOG_TIMING, 1, TEXT("Clock time %s"), CDisp((LONGLONG)CurrentTime.m_time)));

	DbgLog((LOG_TIMING, 1, TEXT("Time difference %dms"), Offset.Millisecs()));


	// Do we have a sample ready to render
	if (m_pMediaSample[m_inputSelected] == NULL) return ;

	m_pMediaSample[m_inputSelected]->GetTime((REFERENCE_TIME*)&StartTime, (REFERENCE_TIME*)&EndTime);
	DbgLog((LOG_TIMING, 1, TEXT("Next sample stream times (Start %d End %d ms)"),
	        StartTime.Millisecs(), EndTime.Millisecs()));

	// Calculate how long it is until it is due for rendering
	CRefTime Wait = (m_tStart + StartTime) - CurrentTime;
	DbgLog((LOG_TIMING, 1, TEXT("Wait required %d ms"), Wait.Millisecs()));
}
#endif


// Wait until the clock sets the timer event or we're otherwise signalled. We
// set an arbitrary timeout for this wait and if it fires then we display the
// current renderer state on the debugger. It will often fire if the filter's
// left paused in an application however it may also fire during stress tests
// if the synchronisation with application seeks and state changes is faulty

#define RENDER_TIMEOUT 10000

HRESULT CAudioSwitchRenderer::WaitForRenderTime()
{
	HANDLE WaitObjects[] = { m_ThreadSignal, m_RenderEvent };
	DWORD Result = WAIT_TIMEOUT;

	// Wait for either the time to arrive or for us to be stopped

	OnWaitStart();
	while (Result == WAIT_TIMEOUT)
	{
		Result = WaitForMultipleObjects(2, WaitObjects, FALSE, RENDER_TIMEOUT);

#ifdef DEBUG
		if (Result == WAIT_TIMEOUT) DisplayRendererState();
#endif

	}
	OnWaitEnd();

	// We may have been awoken without the timer firing

	if (Result == WAIT_OBJECT_0)
	{
		return VFW_E_STATE_CHANGED;
	}

	SignalTimerFired();
	return NOERROR;
}


// Poll waiting for Receive to complete.  This really matters when
// Receive may set the palette and cause window messages
// The problem is that if we don't really wait for a renderer to
// stop processing we can deadlock waiting for a transform which
// is calling the renderer's Receive() method because the transform's
// Stop method doesn't know to process window messages to unblock
// the renderer's Receive processing
void CAudioSwitchRenderer::WaitForReceiveToComplete()
{
	for (;;)
	{
		if (!m_bInReceive)
		{
			break;
		}

		MSG msg;
		//  Receive all interthread snedmessages
		PeekMessage(&msg, NULL, WM_NULL, WM_NULL, PM_NOREMOVE);

		Sleep(1);
	}

	// If the wakebit for QS_POSTMESSAGE is set, the PeekMessage call
	// above just cleared the changebit which will cause some messaging
	// calls to block (waitMessage, MsgWaitFor...) now.
	// Post a dummy message to set the QS_POSTMESSAGE bit again
	if (HIWORD(GetQueueStatus(QS_POSTMESSAGE)) & QS_POSTMESSAGE)
	{
		//  Send dummy message
		PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0);
	}
}

// A filter can have four discrete states, namely Stopped, Running, Paused,
// Intermediate. We are in an intermediate state if we are currently trying
// to pause but haven't yet got the first sample (or if we have been flushed
// in paused state and therefore still have to wait for a sample to arrive)

// This class contains an event called m_evComplete which is signalled when
// the current state is completed and is not signalled when we are waiting to
// complete the last state transition. As mentioned above the only time we
// use this at the moment is when we wait for a media sample in paused state
// If while we are waiting we receive an end of stream notification from the
// source filter then we know no data is imminent so we can reset the event
// This means that when we transition to paused the source filter must call
// end of stream on us or send us an image otherwise we'll hang indefinately


// Simple internal way of getting the real state

FILTER_STATE CAudioSwitchRenderer::GetRealState()
{
	return m_State;
}


// The renderer doesn't complete the full transition to paused states until
// it has got one media sample to render. If you ask it for its state while
// it's waiting it will return the state along with VFW_S_STATE_INTERMEDIATE

STDMETHODIMP CAudioSwitchRenderer::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
	CheckPointer(State, E_POINTER);

	if (WaitDispatchingMessages(m_evComplete, dwMSecs) == WAIT_TIMEOUT)
	{
		*State = m_State;
		return VFW_S_STATE_INTERMEDIATE;
	}
	*State = m_State;
	return NOERROR;
}


// If we're pausing and we have no samples we don't complete the transition
// to State_Paused and we return S_FALSE. However if the m_bAbort flag has
// been set then all samples are rejected so there is no point waiting for
// one. If we do have a sample then return NOERROR. We will only ever return
// VFW_S_STATE_INTERMEDIATE from GetState after being paused with no sample
// (calling GetState after either being stopped or Run will NOT return this)

HRESULT CAudioSwitchRenderer::CompleteStateChange(FILTER_STATE OldState)
{
	// Allow us to be paused when disconnected

	if (m_pInputPin[m_inputSelected]->IsConnected() == FALSE)
	{
		Ready();
		return S_OK;
	}

	// Have we run off the end of stream

	if (IsEndOfStream() == TRUE)
	{
		Ready();
		return S_OK;
	}

	// Make sure we get fresh data after being stopped

	if (HaveCurrentSample() == TRUE)
	{
		if (OldState != State_Stopped)
		{
			Ready();
			return S_OK;
		}
	}
	NotReady();
	return S_FALSE;
}


// When we stop the filter the things we do are:-

//      Decommit the allocator being used in the connection
//      Release the source filter if it's waiting in Receive
//      Cancel any advise link we set up with the clock
//      Any end of stream signalled is now obsolete so reset
//      Allow us to be stopped when we are not connected

STDMETHODIMP CAudioSwitchRenderer::Stop()
{
	CAutoLock cRendererLock(&m_InterfaceLock);

	// Make sure there really is a state change

	if (m_State == State_Stopped)
	{
		return NOERROR;
	}

	// Is our input pin connected

	if (m_pInputPin[m_inputSelected]->IsConnected() == FALSE)
	{
		NOTE("Input pin is not connected");
		m_State = State_Stopped;
		return NOERROR;
	}

	CBaseFilter::Stop();

	// If we are going into a stopped state then we must decommit whatever
	// allocator we are using it so that any source filter waiting in the
	// GetBuffer can be released and unlock themselves for a state change

	if (m_pInputPin[m_inputSelected]->Allocator())
	{
		m_pInputPin[m_inputSelected]->Allocator()->Decommit();
	}

	// Cancel any scheduled rendering

	SetRepaintStatus(TRUE);
	StopStreaming();
	SourceThreadCanWait(FALSE);
	ResetEndOfStream();
	CancelNotification();

	// There should be no outstanding clock advise
	ASSERT(CancelNotification() == S_FALSE);
	ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent, 0));
	ASSERT(m_EndOfStreamTimer == 0);

	Ready();
	WaitForReceiveToComplete();
	m_bAbort = FALSE;
	return NOERROR;
}


// When we pause the filter the things we do are:-

//      Commit the allocator being used in the connection
//      Allow a source filter thread to wait in Receive
//      Cancel any clock advise link (we may be running)
//      Possibly complete the state change if we have data
//      Allow us to be paused when we are not connected

STDMETHODIMP CAudioSwitchRenderer::Pause()
{
	CAutoLock cRendererLock(&m_InterfaceLock);
	FILTER_STATE OldState = m_State;
	ASSERT(m_pInputPin[m_inputSelected]->IsFlushing() == FALSE);

	// Make sure there really is a state change

	if (m_State == State_Paused)
	{
		return CompleteStateChange(State_Paused);
	}

	// Has our input pin been connected

	if (m_pInputPin[m_inputSelected]->IsConnected() == FALSE)
	{
		NOTE("Input pin is not connected");
		m_State = State_Paused;
		return CompleteStateChange(State_Paused);
	}

	// Pause the base filter class

	HRESULT hr = CBaseFilter::Pause();
	if (FAILED(hr))
	{
		NOTE("Pause failed");
		return hr;
	}

	// Enable EC_REPAINT events again

	SetRepaintStatus(TRUE);
	StopStreaming();
	SourceThreadCanWait(TRUE);
	CancelNotification();
	ResetEndOfStreamTimer();

	// If we are going into a paused state then we must commit whatever
	// allocator we are using it so that any source filter can call the
	// GetBuffer and expect to get a buffer without returning an error

	if (m_pInputPin[m_inputSelected]->Allocator())
	{
		m_pInputPin[m_inputSelected]->Allocator()->Commit();
	}

	// There should be no outstanding advise
	ASSERT(CancelNotification() == S_FALSE);
	ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent, 0));
	ASSERT(m_EndOfStreamTimer == 0);
	ASSERT(m_pInputPin[m_inputSelected]->IsFlushing() == FALSE);

	// When we come out of a stopped state we must clear any image we were
	// holding onto for frame refreshing. Since renderers see state changes
	// first we can reset ourselves ready to accept the source thread data
	// Paused or running after being stopped causes the current position to
	// be reset so we're not interested in passing end of stream signals

	if (OldState == State_Stopped)
	{
		m_bAbort = FALSE;
		ClearPendingSample();
	}
	return CompleteStateChange(OldState);
}


// When we run the filter the things we do are:-

//      Commit the allocator being used in the connection
//      Allow a source filter thread to wait in Receive
//      Signal the render event just to get us going
//      Start the base class by calling StartStreaming
//      Allow us to be run when we are not connected
//      Signal EC_COMPLETE if we are not connected

STDMETHODIMP CAudioSwitchRenderer::Run(REFERENCE_TIME StartTime)
{
	CAutoLock cRendererLock(&m_InterfaceLock);
	FILTER_STATE OldState = m_State;

	// Make sure there really is a state change

	if (m_State == State_Running)
	{
		return NOERROR;
	}

	// Send EC_COMPLETE if we're not connected

	if (m_pInputPin[m_inputSelected]->IsConnected() == FALSE)
	{
		NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)this);
		m_State = State_Running;
		return NOERROR;
	}

	Ready();

	// Pause the base filter class

	HRESULT hr = CBaseFilter::Run(StartTime);
	if (FAILED(hr))
	{
		NOTE("Run failed");
		return hr;
	}

	// Allow the source thread to wait
	ASSERT(m_pInputPin[m_inputSelected]->IsFlushing() == FALSE);
	SourceThreadCanWait(TRUE);
	SetRepaintStatus(FALSE);

	// There should be no outstanding advise
	ASSERT(CancelNotification() == S_FALSE);
	ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent, 0));
	ASSERT(m_EndOfStreamTimer == 0);
	ASSERT(m_pInputPin[m_inputSelected]->IsFlushing() == FALSE);

	// If we are going into a running state then we must commit whatever
	// allocator we are using it so that any source filter can call the
	// GetBuffer and expect to get a buffer without returning an error

	if (m_pInputPin[m_inputSelected]->Allocator())
	{
		m_pInputPin[m_inputSelected]->Allocator()->Commit();
	}

	// When we come out of a stopped state we must clear any image we were
	// holding onto for frame refreshing. Since renderers see state changes
	// first we can reset ourselves ready to accept the source thread data
	// Paused or running after being stopped causes the current position to
	// be reset so we're not interested in passing end of stream signals

	if (OldState == State_Stopped)
	{
		m_bAbort = FALSE;
		ClearPendingSample();
	}
	return StartStreaming();
}


// Return the number of input pins we support

int CAudioSwitchRenderer::GetPinCount()
{
	return 16;
}


// We only support one input pin and it is numbered zero

CBasePin *CAudioSwitchRenderer::GetPin(int n)
{
	CAutoLock cRendererLock(&m_InterfaceLock);
	HRESULT hr = NOERROR;
	ASSERT(n < 16 && n >= 0);

	// Should only ever be called with zero

	if (n > 16)
	{
		return NULL;
	}

	// Create the input pin if not already done so

	if (m_pInputPin[n] == NULL)
	{
		WCHAR t[256] = {0};
		StringCchPrintfW(t, 256, L"In%d", n);
		m_pInputPin[n] = new CAudioSwitchRendererInputPin(this, &hr, t);
	}
	return m_pInputPin[n];
}


// If "In" then return the IPin for our input pin, otherwise NULL and error

STDMETHODIMP CAudioSwitchRenderer::FindPin(LPCWSTR Id, IPin **ppPin)
{
	CheckPointer(ppPin, E_POINTER);

	int gotit = 0;
	for (int i = 0;i < 16;i++)
	{
		WCHAR t[256] = {0};
		StringCchPrintfW(t, 256, L"In%d", i);
		if (0 == lstrcmpW(Id, t))
		{
			gotit = 1;
			*ppPin = GetPin(i);
			ASSERT(*ppPin);
			(*ppPin)->AddRef();
		}
	}
	if (!gotit)
	{
		*ppPin = NULL;
		return VFW_E_NOT_FOUND;
	}
	return NOERROR;
}


// Called when the input pin receives an EndOfStream notification. If we have
// not got a sample, then notify EC_COMPLETE now. If we have samples, then set
// m_bEOS and check for this on completing samples. If we're waiting to pause
// then complete the transition to paused state by setting the state event

HRESULT CAudioSwitchRenderer::EndOfStream()
{
	// Ignore these calls if we are stopped

	if (m_State == State_Stopped)
	{
		return NOERROR;
	}

	// If we have a sample then wait for it to be rendered

	m_bEOS = TRUE;
	if (m_pMediaSample[m_inputSelected])
	{
		return NOERROR;
	}

	// If we are waiting for pause then we are now ready since we cannot now
	// carry on waiting for a sample to arrive since we are being told there
	// won't be any. This sets an event that the GetState function picks up

	Ready();

	// Only signal completion now if we are running otherwise queue it until
	// we do run in StartStreaming. This is used when we seek because a seek
	// causes a pause where early notification of completion is misleading

	if (m_bStreaming)
	{
		SendEndOfStream();
	}
	return NOERROR;
}


// When we are told to flush we should release the source thread

HRESULT CAudioSwitchRenderer::BeginFlush()
{
	// If paused then report state intermediate until we get some data

	if (m_State == State_Paused)
	{
		NotReady();
	}

	SourceThreadCanWait(FALSE);
	CancelNotification();
	ClearPendingSample();
	//  Wait for Receive to complete
	WaitForReceiveToComplete();
	return NOERROR;
}


// After flushing the source thread can wait in Receive again

HRESULT CAudioSwitchRenderer::EndFlush()
{
	// Reset the current sample media time
	if (m_pPosition) m_pPosition->ResetMediaTime();

	// There should be no outstanding advise

	ASSERT(CancelNotification() == S_FALSE);
	SourceThreadCanWait(TRUE);
	return NOERROR;
}


// We can now send EC_REPAINTs if so required

HRESULT CAudioSwitchRenderer::CompleteConnect(IPin *pReceivePin)
{
	SetRepaintStatus(TRUE);
	m_bAbort = FALSE;
	return NOERROR;
}


// Called when we go paused or running

HRESULT CAudioSwitchRenderer::Active()
{
	return NOERROR;
}


// Called when we go into a stopped state

HRESULT CAudioSwitchRenderer::Inactive()
{
	if (m_pPosition)
	{
		m_pPosition->ResetMediaTime();
	}
	//  People who derive from this may want to override this behaviour
	//  to keep hold of the sample in some circumstances
	ClearPendingSample();
	return NOERROR;
}


// Tell derived classes about the media type agreed

HRESULT CAudioSwitchRenderer::SetMediaType(const CMediaType *pmt)
{
	return NOERROR;
}


// When we break the input pin connection we should reset the EOS flags. When
// we are asked for either IMediaPosition or IMediaSeeking we will create a
// CPosPassThru object to handles media time pass through. When we're handed
// samples we store (by calling CPosPassThru::RegisterMediaTime) their media
// times so we can then return a real current position of data being rendered

HRESULT CAudioSwitchRenderer::BreakConnect()
{
	// Do we have a quality management sink

	if (m_pQSink)
	{
		m_pQSink->Release();
		m_pQSink = NULL;
	}

	// Check we have a valid connection

	int n = 0;
	for (int i = 0;i < 16;i++)
	{
		if (!m_pInputPin[i] || m_pInputPin[i]->IsConnected() == FALSE) { n++; continue; }

		// Check we are stopped before disconnecting
		if (m_State != State_Stopped && !m_pInputPin[i]->CanReconnectWhenActive())
		{
			return VFW_E_NOT_STOPPED;
		}
	}

	if (n == 16) return S_FALSE;

	SetRepaintStatus(FALSE);
	ResetEndOfStream();
	ClearPendingSample();
	m_bAbort = FALSE;
	return NOERROR;
}


// Retrieves the sample times for this samples (note the sample times are
// passed in by reference not value). We return S_FALSE to say schedule this
// sample according to the times on the sample. We also return S_OK in
// which case the object should simply render the sample data immediately

HRESULT CAudioSwitchRenderer::GetSampleTimes(IMediaSample *pMediaSample,
        REFERENCE_TIME *pStartTime,
        REFERENCE_TIME *pEndTime)
{
	ASSERT(m_dwAdvise == 0);
	ASSERT(pMediaSample);

	// If the stop time for this sample is before or the same as start time,
	// then just ignore it (release it) and schedule the next one in line
	// Source filters should always fill in the start and end times properly!

	if (SUCCEEDED(pMediaSample->GetTime(pStartTime, pEndTime)))
	{
		if (*pEndTime < *pStartTime)
		{
			return VFW_E_START_TIME_AFTER_END;
		}
	}
	else
	{
		// no time set in the sample... draw it now?
		return S_OK;
	}

	// Can't synchronise without a clock so we return S_OK which tells the
	// caller that the sample should be rendered immediately without going
	// through the overhead of setting a timer advise link with the clock

	if (m_pClock == NULL)
	{
		return S_OK;
	}
	return ShouldDrawSampleNow(pMediaSample, pStartTime, pEndTime);
}


// By default all samples are drawn according to their time stamps so we
// return S_FALSE. Returning S_OK means draw immediately, this is used
// by the derived video renderer class in its quality management.

HRESULT CAudioSwitchRenderer::ShouldDrawSampleNow(IMediaSample *pMediaSample,
        REFERENCE_TIME *ptrStart,
        REFERENCE_TIME *ptrEnd)
{
	return S_FALSE;
}


// We must always reset the current advise time to zero after a timer fires
// because there are several possible ways which lead us not to do any more
// scheduling such as the pending image being cleared after state changes

void CAudioSwitchRenderer::SignalTimerFired()
{
	m_dwAdvise = 0;
}


// Cancel any notification currently scheduled. This is called by the owning
// window object when it is told to stop streaming. If there is no timer link
// outstanding then calling this is benign otherwise we go ahead and cancel
// We must always reset the render event as the quality management code can
// signal immediate rendering by setting the event without setting an advise
// link. If we're subsequently stopped and run the first attempt to setup an
// advise link with the reference clock will find the event still signalled

HRESULT CAudioSwitchRenderer::CancelNotification()
{
	ASSERT(m_dwAdvise == 0 || m_pClock);
	DWORD_PTR dwAdvise = m_dwAdvise;

	// Have we a live advise link

	if (m_dwAdvise)
	{
		m_pClock->Unadvise(m_dwAdvise);
		SignalTimerFired();
		ASSERT(m_dwAdvise == 0);
	}

	// Clear the event and return our status

	m_RenderEvent.Reset();
	return (dwAdvise ? S_OK : S_FALSE);
}


// Responsible for setting up one shot advise links with the clock
// Return FALSE if the sample is to be dropped (not drawn at all)
// Return TRUE if the sample is to be drawn and in this case also
// arrange for m_RenderEvent to be set at the appropriate time

BOOL CAudioSwitchRenderer::ScheduleSample(IMediaSample *pMediaSample)
{
	REFERENCE_TIME StartSample, EndSample;

	// Is someone pulling our leg

	if (pMediaSample == NULL)
	{
		return FALSE;
	}

	// Get the next sample due up for rendering.  If there aren't any ready
	// then GetNextSampleTimes returns an error.  If there is one to be done
	// then it succeeds and yields the sample times. If it is due now then
	// it returns S_OK other if it's to be done when due it returns S_FALSE

	HRESULT hr = GetSampleTimes(pMediaSample, &StartSample, &EndSample);
	if (FAILED(hr))
	{
		return FALSE;
	}

	// If we don't have a reference clock then we cannot set up the advise
	// time so we simply set the event indicating an image to render. This
	// will cause us to run flat out without any timing or synchronisation

	if (hr == S_OK)
	{
		EXECUTE_ASSERT(SetEvent((HANDLE) m_RenderEvent));
		return TRUE;
	}

	ASSERT(m_dwAdvise == 0);
	ASSERT(m_pClock);
	ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent, 0));

	// We do have a valid reference clock interface so we can ask it to
	// set an event when the image comes due for rendering. We pass in
	// the reference time we were told to start at and also the current
	// stream time which is the offset from the start reference time

	hr = m_pClock->AdviseTime(
	         (REFERENCE_TIME) m_tStart,           // Start run time
	         StartSample,                         // Stream time
	         (HEVENT)(HANDLE) m_RenderEvent,      // Render notification
	         &m_dwAdvise);                       // Advise cookie

	if (SUCCEEDED(hr))
	{
		return TRUE;
	}

	// We could not schedule the next sample for rendering despite the fact
	// we have a valid sample here. This is a fair indication that either
	// the system clock is wrong or the time stamp for the sample is duff

	ASSERT(m_dwAdvise == 0);
	return FALSE;
}


// This is called when a sample comes due for rendering. We pass the sample
// on to the derived class. After rendering we will initialise the timer for
// the next sample, NOTE signal that the last one fired first, if we don't
// do this it thinks there is still one outstanding that hasn't completed

HRESULT CAudioSwitchRenderer::Render(IMediaSample *pMediaSample)
{
	// If the media sample is NULL then we will have been notified by the
	// clock that another sample is ready but in the mean time someone has
	// stopped us streaming which causes the next sample to be released

	if (pMediaSample == NULL)
	{
		return S_FALSE;
	}

	// If we have stopped streaming then don't render any more samples, the
	// thread that got in and locked us and then reset this flag does not
	// clear the pending sample as we can use it to refresh any output device

	if (m_bStreaming == FALSE)
	{
		return S_FALSE;
	}

	// Time how long the rendering takes

	OnRenderStart(pMediaSample);
	DoRenderSample(pMediaSample);
	OnRenderEnd(pMediaSample);

	return NOERROR;
}


// Checks if there is a sample waiting at the renderer

BOOL CAudioSwitchRenderer::HaveCurrentSample()
{
	CAutoLock cRendererLock(&m_RendererLock);
	return (m_pMediaSample[m_inputSelected] == NULL ? FALSE : TRUE);
}


// Returns the current sample waiting at the video renderer. We AddRef the
// sample before returning so that should it come due for rendering the
// person who called this method will hold the remaining reference count
// that will stop the sample being added back onto the allocator free list

IMediaSample *CAudioSwitchRenderer::GetCurrentSample()
{
	CAutoLock cRendererLock(&m_RendererLock);
	if (m_pMediaSample[m_inputSelected])
	{
		m_pMediaSample[m_inputSelected]->AddRef();
	}
	return m_pMediaSample[m_inputSelected];
}


// Called when the source delivers us a sample. We go through a few checks to
// make sure the sample can be rendered. If we are running (streaming) then we
// have the sample scheduled with the reference clock, if we are not streaming
// then we have received an sample in paused mode so we can complete any state
// transition. On leaving this function everything will be unlocked so an app
// thread may get in and change our state to stopped (for example) in which
// case it will also signal the thread event so that our wait call is stopped

HRESULT CAudioSwitchRenderer::PrepareReceive(IMediaSample *pMediaSample)
{
	CAutoLock cRendererLock(&m_InterfaceLock);
	m_bInReceive = TRUE;

	// Check our flushing and filter state

	HRESULT hr = m_pInputPin[m_inputSelected]->CBaseInputPin::Receive(pMediaSample);

	if (hr != NOERROR)
	{
		m_bInReceive = FALSE;
		return E_FAIL;
	}

	// Has the type changed on a media sample. We do all rendering
	// synchronously on the source thread, which has a side effect
	// that only one buffer is ever outstanding. Therefore when we
	// have Receive called we can go ahead and change the format
	// Since the format change can cause a SendMessage we just don't
	// lock
	if (m_pInputPin[m_inputSelected]->SampleProps()->pMediaType)
	{
		m_pInputPin[m_inputSelected]->SetMediaType((CMediaType *)m_pInputPin[m_inputSelected]->SampleProps()->pMediaType);
	}


	CAutoLock cSampleLock(&m_RendererLock);

	ASSERT(IsActive() == TRUE);
	ASSERT(m_pInputPin[m_inputSelected]->IsFlushing() == FALSE);
	ASSERT(m_pInputPin[m_inputSelected]->IsConnected() == TRUE);
	ASSERT(m_pMediaSample[m_inputSelected] == NULL);

	// Return an error if we already have a sample waiting for rendering
	// source pins must serialise the Receive calls - we also check that
	// no data is being sent after the source signalled an end of stream

	if (m_pMediaSample[m_inputSelected] || m_bEOS || m_bAbort)
	{
		Ready();
		m_bInReceive = FALSE;
		return E_UNEXPECTED;
	}

	// Store the media times from this sample
	if (m_pPosition) m_pPosition->RegisterMediaTime(pMediaSample);

	// Schedule the next sample if we are streaming

	if ((m_bStreaming == TRUE) && (ScheduleSample(pMediaSample) == FALSE))
	{
		ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent, 0));
		ASSERT(CancelNotification() == S_FALSE);
		m_bInReceive = FALSE;
		return VFW_E_SAMPLE_REJECTED;
	}

	// Store the sample end time for EC_COMPLETE handling
	m_SignalTime = m_pInputPin[m_inputSelected]->SampleProps()->tStop;

	// BEWARE we sometimes keep the sample even after returning the thread to
	// the source filter such as when we go into a stopped state (we keep it
	// to refresh the device with) so we must AddRef it to keep it safely. If
	// we start flushing the source thread is released and any sample waiting
	// will be released otherwise GetBuffer may never return (see BeginFlush)

	m_pMediaSample[m_inputSelected] = pMediaSample;
	m_pMediaSample[m_inputSelected]->AddRef();

	if (m_bStreaming == FALSE)
	{
		SetRepaintStatus(TRUE);
	}
	return NOERROR;
}


// Called by the source filter when we have a sample to render. Under normal
// circumstances we set an advise link with the clock, wait for the time to
// arrive and then render the data using the PURE virtual DoRenderSample that
// the derived class will have overriden. After rendering the sample we may
// also signal EOS if it was the last one sent before EndOfStream was called

HRESULT CAudioSwitchRenderer::Receive(IMediaSample *pSample)
{
	ASSERT(pSample);

	// It may return VFW_E_SAMPLE_REJECTED code to say don't bother

	HRESULT hr = PrepareReceive(pSample);
	ASSERT(m_bInReceive == SUCCEEDED(hr));
	if (FAILED(hr))
	{
		if (hr == VFW_E_SAMPLE_REJECTED)
		{
			return NOERROR;
		}
		return hr;
	}

	// We realize the palette in "PrepareRender()" so we have to give away the
	// filter lock here.
	if (m_State == State_Paused)
	{
		PrepareRender();
		// no need to use InterlockedExchange
		m_bInReceive = FALSE;
		{
			// We must hold both these locks
			CAutoLock cRendererLock(&m_InterfaceLock);
			if (m_State == State_Stopped)
				return NOERROR;
			m_bInReceive = TRUE;
		}
		Ready();
	}
	// Having set an advise link with the clock we sit and wait. We may be
	// awoken by the clock firing or by a state change. The rendering call
	// will lock the critical section and check we can still render the data

	hr = WaitForRenderTime();
	if (FAILED(hr))
	{
		m_bInReceive = FALSE;
		return NOERROR;
	}

	PrepareRender();

	//  Set this here and poll it until we work out the locking correctly
	//  It can't be right that the streaming stuff grabs the interface
	//  lock - after all we want to be able to wait for this stuff
	//  to complete
	m_bInReceive = FALSE;

	// We must hold both these locks
	CAutoLock cRendererLock(&m_InterfaceLock);

	// since we gave away the filter wide lock, the sate of the filter could
	// have chnaged to Stopped
	if (m_State == State_Stopped)
		return NOERROR;

	CAutoLock cSampleLock(&m_RendererLock);

	// Deal with this sample

	Render(m_pMediaSample[m_inputSelected]);
	ClearPendingSample();
	SendEndOfStream();
	CancelNotification();
	return NOERROR;
}


// This is called when we stop or are inactivated to clear the pending sample
// We release the media sample interface so that they can be allocated to the
// source filter again, unless of course we are changing state to inactive in
// which case GetBuffer will return an error. We must also reset the current
// media sample to NULL so that we know we do not currently have an image

HRESULT CAudioSwitchRenderer::ClearPendingSample()
{
	CAutoLock cRendererLock(&m_RendererLock);
	for (int i = 0;i < 16;i++)
	{
		if (m_pMediaSample[i])
		{
			m_pMediaSample[i]->Release();
			m_pMediaSample[i] = NULL;
		}
	}
	return NOERROR;
}

//  Do the timer callback work
void CAudioSwitchRenderer::TimerCallback()
{
	//  Lock for synchronization (but don't hold this lock when calling
	//  timeKillEvent)
	CAutoLock cRendererLock(&m_RendererLock);

	// See if we should signal end of stream now

	if (m_EndOfStreamTimer)
	{
		m_EndOfStreamTimer = 0;
		SendEndOfStream();
	}
}


// If we are at the end of the stream signal the filter graph but do not set
// the state flag back to FALSE. Once we drop off the end of the stream we
// leave the flag set (until a subsequent ResetEndOfStream). Each sample we
// get delivered will update m_SignalTime to be the last sample's end time.
// We must wait this long before signalling end of stream to the filtergraph

#define TIMEOUT_DELIVERYWAIT 50
#define TIMEOUT_RESOLUTION 10

HRESULT CAudioSwitchRenderer::SendEndOfStream()
{
	ASSERT(CritCheckIn(&m_RendererLock));
	if (m_bEOS == FALSE || m_bEOSDelivered || m_EndOfStreamTimer)
	{
		return NOERROR;
	}

	// If there is no clock then signal immediately
	if (m_pClock == NULL)
	{
		return NotifyEndOfStream();
	}

	// How long into the future is the delivery time

	REFERENCE_TIME Signal = m_tStart + m_SignalTime;
	REFERENCE_TIME CurrentTime;
	m_pClock->GetTime(&CurrentTime);
	LONG Delay = LONG((Signal - CurrentTime) / 10000);

	// Dump the timing information to the debugger

	NOTE1("Delay until end of stream delivery %d", Delay);
	NOTE1("Current %s", (LPCTSTR)CDisp((LONGLONG)CurrentTime));
	NOTE1("Signal %s", (LPCTSTR)CDisp((LONGLONG)Signal));

	// Wait for the delivery time to arrive

	if (Delay < TIMEOUT_DELIVERYWAIT)
	{
		return NotifyEndOfStream();
	}

	// Signal a timer callback on another worker thread

	m_EndOfStreamTimer = timeSetEvent((UINT) Delay,        // Period of timer
	                                  TIMEOUT_RESOLUTION,  // Timer resolution
	                                  EndOfStreamTimer,    // Callback function
	                                  DWORD_PTR(this),     // Used information
	                                  TIME_ONESHOT);      // Type of callback
	if (m_EndOfStreamTimer == 0)
	{
		return NotifyEndOfStream();
	}
	return NOERROR;
}


// Signals EC_COMPLETE to the filtergraph manager

HRESULT CAudioSwitchRenderer::NotifyEndOfStream()
{
	CAutoLock cRendererLock(&m_RendererLock);
	ASSERT(m_bEOS == TRUE);
	ASSERT(m_bEOSDelivered == FALSE);
	ASSERT(m_EndOfStreamTimer == 0);

	// Has the filter changed state

	if (m_bStreaming == FALSE)
	{
		ASSERT(m_EndOfStreamTimer == 0);
		return NOERROR;
	}

	// Reset the end of stream timer
	m_EndOfStreamTimer = 0;

	// If we've been using the IMediaPosition interface, set it's start
	// and end media "times" to the stop position by hand.  This ensures
	// that we actually get to the end, even if the MPEG guestimate has
	// been bad or if the quality management dropped the last few frames

	if (m_pPosition) m_pPosition->EOS();
	m_bEOSDelivered = TRUE;
	NOTE("Sending EC_COMPLETE...");
	return NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)this);
}


// Reset the end of stream flag, this is typically called when we transfer to
// stopped states since that resets the current position back to the start so
// we will receive more samples or another EndOfStream if there aren't any. We
// keep two separate flags one to say we have run off the end of the stream
// (this is the m_bEOS flag) and another to say we have delivered EC_COMPLETE
// to the filter graph. We need the latter otherwise we can end up sending an
// EC_COMPLETE every time the source changes state and calls our EndOfStream

HRESULT CAudioSwitchRenderer::ResetEndOfStream()
{
	ResetEndOfStreamTimer();
	CAutoLock cRendererLock(&m_RendererLock);

	m_bEOS = FALSE;
	m_bEOSDelivered = FALSE;
	m_SignalTime = 0;

	return NOERROR;
}


// Kills any outstanding end of stream timer

void CAudioSwitchRenderer::ResetEndOfStreamTimer()
{
	ASSERT(CritCheckOut(&m_RendererLock));
	if (m_EndOfStreamTimer)
	{
		timeKillEvent(m_EndOfStreamTimer);
		m_EndOfStreamTimer = 0;
	}
}


// This is called when we start running so that we can schedule any pending
// image we have with the clock and display any timing information. If we
// don't have any sample but we have queued an EOS flag then we send it. If
// we do have a sample then we wait until that has been rendered before we
// signal the filter graph otherwise we may change state before it's done

HRESULT CAudioSwitchRenderer::StartStreaming()
{
	CAutoLock cRendererLock(&m_RendererLock);
	if (m_bStreaming == TRUE)
	{
		return NOERROR;
	}

	// Reset the streaming times ready for running

	m_bStreaming = TRUE;
	timeBeginPeriod(1);
	OnStartStreaming();

	// There should be no outstanding advise
	ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent, 0));
	ASSERT(CancelNotification() == S_FALSE);

	// If we have an EOS and no data then deliver it now

	if (m_pMediaSample[m_inputSelected] == NULL)
	{
		return SendEndOfStream();
	}

	// Have the data rendered

	ASSERT(m_pMediaSample[m_inputSelected]);
	if (!ScheduleSample(m_pMediaSample[m_inputSelected]))
		m_RenderEvent.Set();

	return NOERROR;
}


// This is called when we stop streaming so that we can set our internal flag
// indicating we are not now to schedule any more samples arriving. The state
// change methods in the filter implementation take care of cancelling any
// clock advise link we have set up and clearing any pending sample we have

HRESULT CAudioSwitchRenderer::StopStreaming()
{
	CAutoLock cRendererLock(&m_RendererLock);
	m_bEOSDelivered = FALSE;

	if (m_bStreaming == TRUE)
	{
		m_bStreaming = FALSE;
		OnStopStreaming();
		timeEndPeriod(1);
	}
	return NOERROR;
}


// We have a boolean flag that is reset when we have signalled EC_REPAINT to
// the filter graph. We set this when we receive an image so that should any
// conditions arise again we can send another one. By having a flag we ensure
// we don't flood the filter graph with redundant calls. We do not set the
// event when we receive an EndOfStream call since there is no point in us
// sending further EC_REPAINTs. In particular the AutoShowWindow method and
// the DirectDraw object use this method to control the window repainting

void CAudioSwitchRenderer::SetRepaintStatus(BOOL bRepaint)
{
	CAutoLock cSampleLock(&m_RendererLock);
	m_bRepaintStatus = bRepaint;
}


// Pass the window handle to the upstream filter

void CAudioSwitchRenderer::SendNotifyWindow(IPin *pPin, HWND hwnd)
{
	IMediaEventSink *pSink;

	// Does the pin support IMediaEventSink
	HRESULT hr = pPin->QueryInterface(IID_IMediaEventSink, (void **) & pSink);
	if (SUCCEEDED(hr))
	{
		pSink->Notify(EC_NOTIFY_WINDOW, LONG_PTR(hwnd), 0);
		pSink->Release();
	}
	NotifyEvent(EC_NOTIFY_WINDOW, LONG_PTR(hwnd), 0);
}


// Signal an EC_REPAINT to the filter graph. This can be used to have data
// sent to us. For example when a video window is first displayed it may
// not have an image to display, at which point it signals EC_REPAINT. The
// filtergraph will either pause the graph if stopped or if already paused
// it will call put_CurrentPosition of the current position. Setting the
// current position to itself has the stream flushed and the image resent

#define RLOG(_x_) DbgLog((LOG_TRACE,1,TEXT(_x_)));

void CAudioSwitchRenderer::SendRepaint()
{
	CAutoLock cSampleLock(&m_RendererLock);
	ASSERT(m_pInputPin[m_inputSelected]);

	// We should not send repaint notifications when...
	//    - An end of stream has been notified
	//    - Our input pin is being flushed
	//    - The input pin is not connected
	//    - We have aborted a video playback
	//    - There is a repaint already sent

	if (m_bAbort == FALSE)
	{
		if (m_pInputPin[m_inputSelected]->IsConnected() == TRUE)
		{
			if (m_pInputPin[m_inputSelected]->IsFlushing() == FALSE)
			{
				if (IsEndOfStream() == FALSE)
				{
					if (m_bRepaintStatus == TRUE)
					{
						for (int i = 0;i < 16;i++)
						{
							IPin *pPin = (IPin *) m_pInputPin[i];
							if (!pPin) continue;
							NotifyEvent(EC_REPAINT, (LONG_PTR) pPin, 0);
							SetRepaintStatus(FALSE);
							RLOG("Sending repaint");
						}
					}
				}
			}
		}
	}
}


// When a video window detects a display change (WM_DISPLAYCHANGE message) it
// can send an EC_DISPLAY_CHANGED event code along with the renderer pin. The
// filtergraph will stop everyone and reconnect our input pin. As we're then
// reconnected we can accept the media type that matches the new display mode
// since we may no longer be able to draw the current image type efficiently

BOOL CAudioSwitchRenderer::OnDisplayChange()
{
	// Ignore if we are not connected yet

	CAutoLock cSampleLock(&m_RendererLock);
	int n = 0;
	for (int i = 0;i < 16;i++)
		if (!m_pInputPin[i] || m_pInputPin[i]->IsConnected() == FALSE) n++;
	if (n == 16)
		return FALSE;

	RLOG("Notification of EC_DISPLAY_CHANGE");

	// Pass our input pin as parameter on the event

	for (int i = 0;i < 16;i++)
		if (m_pInputPin[i] && m_pInputPin[i]->IsConnected())
		{
			IPin *pPin = (IPin *) m_pInputPin[i];
			m_pInputPin[i]->AddRef();
			NotifyEvent(EC_DISPLAY_CHANGED, (LONG_PTR) pPin, 0);
			SetAbortSignal(TRUE);
			ClearPendingSample();
			m_pInputPin[i]->Release();
		}

	return TRUE;
}


// Called just before we start drawing.
// Store the current time in m_trRenderStart to allow the rendering time to be
// logged.  Log the time stamp of the sample and how late it is (neg is early)

void CAudioSwitchRenderer::OnRenderStart(IMediaSample *pMediaSample)
{
#ifdef PERF
	REFERENCE_TIME trStart, trEnd;
	pMediaSample->GetTime(&trStart, &trEnd);

	MSR_INTEGER(m_idBaseStamp, (int)trStart);     // dump low order 32 bits

	m_pClock->GetTime(&m_trRenderStart);
	MSR_INTEGER(0, (int)m_trRenderStart);
	REFERENCE_TIME trStream;
	trStream = m_trRenderStart - m_tStart;     // convert reftime to stream time
	MSR_INTEGER(0, (int)trStream);

	const int trLate = (int)(trStream - trStart);
	MSR_INTEGER(m_idBaseAccuracy, trLate / 10000);  // dump in mSec
#endif

} // OnRenderStart


// Called directly after drawing an image.
// calculate the time spent drawing and log it.

void CAudioSwitchRenderer::OnRenderEnd(IMediaSample *pMediaSample)
{
#ifdef PERF
	REFERENCE_TIME trNow;
	m_pClock->GetTime(&trNow);
	MSR_INTEGER(0, (int)trNow);
	int t = (int)((trNow - m_trRenderStart) / 10000);   // convert UNITS->msec
	MSR_INTEGER(m_idBaseRenderTime, t);
#endif
} // OnRenderEnd

void CAudioSwitchRenderer::SetSelectedInput(int n)
{
	if (m_inputSelected == n) return ;
	if (n > 15 || n < 0) return ;
	ClearPendingSample();
	m_inputSelected = n;
	GetSelectedPin()->NotifyMediaType();
}

int CAudioSwitchRenderer::GetSelectedInput()
{
	return m_inputSelected;
}

int CAudioSwitchRenderer::GetConnectedInputsCount()
{
	int n = 0;
	for (int i = 0;i < 16;i++)
	{
		if (m_pInputPin[i] && m_pInputPin[i]->IsConnected()) n++;
	}
	return n;
}

// Constructor must be passed the base renderer object

CAudioSwitchRendererInputPin::CAudioSwitchRendererInputPin(CAudioSwitchRenderer *pRenderer,
        HRESULT *phr,
        LPCWSTR pPinName) :
		CBaseInputPin(NAME("Renderer pin"),
		              pRenderer,
		              &pRenderer->m_InterfaceLock,
		              (HRESULT *) phr,
		              pPinName)
{
	m_pRenderer = pRenderer;
	ASSERT(m_pRenderer);
}


// Signals end of data stream on the input pin

STDMETHODIMP CAudioSwitchRendererInputPin::EndOfStream()
{
	HRESULT hr = NOERROR;
	if (m_pRenderer->GetSelectedPin() == this)
	{
		CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
		CAutoLock cSampleLock(&m_pRenderer->m_RendererLock);

		// Make sure we're streaming ok

		hr = CheckStreaming();
		if (hr != NOERROR)
		{
			return hr;
		}

		// Pass it onto the renderer

		hr = m_pRenderer->EndOfStream();
	}
	if (SUCCEEDED(hr))
	{
		hr = CBaseInputPin::EndOfStream();
	}
	return hr;
}


// Signals start of flushing on the input pin - we do the final reset end of
// stream with the renderer lock unlocked but with the interface lock locked
// We must do this because we call timeKillEvent, our timer callback method
// has to take the renderer lock to serialise our state. Therefore holding a
// renderer lock when calling timeKillEvent could cause a deadlock condition

STDMETHODIMP CAudioSwitchRendererInputPin::BeginFlush()
{
	if (m_pRenderer->GetSelectedPin() == this)
	{
		CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
		{
			CAutoLock cSampleLock(&m_pRenderer->m_RendererLock);
			CBaseInputPin::BeginFlush();
			m_pRenderer->BeginFlush();
		}
		return m_pRenderer->ResetEndOfStream();
	}
	else return CBaseInputPin::BeginFlush();
}


// Signals end of flushing on the input pin

STDMETHODIMP CAudioSwitchRendererInputPin::EndFlush()
{
	HRESULT hr = NOERROR;
	if (m_pRenderer->GetSelectedPin() == this)
	{
		CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
		CAutoLock cSampleLock(&m_pRenderer->m_RendererLock);
		hr = m_pRenderer->EndFlush();
	}
	if (SUCCEEDED(hr))
	{
		hr = CBaseInputPin::EndFlush();
	}
	return hr;
}


// Pass the sample straight through to the renderer object

STDMETHODIMP CAudioSwitchRendererInputPin::Receive(IMediaSample *pSample)
{
	if (m_pRenderer->GetSelectedPin() != this)
		return NOERROR;
	return m_pRenderer->Receive(pSample);
}


// Called when the input pin is disconnected

HRESULT CAudioSwitchRendererInputPin::BreakConnect()
{
	if (m_pRenderer->GetSelectedPin() == this)
	{
		HRESULT hr = m_pRenderer->BreakConnect();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	return CBaseInputPin::BreakConnect();
}


// Called when the input pin is connected

HRESULT CAudioSwitchRendererInputPin::CompleteConnect(IPin *pReceivePin)
{
	if (m_pRenderer->GetSelectedPin() == this)
	{
		HRESULT hr = m_pRenderer->CompleteConnect(pReceivePin);
		if (FAILED(hr))
		{
			return hr;
		}
	}
	return CBaseInputPin::CompleteConnect(pReceivePin);
}


// Give the pin id of our one and only pin

STDMETHODIMP CAudioSwitchRendererInputPin::QueryId(LPWSTR *Id)
{
	CheckPointer(Id, E_POINTER);

	*Id = (LPWSTR)CoTaskMemAlloc(8);
	if (*Id == NULL)
	{
		return E_OUTOFMEMORY;
	}
	StringCbCopyW(*Id, 8, m_pName);
	return NOERROR;
}


// Will the filter accept this media type

HRESULT CAudioSwitchRendererInputPin::CheckMediaType(const CMediaType *pmt)
{
	return m_pRenderer->CheckMediaType(pmt);
}


// Called when we go paused or running

HRESULT CAudioSwitchRendererInputPin::Active()
{
	return m_pRenderer->Active();
}


// Called when we go into a stopped state

HRESULT CAudioSwitchRendererInputPin::Inactive()
{
	return m_pRenderer->Inactive();
}


// Tell derived classes about the media type agreed

HRESULT CAudioSwitchRendererInputPin::SetMediaType(const CMediaType *pmt)
{
	HRESULT hr = CBaseInputPin::SetMediaType(pmt);
	if (FAILED(hr))
	{
		return hr;
	}
	m_mt = *pmt;
	if (m_pRenderer->GetSelectedPin() != this)
		return NOERROR;
	return m_pRenderer->SetMediaType(pmt);
}

HRESULT CAudioSwitchRendererInputPin::NotifyMediaType()
{
	if (m_pRenderer->GetSelectedPin() != this)
		return NOERROR;
	return m_pRenderer->SetMediaType(&m_mt);
}
