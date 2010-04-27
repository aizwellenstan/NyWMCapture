/*	キャプチャファクトリ
*/
#pragma once
#include <comdef.h>

#include "ICaptureFactory.h"

class CCaptureFactory_WM5:public ICaptureFactory
{
private:
	GUID m_capture_clsid;
public:
	CCaptureFactory_WM5(const GUID* i_clsid);
	CCaptureFactory_WM5();
public:
	IBaseFilter* CreateCaptureFilter(LONG i_rate);
	IBaseFilter* CreateRenderFilter(const GUID* i_subtype_id,LONG i_width,LONG i_height,INySampleCB* i_cb);
};