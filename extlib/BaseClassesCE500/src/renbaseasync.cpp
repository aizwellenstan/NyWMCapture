//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of your Microsoft Windows CE
// Source Alliance Program license form.  If you did not accept the terms of
// such a license, you are not authorized to use this source code.
//
//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//
//--------------------------------------------------------------------------;

#include <streams.h>        // ActiveMovie base class definitions
#include <mmsystem.h>       // Needed for definition of timeGetTime
#include <limits.h>         // Standard data type limit definitions
#include <measure.h>        // Used for time critical log functions

#pragma warning(disable:4355)

static DWORD s_dwVideoThreadPriority = -1;

// Implements the CBaseRendererAsync class

CBaseRendererAsync::CBaseRendererAsync(REFCLSID RenderClass, // CLSID for this renderer
                                       TCHAR *pName,         // Debug ONLY description
                                       LPUNKNOWN pUnk,       // Aggregated owner object
                                       HRESULT *phr) :       // General OLE return code

CBaseRenderer(RenderClass, pName,pUnk,phr),
m_pScheduledSample(NULL)
{
    if (-1 == s_dwVideoThreadPriority)
    {
        s_dwVideoThreadPriority = 248 + THREAD_PRIORITY_NORMAL;

        HKEY hKey;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                          TEXT("SOFTWARE\\Microsoft\\DirectShow\\ThreadPriority"), 
                                          0, 0, &hKey))
        {
            DWORD dwType, dwSize, dwValue;

            // Use default value if the value isn't defined, or is invalidate
            dwSize = sizeof (DWORD);
            if (ERROR_SUCCESS == RegQueryValueEx (hKey, TEXT("Video"), NULL, &dwType, LPBYTE(&dwValue), &dwSize) &&
                REG_DWORD == dwType && dwValue < 256)
            {
                 s_dwVideoThreadPriority = dwValue;
            }

            RegCloseKey (hKey);
        }
    }
}


// Delete the dynamically allocated IMediaPosition and IMediaSeeking helper
// object. The object is created when somebody queries us. These are standard
// control interfaces for seeking and setting start/stop positions and rates.
// We will probably also have made an input pin based on CRendererInputPin
// that has to be deleted, it's created when an enumerator calls our GetPin

CBaseRendererAsync::~CBaseRendererAsync()
{
    KillThread();
    m_RendererQueue.Flush();
}



// Overriden to say what interfaces we support and where

STDMETHODIMP CBaseRendererAsync::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    return CBaseRenderer::NonDelegatingQueryInterface(riid,ppv);
}


// For the asynchronous renderer case, the source thread never waits.
HRESULT CBaseRendererAsync::SourceThreadCanWait(BOOL bCanWait)
{
    return NOERROR;
}


#ifdef DEBUG
// Dump the current renderer state to the debug terminal. The hardest part of
// the renderer is the window where we unlock everything to wait for a clock
// to signal it is time to draw or for the application to cancel everything
// by stopping the filter. If we get things wrong we can leave the thread in
// WaitForRenderTime with no way for it to ever get out and we will deadlock

void CBaseRendererAsync::DisplayRendererState()
{
    CBaseRenderer::DisplayRendererState();

    DbgLog((LOG_TIMING, 1, TEXT("Sample currently scheduled %d"),m_pScheduledSample));
}
#endif

HRESULT CBaseRendererAsync::WaitForRenderTime()
{
    // should not have been called for the asynchronous renderer case
    ASSERT(0);
    return E_FAIL;
}


void CBaseRendererAsync::WaitForReceiveToComplete()
{
    // return immediately for the asynchronous renderer case
}


// When we stop the filter the things we do are:-

//      Decommit the allocator being used in the connection
//      Release the source filter if it's waiting in Receive
//      Cancel any advise link we set up with the clock
//      Any end of stream signalled is now obsolete so reset
//      Allow us to be stopped when we are not connected

