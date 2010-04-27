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
#include "stdafx.h"
#include "NySample.h"
#include <windows.h>

const IID IID_INySample=
	{0x88468260, 0x109A, 0x4bb1, 0xAE, 0xD7, 0x5B,0xD3,0x9C,0xFE,0xBB,0x7C};

CNySample::CNySample(void)
{
	this->m_RefCount  =0;
	this->m_data=NULL;
	this->m_data_size=0;
}

CNySample::~CNySample(void)
{
}
void CNySample::SetParam(void  __RPC_FAR *i_data,INT32 i_length)
{
	this->m_data     =i_data;
	this->m_data_size=i_length;
}
/*------------------------------------------------------------
IUnknown
------------------------------------------------------------*/

HRESULT STDMETHODCALLTYPE CNySample::QueryInterface(REFIID iid, void** ppvObj)
{
	if(iid==IID_IUnknown || iid==IID_INySample){
		*ppvObj=this;
		this->AddRef();
		return S_OK;
	}else{
		return E_NOINTERFACE;
	}
}
ULONG STDMETHODCALLTYPE CNySample::AddRef(void)
{
	this->m_RefCount++;
	return this->m_RefCount;
}

ULONG STDMETHODCALLTYPE CNySample::Release(void)
{
	OutputDebugString(_T("[TRACE]CNySample::Release-1\n"));
	if (this->m_RefCount > 0){
		OutputDebugString(_T("[TRACE]CNySample::Release-2\n"));
		return this->m_RefCount;
	}
	OutputDebugString(_T("[TRACE]CNySample::Release-3\n"));
	delete this;
	return 0;
}
/*


*/
void*  __RPC_FAR STDMETHODCALLTYPE CNySample::GetData(void)
{
	return this->m_data;
}

INT32 STDMETHODCALLTYPE CNySample::GetDataSize(void)
{
	return this->m_data_size;
}

HRESULT STDMETHODCALLTYPE CNySample::CopyToBuffer(void __RPC_FAR *o_data,INT32 i_start,INT32 i_length)
{
	if(i_start+i_length>this->m_data_size){
		return S_FALSE;
	}
	CopyMemory(o_data,((BYTE*)this->m_data)+i_start,i_length);
	return S_OK;
}
