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
#include "NyComRegister.h"


#define MAX_KEY_LEN  260
#define CHARS_IN_GUID 39


bool CNyComRegister::RegDeleteKeyNT(HKEY i_hkey, LPTSTR strSubKey )
{
	DWORD   dwRtn, dwSubKeyLength;
	LPTSTR  pSubKey = NULL;
	TCHAR   szSubKey[MAX_KEY_LEN]; // (256) this should be dynamic.
	HKEY    hKey;
	if((dwRtn=RegOpenKeyEx(i_hkey,strSubKey,0,  KEY_ALL_ACCESS | DELETE, &hKey )) == ERROR_SUCCESS)
	{
		while (dwRtn == ERROR_SUCCESS ){
			dwSubKeyLength = MAX_KEY_LEN;
			dwRtn=RegEnumKeyEx(hKey,0,szSubKey,&dwSubKeyLength,NULL,NULL,NULL,NULL);

			if(dwRtn == ERROR_NO_MORE_ITEMS){
				break;
			}else if(dwRtn == ERROR_SUCCESS){
				dwRtn=RegDeleteKeyNT(hKey, szSubKey);
			}
		}
		RegCloseKey(hKey);
		dwRtn = RegDeleteKey(i_hkey, strSubKey);
	}
	return dwRtn==ERROR_SUCCESS?true:false;
}


bool CNyComRegister::RegisterCLSID(
	const CLSID i_clsid,
	LPCWSTR i_description,
	LPCWSTR i_filename,
	LPCWSTR i_thread_model,
	LPCWSTR i_server_type)

{
	TCHAR achTemp[MAX_PATH];
	OLECHAR szCLSID[CHARS_IN_GUID];
	HRESULT hr = StringFromGUID2( i_clsid, szCLSID,CHARS_IN_GUID);
	if(!SUCCEEDED(hr)){
		return false;
	}

	HKEY hkey;
	LONG lreturn;
	DWORD dwDisposition;

	//キー作成
	wsprintf(achTemp, TEXT("CLSID\\%ls"), szCLSID );
	lreturn = RegCreateKeyEx(HKEY_CLASSES_ROOT, (LPCWSTR)achTemp, 0,TEXT(""), 0, 0, 0, &hkey, &dwDisposition);
	if( ERROR_SUCCESS != lreturn )
	{
		return false;
	}

	//キーの詳細
	wsprintf( achTemp, TEXT("%ls"), i_description );
	lreturn = RegSetValueEx(hkey, TEXT(""), 0L, REG_SZ, (CONST BYTE *)achTemp,sizeof(TCHAR) * (lstrlen(achTemp)+1) );
	if( ERROR_SUCCESS != lreturn )
	{
		RegCloseKey( hkey );
		return false;
	}

	//ServerTypeの書き込み
	HKEY hsubkey;
	wsprintf( achTemp, TEXT("%ls"), i_server_type );
	lreturn = RegCreateKeyEx(hkey, (LPCWSTR)achTemp, 0, TEXT(""), 0, 0, 0,&hsubkey, &dwDisposition);
	if( ERROR_SUCCESS != lreturn )
	{
		RegCloseKey( hkey );
		return false;
	}
	//ServerにThreadingModelとファイル名を書き込み

	wsprintf( achTemp, TEXT("%ls"), i_filename );
	lreturn = RegSetValueEx(hsubkey, TEXT(""), 0L, REG_SZ,(CONST BYTE *)achTemp,sizeof(TCHAR) * (lstrlen(achTemp)+1) );
	if( ERROR_SUCCESS != lreturn )
	{
		RegCloseKey( hkey );
		RegCloseKey( hsubkey );
		return false;
	}

	wsprintf( achTemp, TEXT("%ls"), i_thread_model );
	lreturn = RegSetValueEx( hsubkey, TEXT("ThreadingModel"), 0L, REG_SZ, (CONST BYTE *)achTemp, sizeof(TCHAR) * (lstrlen(achTemp)+1) );

	//終了
	RegCloseKey( hkey );
	RegCloseKey( hsubkey );

	return true;
}
bool CNyComRegister::UnRegisterCLSID(const CLSID  i_clsid)
{
	OLECHAR szCLSID[CHARS_IN_GUID];
	HRESULT hr = StringFromGUID2( i_clsid, szCLSID, CHARS_IN_GUID );
	if(!SUCCEEDED(hr)){
		return false;
	}

	TCHAR achBuffer[MAX_KEY_LEN];
	wsprintf( achBuffer, TEXT("CLSID\\%ls"), szCLSID );

	//サブキー配下を削除

	return RegDeleteKeyNT( HKEY_CLASSES_ROOT, achBuffer );
}