STDMETHODIMP CBaseRendererAsync::Stop()
{
    CAutoLock cRendererLock(&m_InterfaceLock);

    DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync::Stop")));

    // Make sure there really is a state change
    
    if (m_State == State_Stopped) {
        // Even though we're "stopped", we may have gotten into
        // an indeterminate-state during a Pause() operation, leaving our CAMThread
        // running.  Let's make sure we use KillThread() to make any of those naughty
        // threads exit.
        if( KillThread() == NOERROR )
        {
            DEBUGMSG( 1, ( TEXT( "WMT: Closing CAMThread down properly!\n" ) ) );
        }
        return NOERROR;
    }

    // Make the worker thread go to a known state
    StopThread();

    // Now call the base class
    return CBaseRenderer::Stop();
}


// When we pause the filter the things we do are:-
//
// 
//      Commit the allocator being used in the connection
//      Allow a source filter thread to wait in Receive
//      Cancel any clock advise link (we may be running)
//      Possibly complete the state change if we have data
//      Allow us to be paused when we are not connected

STDMETHODIMP CBaseRendererAsync::Pause()
{
    CAutoLock cRendererLock(&m_InterfaceLock);
    FILTER_STATE OldState = m_State;
    HRESULT hr;

    DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync::Pause")));
    
    if (m_State != State_Paused)
    {
        // if we're going to Pause from Stopped, we're starting.
        // Create our worker thread (if needed) and put it in running mode.
        if (m_State == State_Stopped)
        {
            if (!ThreadExists())
            {
                if (!Create())
                {
                    return E_OUTOFMEMORY;
                }
            }
            RunThread();
        }
        m_bFirstSample = TRUE;       // set after a state change
    }

    hr = CBaseRenderer::Pause();

    return hr;
}


// When we run the filter the things we do are:-

//      Commit the allocator being used in the connection
//      Allow a source filter thread to wait in Receive
//      Signal the render event just to get us going
//      Start the base class by calling StartStreaming
//      Allow us to be run when we are not connected
//      Signal EC_COMPLETE if we are not connected

STDMETHODIMP CBaseRendererAsync::Run(REFERENCE_TIME StartTime)
{
    CAutoLock cRendererLock(&m_InterfaceLock);
    FILTER_STATE OldState = m_State;
    HRESULT hr;

    DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync::Run")));
    
    if (m_State == State_Stopped)
    {
        // if we're going to Run from Stopped, we're starting.
        // Create our worker thread (if needed) and put it in running mode.
        if (!ThreadExists())
        {
            if (!Create())
            {
                return E_OUTOFMEMORY;
            }
        }
        RunThread();
        m_bFirstSample = TRUE;       // set after a state change
    }

    hr = CBaseRenderer::Run(StartTime);

    return hr;
}


HRESULT CBaseRendererAsync::BeginFlush()
{
    CAutoLock cSampleLock(&m_RendererLock);

    // If paused then report state intermediate until we get some data    
    if (m_State == State_Paused) {
        NotReady();
    }
    
    CancelNotification();
    return NOERROR;
}


HRESULT CBaseRendererAsync::EndFlush()
{
    // sync with renderer thread -- stop the worker thread from delivering samples...
    StopThread();  

    // now that we know the worker thread is in a stopped state, we can get the RendererLock
    CAutoLock cSampleLock(&m_RendererLock);

    // Cancel any outstanding advise
    CancelNotification();

    ClearPendingSample();
    m_RendererQueue.Flush();

    // Reset the current sample media time
    if (m_pPosition) m_pPosition->ResetMediaTime();
    
    // put the worker thread in running mode again
    RunThread();

    return NOERROR;
}


// Called when we go into a stopped state
HRESULT CBaseRendererAsync::Inactive()
{
    HRESULT hr = CBaseRenderer::Inactive();
    m_RendererQueue.Flush();
    return hr;
}

HRESULT CBaseRendererAsync::BreakConnect()
{
    HRESULT hr = CBaseRenderer::BreakConnect();
    m_RendererQueue.Flush();
    return hr;
}


// We must always reset the current advise time to zero after a timer fires
// because there are several possible ways which lead us not to do any more
// scheduling such as the pending image being cleared after state changes

