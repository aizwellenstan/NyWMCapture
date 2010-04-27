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
// NyWMCapture.cpp : DLL の初期化ルーチンです。
//

#include "stdafx.h"
#include "NyWMCapture.h"
#include "NyWMCaptureMain.h"
#include "NyComRegister.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const IID IID_INyDWMCapture=
	{0xf633a3d8, 0xd61f, 0x4059, 0xbc, 0x45, 0xab, 0xa5, 0x2b, 0x97, 0x5c, 0x13};


/*
	COMのメイン
*/
class CNyWMCaptureFactory :public IClassFactory
{
private:
	ULONG m_RefCount;
	ULONG m_LockCount;
public:
	CNyWMCaptureFactory(void)
	{
		this->m_RefCount=0;
		this->m_LockCount=0;
	}
	~CNyWMCaptureFactory(void)
	{
	}
	BOOL IsLock(void)
	{
		return this->m_LockCount>0?TRUE:FALSE;
	}
public:
	//IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObj)
	{
		//いい加減
		*ppvObj=this;
		this->AddRef();
		return S_OK;
	}
	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		this->m_RefCount++;
		return this->m_RefCount;
	}
	ULONG STDMETHODCALLTYPE Release(void)
	{
		if (this->m_RefCount > 0){
			return this->m_RefCount;
		}
		delete this;
		return 0;
	}
public:
	//IClassFactory
	HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown __RPC_FAR *pUnkOuter,REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if(riid==IID_IUnknown || riid==IID_INyDWMCapture){
			*ppvObject=new CNyWMCaptureMain();
			((IUnknown*)*ppvObject)->AddRef();
			return S_OK;
		}else{
			return E_NOINTERFACE;
		}
	}	
	HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock)
	{
		if(fLock){
			this->m_LockCount++;
		}else{
			this->m_LockCount--;
		}
		/*アンダーフローチェックいれる？*/
		return S_OK;
	}
};



//ClassFactoryは1インスタンスでやりくり…。
static CNyWMCaptureFactory main_factory;

// DllGetClassObject - クラス ファクトリを返します。

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	if(rclsid==CLSID_NyDWMCapture){
		if(riid==IID_IUnknown || riid==IID_IClassFactory){
			*ppv=&main_factory;
			((IUnknown*)*ppv)->AddRef();
		}else{
			return E_NOINTERFACE;
		}
	}else{
		return CLASS_E_CLASSNOTAVAILABLE;
	}
	return S_OK;
}


// DllCanUnloadNow - COM が DLL をアンロードできるようにします。

STDAPI DllCanUnloadNow(void)
{
	return main_factory.IsLock()?S_FALSE:S_OK;
}


#define DLL_NAME L"NyWMCapture.dll"
#define DLL_DESCRIPTION L"NyWMCapture"
// DllRegisterServer - エントリをシステム レジストリに追加します。

STDAPI DllRegisterServer(void)
{	
	/*	動的リンクしたDLLのファイルパスを計算する方法がわからんな！
		\Windowsにコピーして使ってくれたまえ！
	*/
//	TCHAR buf[MAX_PATH];
//	DWORD dw;
//	dw=GetModuleFileName(app.m_hInstance,buf,MAX_PATH);
//	if(dw<1){
//		return S_FALSE;
//	}
	CNyComRegister reg;
	return reg.RegisterCLSID(CLSID_NyDWMCapture,DLL_DESCRIPTION,DLL_NAME)?S_OK:S_FALSE;
}


// DllUnregisterServer - エントリをレジストリから削除します。

STDAPI DllUnregisterServer(void)
{
	CNyComRegister reg;
	return reg.UnRegisterCLSID(CLSID_NyDWMCapture)?S_OK:S_FALSE;
}

/*
	DLLのエントリポイント
*/


extern "C" BOOL WINAPI DllEntry(
    HANDLE hinstDLL,
    DWORD dwReason,
    LPVOID lpvReserved);

BOOL WINAPI DllEntry(    HANDLE hinstDLL,
    DWORD dwReason,
    LPVOID lpvReserved)
{
    return TRUE;
}
