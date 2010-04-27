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
#include "StdAfx.h"
#include <windows.h>
#include <commdlg.h>
#include <initguid.h>
#include "NySample.h"
#include "NyDumpRender.h"
#include "ENyWMCapture.h"
#include "NyWMCaptureInterface.h"
/*

	FilterCore

*/
DEFINE_GUID(IID_INyDump,0xF383EA14,0xEA7F,0x472c,0x86,0xD1,0x16,0xA9,0xD3,0x33,0x02,0x3E);


/**
�I�u�W�F�N�g���\�z���܂��B
*/
CNyDumpRender::CNyDumpRender(LPUNKNOWN pUnk, HRESULT *phr) :
    CBaseRenderer(IID_INyDump,NAME("CNyDump"),pUnk,phr)
	, m_buffer(NULL)
{
	this->m_video_width  =320;
	this->m_video_height =240;
	this->m_video_subtype=MEDIASUBTYPE_RGB24;
	this->m_callback     =NULL;
	this->m_sample=new CNySample();
	this->AddRef();
}

/**
�I�u�W�F�N�g����̂��܂��B
*/
CNyDumpRender::~CNyDumpRender()
{
	OutputDebugString(_T("[TRACE]CNyDumpRender::~CNyDumpRender-1\n"));
	if(this->m_callback!=NULL){
		this->m_callback->Release();
		this->m_callback=NULL;
	}
	OutputDebugString(_T("[TRACE]CNyDumpRender::~CNyDumpRender-2\n"));
	delete this->m_sample;
	OutputDebugString(_T("[TRACE]CNyDumpRender::~CNyDumpRender-3\n"));
}

/**
Basefilter�̃I�[�o���C�h
�T���v���������_�����O���邽�߂ɌĂяo����܂��B
���̃N���X�ł́A�T���v���̊l�����������܂��B
*/
HRESULT CNyDumpRender::DoRenderSample(IMediaSample *pMediaSample)
{
	static int c=0;
	long length;
	BYTE* data;
	length=pMediaSample->GetActualDataLength();
	pMediaSample->GetPointer(&data);
	
	this->m_sample->SetParam(data,length);

	/*	�t�B���^�̃R�[���o�b�N�֐����Ăяo��
	*/
	if(this->m_callback!=NULL)
	{
		this->m_callback->OnSample(this->m_sample);
	}
	pMediaSample->Release();
	return S_OK;
}

/**
�w�肵�����f�B�A�^�C�v���󂯕t���邩�ǂ����𔻒f���܂��B
���̊֐��́Am_video_width,m_video_height,m_video_subtype�Ɉ�v����Video��I�����܂��B
*/
HRESULT CNyDumpRender::CheckMediaType(const CMediaType *pMediaType)
{
	if(!IsEqualGUID(*pMediaType->FormatType(), FORMAT_VideoInfo)){
		return S_FALSE;
	}

	if(IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video)){
		if(IsEqualGUID(*pMediaType->Subtype(), this->m_video_subtype)){
			VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pMediaType->Format();
			if(pvi->bmiHeader.biWidth==this->m_video_width && pvi->bmiHeader.biHeight==this->m_video_height){
				/*pvi���O������ǂݏo���\�ɂ�����@�͌�ōl���悤�B*/
				return S_OK;
			}
		}
	}

	return S_FALSE;
}	
/**
�s���̃��f�B�A �^�C�v��ݒ肷��Ƃ��ɌĂяo����܂��B
*/
HRESULT CNyDumpRender::SetMediaType(const CMediaType *pmt)
{
	this->InitInfo(pmt);
	return S_OK;
}


void CNyDumpRender::InitInfo(const CMediaType *pmt)
{
	// �r�f�I �w�b�_�[�ւ̃|�C���^���l������B
	const VIDEOINFOHEADER *pVideoHeader = (const VIDEOINFOHEADER *)pmt->pbFormat;
	BITMAPINFO bitmapInfo;
	// �r�f�I �w�b�_�[�ɂ́A�r�b�g�}�b�v��񂪊܂܂��B
	// �r�b�g�}�b�v���� BITMAPINFO �\���̂ɃR�s�[����B
	ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
	CopyMemory(&bitmapInfo.bmiHeader
		, &(pVideoHeader->bmiHeader)
		, sizeof(BITMAPINFOHEADER));
}


//
// NonDelegatingQueryInterface
//
// Override this to say what interfaces we support where
//

STDMETHODIMP CNyDumpRender::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
	HRESULT hr;
//    CAutoLock lock(&m_Lock);

    // Do we have this interface
	hr=CBaseRenderer::NonDelegatingQueryInterface(riid, ppv);
	return hr;

} // NonDelegatingQueryInterface

/********************************************************************************

�Ǝ��g��

*********************************************************************************/

void CNyDumpRender::SetSize(INT32 i_width,INT32 i_height)
{
	//���쒆�Ɏw�肵�Ȃ����ƁI
	this->m_video_height=i_height;
	this->m_video_width =i_width;
	return;
}

void CNyDumpRender::SetSubType(const GUID __RPC_FAR *i_subtype)
{
	//���쒆�Ɏw�肵�Ȃ����ƁI
	this->m_video_subtype=*i_subtype;
	return;
}

void CNyDumpRender::SetCallBack(INySampleCB __RPC_FAR *i_callback)
{
	//���쒆�Ɏw�肵�Ȃ����ƁI

	//�Â��I�u�W�F�N�g��Release
	if(this->m_callback!=NULL){
		this->m_callback->Release();
		this->m_callback=NULL;
	}
	if(i_callback!=NULL){
		i_callback->AddRef();
		this->m_callback=i_callback;

	}
}