void CBaseRendererAsync::SignalTimerFired()
{
    m_dwAdvise = 0;
    if (m_pScheduledSample)
    {
        m_pScheduledSample->SetScheduled(FALSE);
        m_pScheduledSample = NULL;
    }
}


//
// Called when the source delivers us a sample. We go through a few checks to
// make sure the sample can be rendered. We queue the sample for further processing
// by the renderer thread. If we are running (streaming) then we set an event to awaken
// the renderer thread.
//
HRESULT CBaseRendererAsync::PrepareReceive(IMediaSample *pMediaSample)
{
    CAutoLock cRendererLock(&m_InterfaceLock);
    
    // Check our flushing and filter state
    
    PERFLOG_TRACE_OBJECT(PERFLOG_EV_RENDERER_OBJECT_RECEIVED, PERFLOG_STREAM_VIDEO, -1, pMediaSample, -1, -1, -1, -1, -1);

    HRESULT hr = m_pInputPin->CBaseInputPin::Receive(pMediaSample);
    
    if (hr != NOERROR) {
        return E_FAIL;
    }
    
    // Has the type changed on a media sample. We do all rendering
    // synchronously on the source thread, which has a side effect
    // that only one buffer is ever outstanding. Therefore when we
    // have Receive called we can go ahead and change the format
    // Since the format change can cause a SendMessage we just don't
    // lock
    if (m_pInputPin->SampleProps()->pMediaType) {
        m_pInputPin->SetMediaType(
            (CMediaType *)m_pInputPin->SampleProps()->pMediaType);
    }
    
    
    CAutoLock cSampleLock(&m_RendererLock);
    
    ASSERT(IsActive() == TRUE);
    ASSERT(m_pInputPin->IsFlushing() == FALSE);
    ASSERT(m_pInputPin->IsConnected() == TRUE);
    ASSERT(m_pMediaSample == NULL);
    
    // check that no data is being sent after the source signalled an end of stream    
    if (m_bEOS || m_bAbort) {
        Ready();
        return E_UNEXPECTED;
    }
    
    // Store the media times from this sample
    if (m_pPosition) m_pPosition->RegisterMediaTime(pMediaSample);
    
    //
    // Queue the received sample. Note that we AddRef pMediaSample since we're 
    // returning without using it immediately.
    //
    CRendererSample *pRendererSample = new CRendererSample(pMediaSample);
    if (!pRendererSample)
    {
        return E_OUTOFMEMORY;
    }
    m_RendererQueue.Insert (pRendererSample);

    //
    // If we're streaming, set an event for proper scheduling by the worker thread.
    //
    if (m_bStreaming)
    {
        m_ScheduleEvent.Set();
    }

    // Store the sample end time for EC_COMPLETE handling
    m_SignalTime = m_pInputPin->SampleProps()->tStop;
    
    if (m_bStreaming == FALSE) {
        SetRepaintStatus(TRUE);
    }
    return NOERROR;
}


//
// Called by the source filter when we have a sample to render. We just prepare for
// receiving the sample, and return immediately. The sample is going to be processed
// by the renderer thread (scheduled and rendered).
//
HRESULT CBaseRendererAsync::Receive(IMediaSample *pSample)
{
    ASSERT(pSample);

    // It may return VFW_E_SAMPLE_REJECTED code to say don't bother
    
    HRESULT hr = PrepareReceive(pSample);
    if (FAILED(hr)) {
        if (hr == VFW_E_SAMPLE_REJECTED) {
            return NOERROR;
        }
        return hr;
    }

    Ready();   // signal we have a sample now

    return NOERROR;
}


BOOL CBaseRendererAsync::HaveCurrentSample()
{
    CAutoLock cRendererLock(&m_RendererLock);
    CRendererSample *pRendererSample;

//    return ((m_RendererQueue.Peek (&pRendererSample) == S_OK) ? TRUE : FALSE);
	return ((m_RendererQueue.Peek (&pRendererSample) == S_OK) ? TRUE : FALSE);
}


