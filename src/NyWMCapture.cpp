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
// NyWMCapture.cpp : DLL �̏��������[�`���ł��B
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
	COM�̃��C��
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
		//��������
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
		/*�A���_�[�t���[�`�F�b�N�����H*/
		return S_OK;
	}
};



//ClassFactory��1�C���X�^���X�ł�肭��c�B
static CNyWMCaptureFactory main_factory;

// DllGetClassObject - �N���X �t�@�N�g����Ԃ��܂��B

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


// DllCanUnloadNow - COM �� DLL ���A�����[�h�ł���悤�ɂ��܂��B

STDAPI DllCanUnloadNow(void)
{
	return main_factory.IsLock()?S_FALSE:S_OK;
}


#define DLL_NAME L"NyWMCapture.dll"
#define DLL_DESCRIPTION L"NyWMCapture"
// DllRegisterServer - �G���g�����V�X�e�� ���W�X�g���ɒǉ����܂��B

STDAPI DllRegisterServer(void)
{	
	/*	���I�����N����DLL�̃t�@�C���p�X���v�Z������@���킩���ȁI
		\Windows�ɃR�s�[���Ďg���Ă��ꂽ�܂��I
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


// DllUnregisterServer - �G���g�������W�X�g������폜���܂��B

STDAPI DllUnregisterServer(void)
{
	CNyComRegister reg;
	return reg.UnRegisterCLSID(CLSID_NyDWMCapture)?S_OK:S_FALSE;
}

/*
	DLL�̃G���g���|�C���g
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
