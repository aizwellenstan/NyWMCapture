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
#include "NyWMCaptureInterface.h"

/*	CLSID
*/

/*	IID
*/


interface IBaseFilter;
interface ICaptureGraphBuilder2;
interface IGraphBuilder;
interface IMediaControl;
interface INySampleCB;
class ICaptureFactory;


class CNyWMCaptureMain :public INyWMCapture
{
private:
	struct{
		IGraphBuilder         *graphi;
		ICaptureGraphBuilder2 *builder;
		IBaseFilter           *capture;
		IBaseFilter           *render;
		IMediaControl         *mediactrl;
	}m_ds_objects;
	bool m_is_initialized;
	bool m_is_start;

	ULONG m_rate;
	INySampleCB* m_callback;
	LONG  m_width;
	LONG  m_height;
	const GUID* m_subtype;
private:
	ULONG m_RefCount;
public:
	CNyWMCaptureMain(void);
	~CNyWMCaptureMain(void);
public:
	//IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObj);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
public:
	//API
	HRESULT STDMETHODCALLTYPE Initialize(ULONG i_device_id,ULONG i_subtype_id,ULONG i_capture_category_id);
	HRESULT STDMETHODCALLTYPE Finalize(void);
	HRESULT STDMETHODCALLTYPE Start(void);
	HRESULT STDMETHODCALLTYPE Stop(void);
	HRESULT STDMETHODCALLTYPE SetCallBack(INySampleCB __RPC_FAR *i_callback);
	HRESULT STDMETHODCALLTYPE SetSize(INT32 i_width,INT32 i_height);
	HRESULT STDMETHODCALLTYPE SetSubType(INT32 i_subtype_id);
private:
	ICaptureFactory* CreateCaptureFactory(ULONG i_device_id);
	const GUID* PinCategory2GUID(ULONG i_pin_category_id);
	const GUID* SubtypeId2GUID(ULONG i_subtype_id);
	void SetCapturePinProperty(IBaseFilter* i_cam,ULONG i_subtype_id,LONG i_width,LONG i_height);
	const LPCTSTR PinCategory2Name(ULONG i_pin_category_id);
};