BOOL CBaseRendererAsync::ScheduleSample(IMediaSample *pMediaSample)
{
    // We override ShouldDrawSampleNow to add quality management
    
    return CBaseRenderer::ScheduleSample(pMediaSample);
}

//
// Verifies if there's any sample in our renderer sample queue, and if
// there's a sample which is not scheduled, schedule it.
//
HRESULT CBaseRendererAsync::ScheduleQueuedSample (void)
{
    CAutoLock cSampleLock(&m_RendererLock);        // make sure we don't try to schedule the same sample twice
    CRendererSample *pRendererSample;

//    HRESULT hr = m_RendererQueue.Peek (&pRendererSample);
	HRESULT hr = m_RendererQueue.Peek (&pRendererSample);
    if (hr == S_OK  &&  !pRendererSample->IsScheduled())
    {
        DWORD dwAdvise = m_dwAdvise;
        if (ScheduleSample (pRendererSample->MediaSample()))
        {
            pRendererSample->SetScheduled(TRUE);
            m_pScheduledSample = pRendererSample;
        }
        else
        {
            // could not schedule this sample, remove it from the queue and move on...
            hr = m_RendererQueue.Remove (&pRendererSample);
            if (hr != S_OK  ||  !pRendererSample)
            {
                return hr;
            }
            delete pRendererSample;          // we don't need the queued sample anymore
        }
    }
    return hr;
}


//
// Waits for the correct presentation time and then renders
// the sample.
//
HRESULT CBaseRendererAsync::ProcessQueuedSample (void)
{
    CRendererSample *pRendererSample;
    IMediaSample *pMediaSample;

    CAutoLock cSampleLock(&m_RendererLock);
    
    HRESULT hr = m_RendererQueue.Remove (&pRendererSample);
    if (hr != S_OK  ||  !pRendererSample)
    {
        return hr;
    }

    pMediaSample = pRendererSample->MediaSample();
    ASSERT (pMediaSample);
    if (!pMediaSample)
        return E_FAIL;

    //
    // BEWARE we sometimes keep the sample even after rendering it, 
    // such as when we go into a stopped state (we keep it
    // to refresh the device with) so we must AddRef it to keep it safely. If
    // we start flushing the source thread is released and any sample waiting
    // will be released otherwise GetBuffer may never return (see BeginFlush)
    //
    m_pMediaSample = pMediaSample;
    m_pMediaSample->AddRef();

    delete pRendererSample;          // we don't need the queued sample anymore

    PrepareRender();
        
    // since we gave away the filter wide lock, the state of the filter could
    // have changed to Stopped
    if (m_State == State_Stopped)
        return NOERROR;
    
    // Deal with this sample

    if (m_bFirstSample)
    {
        OnReceiveFirstSample(m_pMediaSample);
    }
    else
    {
        Render(m_pMediaSample);
    }

    m_bFirstSample = FALSE;

    ClearPendingSample();
    SendEndOfStream();
    CancelNotification();

    return NOERROR;
}


//
// This is called when we start running so that we can schedule any pending
// image we have with the clock and display any timing information. If we
// don't have any sample but we have queued an EOS flag then we send it. If
// we do have a sample then we wait until that has been rendered before we
// signal the filter graph otherwise we may change state before it's done
//
HRESULT CBaseRendererAsync::StartStreaming()
{
    CAutoLock cRendererLock(&m_RendererLock);
    CRendererSample *pRendererSample;

    if (m_bStreaming == TRUE) {
        return NOERROR;
    }
    
    // Reset the streaming times ready for running
    
    m_bStreaming = TRUE;
    timeBeginPeriod(1);
    OnStartStreaming();
    
    // There should be no outstanding advise
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(CancelNotification() == S_FALSE);
    
    //
    // If we have an EOS and no data then deliver it now
    //
//    if (m_RendererQueue.Peek(&pRendererSample) != S_OK)
	if (m_RendererQueue.Peek(&pRendererSample) != S_OK)
    {
        return SendEndOfStream();
    }

    //
    // Set an event for the worker thread to start scheduling
    //
    m_ScheduleEvent.Set();
    
    return NOERROR;
}


