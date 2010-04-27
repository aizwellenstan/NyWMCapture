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
#include "NyWMCaptureMain.h"
#include "NyPropertyBag.h"
#include "ENyWMCapture.h"
#include "CaptureFactory_WM5.h"
#include "streams.h"

CNyWMCaptureMain::CNyWMCaptureMain(void)
{
	this->m_RefCount  =0;
	this->m_ds_objects.builder  =NULL;
	this->m_ds_objects.capture  =NULL;
	this->m_ds_objects.graphi   =NULL;
	this->m_ds_objects.render   =NULL;
	this->m_ds_objects.mediactrl=NULL;
	this->m_is_initialized      =false;
	this->m_is_start            =false;

	this->m_callback    =NULL;
	this->m_width       =320;
	this->m_height      =240;
	this->m_subtype     =&MEDIASUBTYPE_RGB565;
	this->m_rate        =30;
}

CNyWMCaptureMain::~CNyWMCaptureMain(void)
{
	if(this->m_is_initialized){
		this->Finalize();//�`�F�b�N�͂��Ȃ�
	}
	this->SetCallBack(NULL);
}




/*------------------------------------------------------------
IUnknown
------------------------------------------------------------*/

HRESULT STDMETHODCALLTYPE CNyWMCaptureMain::QueryInterface(REFIID iid, void** ppvObj)
{
	//��������
	*ppvObj=this;
	this->AddRef();
	return S_OK;
}
ULONG STDMETHODCALLTYPE CNyWMCaptureMain::AddRef(void)
{
	this->m_RefCount++;
	return this->m_RefCount;
}

ULONG STDMETHODCALLTYPE CNyWMCaptureMain::Release(void)
{
	if (this->m_RefCount > 0){
		return this->m_RefCount;
	}
	delete this;
	return 0;
}

/*------------------------------------------------------------
INyWMCapture
------------------------------------------------------------*/
HRESULT STDMETHODCALLTYPE CNyWMCaptureMain::Start(void)
{
	//�X�e�[�^�X�`�F�b�N
	if(!this->m_is_initialized){
		return S_FALSE;
	}
	HRESULT hr;
	hr=this->m_ds_objects.mediactrl->Run();
//	if(SUCCEEDED(hr)){
		this->m_is_start=true;
//	}
	hr=S_OK;//�悭�킩��Ȃ�����FALSE�Ԃ��Ă��邯�ǂł��Ă�񂾂�ˁH�H�H
	return hr;
}

HRESULT STDMETHODCALLTYPE CNyWMCaptureMain::Stop(void)
{
	//�X�e�[�^�X�`�F�b�N
	if(!this->m_is_initialized){
		return S_FALSE;
	}
	HRESULT hr;
	hr=this->m_ds_objects.mediactrl->Stop();
//	if(SUCCEEDED(hr)){
		this->m_is_start=false;
//	}
	hr=S_OK;
	return hr;
}


