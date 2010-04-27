#include "StdAfx.h"
#include <streams.h>
#include <comutil.h>
#include "CaptureFactory_WM5.h"
#include "ENyWMCapture.h"
#include "NyPropertyBag.h"
#include "NyDumpRender.h"
#include "NyWMCaptureMain.h"


/*	WM5で標準と思われるキャプチャフィルタのCLSID

*/
const GUID CLSID_WM5_CAPTURE={0xf80b6e95,0xb55a,0x4619,0xae,0xc4,0xa1,0x0e,0xae,0xde,0x98,0x0c};
CCaptureFactory_WM5::CCaptureFactory_WM5(const GUID* i_clsid)
{
	this->m_capture_clsid=*i_clsid;
}
CCaptureFactory_WM5::CCaptureFactory_WM5()
{
	this->m_capture_clsid=CLSID_WM5_CAPTURE;
}

IBaseFilter* CCaptureFactory_WM5::CreateCaptureFilter(LONG i_rate)
{
	HRESULT hr;
	IBaseFilter* pVideoCap=NULL;
	IPersistPropertyBag* pPropertyBag=NULL;

	hr=CoCreateInstance(this->m_capture_clsid, NULL,CLSCTX_INPROC_SERVER, IID_IBaseFilter,(void **)&pVideoCap);
	if(!SUCCEEDED(hr)){
		throw ENyWMCapture();
	}
	try{
		hr=pVideoCap->QueryInterface(IID_IPersistPropertyBag,(void **)&pPropertyBag);
		if(!SUCCEEDED(hr)){
			throw ENyWMCapture();
		}

		_variant_t varCamName = L"CAM1:";
		if(( varCamName.vt == VT_BSTR ) == NULL ) {
			throw ENyWMCapture();
		}
		CNyPropertyBag PropBag;
		PropBag.Write( L"VCapName", &varCamName );   
		pPropertyBag->Load( &PropBag, NULL );
		pPropertyBag->Release();
	}catch(...){
		if(pPropertyBag!=NULL){
			pPropertyBag->Release();
		}
		if(pVideoCap!=NULL){
			pVideoCap->Release();
		}
		throw;
	}
	return pVideoCap;
}

IBaseFilter* CCaptureFactory_WM5::CreateRenderFilter(const GUID* i_subtype,LONG i_width,LONG i_height,INySampleCB* i_cb)
{
	HRESULT hr;
	CNyDumpRender* render=new CNyDumpRender(NULL,&hr);
	if(!SUCCEEDED(hr)){
		throw ENyWMCapture();
	}
	render->SetSize(i_width,i_height);
	render->SetSubType(i_subtype);
	render->SetCallBack(i_cb);
	return render;
}