//
// This is called when we stop streaming so that we can set our internal flag
// indicating we are not to schedule any more samples arriving. The state
// change methods in the filter implementation take care of cancelling any
// clock advise link we have set up and clearing any pending samples we have
//
HRESULT CBaseRendererAsync::StopStreaming()
{
    CAutoLock cRendererLock(&m_RendererLock);
    m_bEOSDelivered = FALSE;

    if (m_bStreaming == TRUE) {
        m_bStreaming = FALSE;
        OnStopStreaming();
        timeEndPeriod(1);
    }
    return NOERROR;
}


//
// When a video window detects a display change (WM_DISPLAYCHANGE message) it
// can send an EC_DISPLAY_CHANGED event code along with the renderer pin. The
// filtergraph will stop everyone and reconnect our input pin. As we're then
// reconnected we can accept the media type that matches the new display mode
// since we may no longer be able to draw the current image type efficiently
//
BOOL CBaseRendererAsync::OnDisplayChange()
{
    BOOL bOk = CBaseRenderer::OnDisplayChange();
    m_RendererQueue.Flush();
    
    return bOk;
}


//
// Worker thread related functionality
//


HRESULT CBaseRendererAsync::RunThread()
{
    DbgLog((LOG_TRACE, 2, TEXT("CBaseRenderer::RunThread")));
    return CallWorker(CMD_RUN);
}

HRESULT CBaseRendererAsync::StopThread()
{
    DbgLog((LOG_TRACE, 2, TEXT("CBaseRenderer::StopThread")));
    return CallWorker(CMD_STOP);
}


HRESULT CBaseRendererAsync::KillThread()
{
    // !!! CAutoLock lock(&m_AccessLock);

    DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync:KillThread")));

    HRESULT hr = CallWorker(CMD_EXIT);
    if (FAILED(hr))
    {
	    return hr;
    }

    // wait for thread completion and then close
    // handle (and clear so we can start another later)
    Close();

    return NOERROR;
}


//
// This thread is running the whole time the graph is active. It needs to monitor the
// passing of stream time and wake up when it wants to render an object.
//
DWORD CBaseRendererAsync::ThreadProc()
{
    BOOL bExit = FALSE;
    DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync::ThreadProc - Starting thread = 0x%x"), m_hThread));

    while (!bExit)
    {
        Command req = GetRequest();
        switch (req)
        {
            case CMD_EXIT:
                DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync::ThreadProc - CMD_EXIT received")));
                m_RendererQueue.Flush();
                bExit = TRUE;
                Reply(NOERROR);
                break;

            case CMD_STOP:
                DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync::ThreadProc - CMD_STOP received")));
                m_RendererQueue.Flush();
                Reply(NOERROR);
                break;

            case CMD_RUN:
                HANDLE events[4];
                BOOL bRunning = TRUE;

                DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync::ThreadProc - CMD_RUN received")));
                Reply(NOERROR);

                DWORD dwCurrentPriority = CeGetThreadPriority(GetCurrentThread());

                if (dwCurrentPriority != s_dwVideoThreadPriority)
                    CeSetThreadPriority(GetCurrentThread(), s_dwVideoThreadPriority);

                events[0] = GetRequestHandle();
                events[1] = m_ThreadSignal;
                events[2] = (HANDLE)m_ScheduleEvent;
                events[3] = (HANDLE)m_RenderEvent;

                DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync::ThreadProc - Start running loop")));

                while (bRunning)
                {
                    DWORD Wait;

                    OnWaitStart();
                    Wait = WaitForMultipleObjects (4, events, FALSE, 200);
                    OnWaitEnd();
                    switch (Wait)
                    {
                        case WAIT_OBJECT_0:            // Received new command
                            DbgLog((LOG_TRACE, 3, TEXT("CBaseRendererAsync::ThreadProc: there's a new cmd request...!\tThread = 0x%x"), m_hThread));
                            bRunning = FALSE;
                            break;

                        case WAIT_OBJECT_0 + 1:        // Received a state change
                            DbgLog((LOG_TRACE, 3, TEXT("CBaseRendererAsync:ThreadProc: there's a state change...!\tThread = 0x%x"), m_hThread));
                            bRunning = FALSE;
                            break;

                        case WAIT_OBJECT_0 + 2:        // m_ScheduleEvent - a new object has been received, schedule it
                            DbgLog((LOG_TRACE, 5, TEXT("CBaseRendererAsync:ThreadProc: trying to schedule new sample\n")));
                            ScheduleQueuedSample();    // Schedule a new sample if there's one
                            break;

                        case WAIT_OBJECT_0 + 3:        // m_RenderEvent - time to render
                            DbgLog((LOG_TRACE, 5, TEXT("CBaseRendererAsync:ThreadProc: Time to render sample\n")));
                            SignalTimerFired();
                            ProcessQueuedSample();
                            ScheduleQueuedSample();    // Schedule a new sample if there's one
                            break;

                        case WAIT_TIMEOUT:
                            DbgLog((LOG_TRACE, 3, TEXT("CBaseRendererAsync:ThreadProc: we timed out waiting for a new object to schedule or dispatch...!\tThread = 0x%x"), m_hThread));
                            ScheduleQueuedSample();    // Schedule a new sample if there's one
                            break;
                    }
                }
                break;
        }
    }

    DbgLog((LOG_TRACE, 2, TEXT("CBaseRendererAsync:ThreadProc exiting... Thread = 0x%x"), m_hThread));

    return S_OK;
}


