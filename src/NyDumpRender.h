/* 
 * The MIT License
 * 
 * Copyright (c) 2008 nyatla
 * airmail(at)ebony.plala.or.jp
 * http://nyatla.jp/
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */
#pragma once

#include <streams.h>
class CNyDumpInputPin;
class CNyDump;
class CNySample;
interface INySampleCB;


/*------------------------------------------------------------
	Interface
------------------------------------------------------------*/



/*------------------------------------------------------------
	Object
------------------------------------------------------------*/


class CNyDumpRender :public CBaseRenderer
{
private:
	void *m_buffer;
	long m_video_width;	 //受け付けるビデオサイズ(w)
	long m_video_height; //受け付けるビデオサイズ(h)
	GUID m_video_subtype;//受け付けるビデオサブタイプ
	INySampleCB* m_callback;
	CNySample*   m_sample;
public:

	CNyDumpRender(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CNyDumpRender();

	DECLARE_IUNKNOWN
    // Overriden to say what interfaces we support where
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

//	DECLARE_IUNKNOWN;
	//CBaseRendererのオーバーライド
    STDMETHODIMP DoRenderSample(IMediaSample *pMediaSample);
    STDMETHODIMP CheckMediaType(const CMediaType *pMediaType);
	STDMETHODIMP SetMediaType(const CMediaType *pmt);
public:
	/*コントロールAPI*/
	void SetSize(INT32 i_width,INT32 i_height);
	void SetSubType(const GUID __RPC_FAR *i_subtype);
	void SetCallBack(INySampleCB __RPC_FAR *i_callback);
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
private:
	

//非公開操作
protected:
	void InitInfo(const CMediaType *pmt);
};
