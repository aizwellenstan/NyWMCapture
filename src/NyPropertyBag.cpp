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
#include "NyPropertyBag.h"
using namespace std;






CNyPropertyBag::CNyPropertyBag(void)
{
	this->AddRef();
}
/*------------------------------------------------------------
IUnknown
------------------------------------------------------------*/

HRESULT STDMETHODCALLTYPE CNyPropertyBag::QueryInterface(REFIID iid, void** ppvObj)
{
	*ppvObj=this;
	return S_OK;
}
ULONG STDMETHODCALLTYPE CNyPropertyBag::AddRef(void)
{
	this->m_RefCount++;
	return this->m_RefCount;
}

ULONG STDMETHODCALLTYPE CNyPropertyBag::Release(void)
{
	if (this->m_RefCount > 0){
		return this->m_RefCount;
	}
	delete this;
	return 0;
}
/*------------------------------------------------------------
IPropertyBag	
------------------------------------------------------------*/
HRESULT STDMETHODCALLTYPE CNyPropertyBag::Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog)
{
    _bstr_t arg(pszPropName);
    if(m_Props.find(arg)==m_Props.end()){
        return E_INVALIDARG;
    }
    return VariantCopy(pVar, &m_Props[arg]);
}
HRESULT STDMETHODCALLTYPE CNyPropertyBag::Write(LPCOLESTR pszPropName, VARIANT *pVar)
{
    m_Props.insert(std::map<_bstr_t, _variant_t>::value_type(_bstr_t(pszPropName), _variant_t(*pVar, true)));
    return S_OK;
}