//
// CBaseVideoRendererAsync functions
//

CBaseVideoRendererAsync::CBaseVideoRendererAsync(
                                       REFCLSID RenderClass, // CLSID for this renderer
                                       TCHAR *pName,         // Debug ONLY description
                                       LPUNKNOWN pUnk,       // Aggregated owner object
                                       HRESULT *phr) :       // General OLE return code

CBaseRendererAsync(RenderClass,pName,pUnk,phr)
{
} // Constructor


// Destructor is just a placeholder

CBaseVideoRendererAsync::~CBaseVideoRendererAsync()
{
    ASSERT(m_dwAdvise == 0);
}


// Overidden to return our IQualProp interface
STDMETHODIMP CBaseVideoRendererAsync::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    // We return IQualProp and delegate everything else
    
    if (riid == IID_IQualProp) {
        return CVideoRendererQuality::NonDelegatingQueryInterface (riid, ppv);
    } else if (riid == IID_IQualityControl) {
        return GetInterface((IBaseFilter *) this, ppv);
    } else {
        return CBaseRendererAsync::NonDelegatingQueryInterface(riid,ppv);
    }
}


// Override JoinFilterGraph so that, just before leaving
// the graph we can send an EC_WINDOW_DESTROYED event

STDMETHODIMP
CBaseVideoRendererAsync::JoinFilterGraph(IFilterGraph *pGraph,LPCWSTR pName)
{
    // Since we send EC_ACTIVATE, we also need to ensure
    // we send EC_WINDOW_DESTROYED or the resource manager may be
    // holding us as a focus object
    if (!pGraph && m_pGraph) {
        
        // We were in a graph and now we're not
        // Do this properly in case we are aggregated
        IBaseFilter* pFilter;
        QueryInterface(IID_IBaseFilter,(void **) &pFilter);
        NotifyEvent(EC_WINDOW_DESTROYED, (LPARAM) pFilter, 0);
        pFilter->Release();
    }
    return CBaseFilter::JoinFilterGraph(pGraph, pName);
}


// The timing functions in this class are called by the window object and by
// the renderer's allocator.
// The windows object calls timing functions as it receives media sample
// images for drawing using GDI.
// The allocator calls timing functions when it starts passing DCI/DirectDraw
// surfaces which are not rendered in the same way; The decompressor writes
// directly to the surface with no separate rendering, so those code paths
// call direct into us.  Since we only ever hand out DCI/DirectDraw surfaces
// when we have allocated one and only one image we know there cannot be any
// conflict between the two.
//
// We use timeGetTime to return the timing counts we use (since it's relative
// performance we are interested in rather than absolute compared to a clock)
// The window object sets the accuracy of the system clock (normally 1ms) by
// calling timeBeginPeriod/timeEndPeriod when it changes streaming states