HRESULT STDMETHODCALLTYPE CNyWMCaptureMain::Initialize(
	ULONG i_device_id,
	ULONG i_subtype_id,
	ULONG i_capture_category_id)
{
	//�X�e�[�^�X�`�F�b�N
	if(this->m_is_initialized){
		return S_FALSE;
	}


	IGraphBuilder *graphi          = NULL;
	ICaptureGraphBuilder2 *builder = NULL;
	IBaseFilter *capture=NULL;
	IBaseFilter *render =NULL;
	IMediaControl *mediactrl=NULL;
	ICaptureFactory* factory=NULL;
	// Create the Filter Graph Manager.
	HRESULT hr;

	try{
		//�t�B���^�O���t
		hr=CoCreateInstance(CLSID_FilterGraph, NULL,CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&graphi);
		if(!SUCCEEDED(hr))
		{
			throw ENyWMCapture();
		}

		//�L���v�`���O���t�r���_	
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder, NULL,CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&builder);
		if(!SUCCEEDED(hr))
		{
			throw ENyWMCapture();
		}
		//MediaControl
		hr=graphi->QueryInterface(IID_IMediaControl,(void **)&mediactrl);
		if(!SUCCEEDED(hr))
		{
			throw ENyWMCapture();
		}

		builder->SetFiltergraph(graphi);

		//�t�@�N�g���[�����
		factory=this->CreateCaptureFactory(i_device_id);

		//�J�����t�B���^�̐ݒ�
		capture=factory->CreateCaptureFilter(this->m_rate);
		//�����_���̐ݒ�
		render =factory->CreateRenderFilter(this->m_subtype,this->m_width,this->m_height,this->m_callback);
		delete factory;
		factory=NULL;
		
		//�O���t�N��
		hr=graphi->AddFilter(capture,L"Video capture source");
		if(!SUCCEEDED(hr)){
			throw ENyWMCapture();
		}
		hr=graphi->AddFilter(render,L"Video render");
		if(!SUCCEEDED(hr)){
			throw ENyWMCapture();
		}
		
		hr=builder->RenderStream(PinCategory2GUID(i_capture_category_id),&MEDIATYPE_Video,capture,NULL,render);
		if(!SUCCEEDED(hr)){
			throw ENyWMCapture();
		}
	}catch(...){
		if(mediactrl!=NULL){
			mediactrl->Release();
		}
		if(builder!=NULL){
			builder->Release();
		}
		if(graphi!=NULL){
			graphi->Release();
		}
		if(capture!=NULL){
			capture->Release();
		}
		if(render!=NULL){
			render->Release();
		}
		if(factory!=NULL){
			delete factory;
		}
		return S_FALSE;
	}
	//
	this->m_ds_objects.mediactrl=mediactrl;
	this->m_ds_objects.builder  =builder;
	this->m_ds_objects.capture  =capture;
	this->m_ds_objects.graphi   =graphi;
	this->m_ds_objects.render   =render;
	this->m_is_initialized      =true;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNyWMCaptureMain::Finalize(void)
{
	//�X�e�[�^�X�`�F�b�N
	if(!this->m_is_initialized){
		return S_FALSE;
	}
	//�Đ����Ȃ��~
	if(!this->m_is_start){
		this->Stop();//�G���[�`�F�b�N�͂��Ȃ�
	}

	this->m_ds_objects.mediactrl->Release();
	this->m_ds_objects.builder->Release();
	this->m_ds_objects.capture->Release();
	this->m_ds_objects.render->Release();
	this->m_ds_objects.graphi->Release();
	this->m_is_initialized=false;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNyWMCaptureMain::SetCallBack(INySampleCB __RPC_FAR *i_callback)
{

	if(this->m_is_initialized){
		return S_FALSE;
	}
	if(this->m_callback!=NULL){
		this->m_callback->Release();
		this->m_callback=NULL;
	}
	if(i_callback!=NULL){
		i_callback->AddRef();
		this->m_callback=i_callback;
	}
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CNyWMCaptureMain::SetSize(INT32 i_width,INT32 i_height)
{
	if(this->m_is_initialized){
		return S_FALSE;
	}
	this->m_width =i_width;
	this->m_height=i_height;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNyWMCaptureMain::SetSubType(INT32 i_subtype_id)
{
	if(this->m_is_initialized){
		return S_FALSE;
	}
	this->m_subtype=SubtypeId2GUID(i_subtype_id);
	return S_OK;
}

/*------------------------------------------------------------
	�t�B���^�쐬
------------------------------------------------------------*/
ICaptureFactory* CNyWMCaptureMain::CreateCaptureFactory(ULONG i_device_id)
{
	switch(i_device_id)
	{
	case NyWMCapture_DeviceId_WM5:
		return new CCaptureFactory_WM5();
		break;
	default:
		throw ENyWMCapture();
	}
}

const GUID* CNyWMCaptureMain::PinCategory2GUID(ULONG i_pin_category_id)
{
	switch(i_pin_category_id)
	{
	case NyWMCapture_PinCategory_CAPTURE:
		return &PIN_CATEGORY_CAPTURE;
	case NyWMCapture_PinCategory_PREVIEW:
		return &PIN_CATEGORY_PREVIEW;
	case NyWMCapture_PinCategory_STILL:
		return &PIN_CATEGORY_STILL;
	default:
		throw ENyWMCapture();
	}
}
const GUID* CNyWMCaptureMain::SubtypeId2GUID(ULONG i_subtype_id)
{
	switch(i_subtype_id)
	{
	case NyWMCapture_MediaSubType_RGB565:
		return &MEDIASUBTYPE_RGB565;
	default:
		throw ENyWMCapture();
	}
}
/*	�s���J�e�S��ID���s�����ɕϊ�
*/
const LPCTSTR CNyWMCaptureMain::PinCategory2Name(ULONG i_pin_category_id)
{
	switch(i_pin_category_id)
	{
	case NyWMCapture_PinCategory_CAPTURE:
		return _T("Capture");
	case NyWMCapture_PinCategory_PREVIEW:
		return _T("Preview");
	case NyWMCapture_PinCategory_STILL:
		return _T("Still");
	default:
		throw ENyWMCapture();
	}
}

void CNyWMCaptureMain::SetCapturePinProperty(IBaseFilter* i_cam,ULONG i_subtype_id,LONG i_width,LONG i_height)
{
	//�s��ID����s�������擾
	LPCTSTR pin_name=this->PinCategory2Name(i_subtype_id);
	// IEnumPins�C���^�[�t�F�C�X�擾
	IEnumPins*pEP =NULL;
	IPin* pPin    =NULL;
	IAMStreamConfig* pConfig = NULL;
	AM_MEDIA_TYPE *pmtConfig=NULL;
	try{

		i_cam->EnumPins(&pEP);
		while(pEP->Next(1,&pPin,NULL)==S_OK) {
			PIN_INFO pInfo;
			pPin->QueryPinInfo(&pInfo);
			if(lstrcmp(pInfo.achName,pin_name)!=0){
				pPin->Release();
				pPin=NULL;
			}
		}
		//PIN���������H
		if(pPin==NULL){
			throw ENyWMCapture();
		}

		// Preview�s���ł���Α����̏���
		int count, size;
		pPin->QueryInterface(IID_IAMStreamConfig, (void**)&pConfig);
		pConfig->GetNumberOfCapabilities(&count,&size);
		if(size!=sizeof(VIDEO_STREAM_CONFIG_CAPS)){
			throw ENyWMCapture();
		}
		//�w��T�C�Y�̃t�H�[�}�b�g������
		for(int i=0;i<count;i++){
			VIDEO_STREAM_CONFIG_CAPS scc;
			pConfig->GetStreamCaps(i, &pmtConfig, (BYTE*)&scc);
			if(pmtConfig->majortype==MEDIATYPE_Video && pmtConfig->formattype == FORMAT_VideoInfo){
				VIDEOINFOHEADER *vih=(VIDEOINFOHEADER *)pmtConfig->pbFormat;
				BITMAPINFOHEADER *bih=&vih->bmiHeader;
				BITMAPINFO *bi=(BITMAPINFO*)&vih->bmiHeader;
				if(bih->biHeight==i_height && bih->biWidth==i_width) {
					//�t�H�[�}�b�g����
					break;
				}
			}
			if(pmtConfig->cbFormat!=0) {
				CoTaskMemFree((PVOID)pmtConfig->pbFormat);
			}
			//pmtConfig->pUnk->Release();//���g�p�炵����
			pmtConfig=NULL;
		}
		//�t�H�[�}�b�g�݂������H
		if(pmtConfig==NULL){
			//NEEE
			throw ENyWMCapture();
		}
		pConfig->SetFormat(pmtConfig);
	}catch(...){
		//���s��
		if(pConfig!=NULL){
			//pmtConfig->pUnk->Release();//���g�p�炵����
			pConfig->Release();
		}
		if(pPin!=NULL){
			pPin->Release();
		}
		if(pEP!=NULL){
			pEP->Release();
		}
		throw;
	}
	//������
	if(pConfig!=NULL){
		//pmtConfig->pUnk->Release();//���g�p�炵����
		pConfig->Release();
	}
	if(pPin!=NULL){
		pPin->Release();
	}
	if(pEP!=NULL){
		pEP->Release();
	}
	return;
}