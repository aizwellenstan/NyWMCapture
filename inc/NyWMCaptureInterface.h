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
#include "unknwn.h"

interface INyWMCapture;
interface INySampleCB;
interface INySample;


/*	DEVICEŽí—Þ
*/ 
#define NyWMCapture_DeviceId_WM5        0x00000001
#define NyWMCapture_DeviceId_Win32      0x00000002
#define NyWMCapture_DeviceId_S01SH      0x00010001
#define NyWMCapture_DeviceId_WS007SH    0x00010002
/*	MediaSubType
*/
#define NyWMCapture_MediaSubType_RGB565 0x00000001

/*	PIN CATEGORY ID
*/
#define NyWMCapture_PinCategory_CAPTURE       0x00000001
#define NyWMCapture_PinCategory_PREVIEW       0x00000002
#define NyWMCapture_PinCategory_ANALOGVIDEOIN 0x00000003
#define NyWMCapture_PinCategory_VBI           0x00000004
#define NyWMCapture_PinCategory_VIDEOPORT     0x00000005
#define NyWMCapture_PinCategory_NABTS         0x00000006
#define NyWMCapture_PinCategory_EDS           0x00000007
#define NyWMCapture_PinCategory_TELETEXT      0x00000008
#define NyWMCapture_PinCategory_CC            0x00000009
#define NyWMCapture_PinCategory_STILL         0x0000000a


DEFINE_GUID(CLSID_NyDWMCapture,
	0x32f37e70, 0xb633, 0x4253, 0xb8, 0xe0, 0xa9, 0x9a, 0x1b, 0xbe, 0xea, 0x84);


MIDL_INTERFACE("f633a3d8-d61f-4059-bc45-aba52b975c13")
INyWMCapture:IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Initialize(ULONG i_device_id,ULONG i_subtype_id,ULONG i_capture_category_id);
	virtual HRESULT STDMETHODCALLTYPE Finalize(void);
	virtual HRESULT STDMETHODCALLTYPE Start(void);
	virtual HRESULT STDMETHODCALLTYPE Stop(void);
	virtual HRESULT STDMETHODCALLTYPE SetCallBack(INySampleCB __RPC_FAR *i_callback);
	virtual HRESULT STDMETHODCALLTYPE SetSize(INT32 i_width,INT32 i_height);
	virtual HRESULT STDMETHODCALLTYPE SetSubType(INT32 i_subtype_id);
};

MIDL_INTERFACE("9665C35C-8F46-48e0-A99D-61FFABB19607")
INySampleCB : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE OnSample(
		INySample __RPC_FAR *i_sample) = 0;
};

MIDL_INTERFACE("88468260-109A-4bb1-AED7-5BD39CFEBB7C")
INySample : public IUnknown
{
public:
	virtual void*  __RPC_FAR STDMETHODCALLTYPE GetData(void) = 0;
	virtual INT32   STDMETHODCALLTYPE GetDataSize(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE CopyToBuffer(void __RPC_FAR *o_data,INT32 i_start,INT32 i_length)=0;
};