//
// IQualityControl methods
//
STDMETHODIMP CBaseVideoRendererAsync::SetSink( IQualityControl * piqc)
{   
    m_pQSink = piqc;
    
    return NOERROR;
} // SetSink


STDMETHODIMP CBaseVideoRendererAsync::Notify( IBaseFilter * pSelf, Quality q)
{
     return CVideoRendererQuality::NotifyQuality(q);
} // Notify


void CBaseVideoRendererAsync::PrepareRender()
{ 
    if (!m_bFirstFrameRendered)
    {
        NotifyEvent( EC_VIDEOFRAMEREADY, NULL, NULL );
        m_bFirstFrameRendered = TRUE;
    }
}


// Send a message to indicate what our supplier should do about quality.
HRESULT CBaseVideoRendererAsync::SendQuality(REFERENCE_TIME trLate,
                                        REFERENCE_TIME trRealStream)
{
    Quality q;

    CVideoRendererQuality::GetQuality(trLate, trRealStream, &q);
    
    // A specific sink interface may be set through IPin
    
    if (m_pQSink==NULL) {
        // Get our input pin's peer.  We send quality management messages
        // to any nominated receiver of these things (set in the IPin
        // interface), or else to our source filter.
        
        IQualityControl *pQC = NULL;
        IPin *pOutputPin = m_pInputPin->GetConnected();
        ASSERT(pOutputPin != NULL);
        
        // And get an AddRef'd quality control interface
        
        HRESULT hr = pOutputPin->QueryInterface(IID_IQualityControl,(void**) &pQC);
        if (SUCCEEDED(hr)) {
            m_pQSink = pQC;
        }
    }
    if (m_pQSink) {
        return m_pQSink->Notify(this,q);
    }
    
    return S_FALSE;
} // SendQuality


// We are called with a valid IMediaSample image to decide whether this is to
// be drawn or not.  There must be a reference clock in operation.
// Return S_OK if it is to be drawn Now (as soon as possible)
// Return S_FALSE if it is to be drawn when it's due
// Return an error if we want to drop it
// m_nNormal=-1 indicates that we dropped the previous frame and so this
// one should be drawn early.  Respect it and update it.
// Use current stream time plus a number of heuristics (detailed below)
// to make the decision

HRESULT CBaseVideoRendererAsync::ShouldDrawSampleNow(IMediaSample *pMediaSample,
                                                REFERENCE_TIME *ptrStart,
                                                REFERENCE_TIME *ptrEnd)
{
    REFERENCE_TIME trRealStream;     // the real time now expressed as stream time.
    REFERENCE_TIME trLate;

    m_pClock->GetTime(&trRealStream);
    trRealStream -= m_tStart;        // convert to stream time (this is a reftime)

    HRESULT hr = ComputeLateness (*ptrStart, trRealStream, &trLate);
    if (SUCCEEDED(hr))
    {
        hr = SendQuality (trLate, trRealStream);
        BOOL bSupplierHandlingQuality = (hr == S_OK);
        hr = CVideoRendererQuality::ShouldDrawSampleNow(pMediaSample, ptrStart, ptrEnd, trRealStream, bSupplierHandlingQuality);
    }
    return hr;
} // ShouldDrawSampleNow



// NOTE we're called by both the window thread and the source filter thread
// so we have to be protected by a critical section (locked before called)
// Also, when the window thread gets signalled to render an image, it always
// does so regardless of how late it is. All the degradation is done when we
// are scheduling the next sample to be drawn. Hence when we start an advise
// link to draw a sample, that sample's time will always become the last one
// drawn - unless of course we stop streaming in which case we cancel links

BOOL CBaseVideoRendererAsync::ScheduleSample(IMediaSample *pMediaSample)
{
    // We override ShouldDrawSampleNow to add quality management
    
    DbgLog((LOG_TRACE, 5, TEXT("CVideoBaseRendererAsync:ThreadProc: ScheduleSample %x\n"), pMediaSample));

    BOOL bDrawImage = CBaseRendererAsync::ScheduleSample(pMediaSample);
    if (bDrawImage == FALSE) {
        CVideoRendererQuality::FrameDropped (pMediaSample);
        DbgLog((LOG_TRACE, 5, TEXT("CVideoBaseRendererAsync:ThreadProc: ScheduleSample DROPPED FRAME\n")));
    }
    return bDrawImage;
}

// Implementation of IQualProp interface needed to support the property page
// This is how the property page gets the data out of the scheduler. We are
// passed into the constructor the owning object in the COM sense, this will
// either be the video renderer or an external IUnknown if we're aggregated.
// We initialise our CUnknown base class with this interface pointer. Then
// all we have to do is to override NonDelegatingQueryInterface to expose
// our IQualProp interface. The AddRef and Release are handled automatically
// by the base class and will be passed on to the appropriate outer object

STDMETHODIMP CBaseVideoRendererAsync::get_FramesDroppedInRenderer(int *pcFramesDropped)
{
    CAutoLock cVideoLock(&m_InterfaceLock);
    return CVideoRendererQuality::get_FramesDroppedInRenderer (pcFramesDropped);
} // get_FramesDroppedInRenderer


// Set *pcFramesDrawn to the number of frames drawn since
// streaming started.

STDMETHODIMP CBaseVideoRendererAsync::get_FramesDrawn( int *pcFramesDrawn)
{
    CAutoLock cVideoLock(&m_InterfaceLock);
    return CVideoRendererQuality::get_FramesDrawn (pcFramesDrawn);
} // get_FramesDrawn


// Set iAvgFrameRate to the frames per hundred secs since
// streaming started.  0 otherwise.

STDMETHODIMP CBaseVideoRendererAsync::get_AvgFrameRate( int *piAvgFrameRate)
{
    CAutoLock cVideoLock(&m_InterfaceLock);
    return CVideoRendererQuality::get_AvgFrameRate (piAvgFrameRate, m_bStreaming);
} // get_AvgFrameRate


// Set *piAvg to the average sync offset since streaming started
// in mSec.  The sync offset is the time in mSec between when the frame
// should have been drawn and when the frame was actually drawn.

STDMETHODIMP CBaseVideoRendererAsync::get_AvgSyncOffset( int *piAvg)
{
    CAutoLock cVideoLock(&m_InterfaceLock);
    return CVideoRendererQuality::get_AvgSyncOffset (piAvg);
} // get_AvgSyncOffset

//
//  Do estimates for standard deviations for per-frame
//  statistics
//
HRESULT CBaseVideoRendererAsync::GetStdDev(
                                      int nSamples,
                                      int *piResult,
                                      LONGLONG llSumSq,
                                      LONGLONG iTot
                                      )
{
    CAutoLock cVideoLock(&m_InterfaceLock);
    return CVideoRendererQuality::GetStdDev (nSamples, piResult, llSumSq, iTot);
}

// Set *piDev to the standard deviation in mSec of the sync offset
// of each frame since streaming started.

STDMETHODIMP CBaseVideoRendererAsync::get_DevSyncOffset( int *piDev)
{
    CAutoLock cVideoLock(&m_InterfaceLock);
    return CVideoRendererQuality::get_DevSyncOffset (piDev);
} // get_DevSyncOffset


// Set *piJitter to the standard deviation in mSec of the inter-frame time
// of frames since streaming started.

STDMETHODIMP CBaseVideoRendererAsync::get_Jitter( int *piJitter)
{
    CAutoLock cVideoLock(&m_InterfaceLock);
    return CVideoRendererQuality::get_Jitter (piJitter);
} // get_Jitter


// This removes a large number of level 4 warnings from the
// Microsoft compiler which in this case are not very useful
#pragma warning(disable: 4514)

