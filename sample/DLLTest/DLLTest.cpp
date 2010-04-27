// DLLTest.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "afxwin.h"
#include "DLLTest.h"
#include "NyWMCaptureInterface.h"


#include <winx.h>

#define MAX_LOADSTRING 100


#define Ny_WM_USER WM_USER+100

const GUID CLSID_NyDWMCapture=
	{0x32f37e70, 0xb633, 0x4253, {0xb8, 0xe0, 0xa9, 0x9a, 0x1b, 0xbe, 0xea, 0x84}};
const IID IID_INyDWMCapture=
	{0xf633a3d8, 0xd61f, 0x4059, {0xbc, 0x45, 0xab, 0xa5, 0x2b, 0x97, 0x5c, 0x13}};

/*********************************************************************************
	サンプルを受けるコールバックオブジェクト


*********************************************************************************/
class CNySampleCB : public INySampleCB
{
private:
	bool  m_is_terminated;
	ULONG m_RefCount;
	HWND  m_target;
/*------------------------------------------------------------
IUnknown
------------------------------------------------------------*/
public:
	CNySampleCB(HWND i_target_hwnd)
	{
		this->m_RefCount=0;
		this->m_target  =i_target_hwnd;
		this->m_is_terminated=false;
	}
	void Terminate()
	{
		this->m_is_terminated=true;
	}
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
	HRESULT STDMETHODCALLTYPE OnSample(
		/* [in] */ INySample  __RPC_FAR *i_sample)
	{
		/*	Windowとの描画動機がわからないから、SendMessageで同期
			SendMessageでメインスレッドと同期を取るときは、メインスレッドと同期を取るので、
			特に終了時にデットロックおこさないように注意すること！
		*/
		int len=i_sample->GetDataSize();
		void* data=i_sample->GetData();
		if(!this->m_is_terminated){
			SendMessage(this->m_target,WM_USER+100,0,(LPARAM)data);
		}
		return S_OK;
	}
};









// グローバル変数:
INyWMCapture*       g_capture;
HINSTANCE			g_hInst;			// 現在のインターフェイス
HWND				g_hWndMenuBar;		// メニュー バー ハンドル
CNySampleCB*        g_cb;

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);





void StartCapture(HWND i_windwos)
{
	HRESULT hr;
	CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_NyDWMCapture
	   , 0
	   , CLSCTX_INPROC_SERVER
	   , IID_INyDWMCapture
	   , reinterpret_cast<void**>(&g_capture) );
	g_cb=new CNySampleCB(i_windwos);
	hr=g_capture->SetCallBack(g_cb);
	hr=g_capture->SetSize(240,320);
	hr=g_capture->Initialize(NyWMCapture_DeviceId_WM5,NyWMCapture_MediaSubType_RGB565,NyWMCapture_PinCategory_PREVIEW);
	hr=g_capture->Start();
}

void StopCapture()
{
	HRESULT hr;
	hr=g_capture->Stop();
	hr=g_capture->Finalize();
	g_capture->Release();
	delete g_cb;
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;

	// アプリケーションの初期化を実行します:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DLLTEST));

	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  関数 : MyRegisterClass()
//
//  目的 : ウィンドウ クラスを登録します。
//
//  コメント:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DLLTEST));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的 : インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// タイトル バー テキスト
    TCHAR szWindowClass[MAX_LOADSTRING];	// メイン ウィンドウ クラス名

    g_hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

    // CAPEDIT および SIPPREF のようなデバイス固有のコントロールを初期化するには、アプリケーションの
    // 初期化中に SHInitExtraControls を一度呼び出す必要があります。
    SHInitExtraControls();

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_DLLTEST, szWindowClass, MAX_LOADSTRING);

    //既に実行している場合は、ウィンドウにフォーカスを与え、終了します。
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // 最下位の子ウィンドウにフォーカスを設定します。
        // 所有するすべてのウィンドウを前に配置して、アクティブにするために "| 0x00000001"
        // が使用されました。
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 

    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    // CW_USEDEFAULT を使用してメイン ウィンドウを作成する場合、menubar の高さは考慮に
    // 入れられません。menubar が存在する場合は、作成した後で、ウィンドウのサイズを
    // 指定し直します。
    if (g_hWndMenuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hWndMenuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }



    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
	StartCapture(hWnd);	

    return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的 :  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
//
//


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    static SHACTIVATEINFO s_sai;
	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // 選択されたメニューの解析:
            switch (wmId)
            {
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
                case IDM_OK:
                    SendMessage (hWnd, WM_CLOSE, 0, 0);				
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
            SHMENUBARINFO mbi;

            memset(&mbi, 0, sizeof(SHMENUBARINFO));
            mbi.cbSize     = sizeof(SHMENUBARINFO);
            mbi.hwndParent = hWnd;
            mbi.nToolBarId = IDR_MENU;
            mbi.hInstRes   = g_hInst;

            if (!SHCreateMenuBar(&mbi)) 
            {
                g_hWndMenuBar = NULL;
            }
            else
            {
                g_hWndMenuBar = mbi.hwndMB;
            }

            // shell アクティベート情報のストラクチャを初期化します。
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: 描画コードをここに追加してください...
            
            EndPaint(hWnd, &ps);
            break;
		case WM_CLOSE:
			/*コールバック関数のメッセージ創出を止める*/
			g_cb->Terminate();
			/*処理待ちの描画メッセージを削除*/
			MSG msg;
			while(PeekMessage(&msg,hWnd,Ny_WM_USER,Ny_WM_USER,PM_REMOVE)){
				OutputDebugString(L"DELETE");
			}
			DestroyWindow(hWnd);
			break;
        case WM_DESTROY:

			StopCapture();
            CommandBar_Destroy(g_hWndMenuBar);
            PostQuitMessage(0);
            break;

        case WM_ACTIVATE:
            // メッセージのアクティベートを shell に通知します。
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;
		case Ny_WM_USER:
			{
				CBitmap bmp;
				bmp.CreateBitmap(240,320,1,16,(void*)lParam);

				CWnd win;
				win.Attach(hWnd);
				CWindowDC wdc(&win);

				CDC mdc;
				mdc.CreateCompatibleDC(&wdc);

				CBitmap *tmp_object;
				tmp_object = mdc.GetCurrentBitmap();
				mdc.SelectObject(&bmp);
				wdc.BitBlt(0,0,240,320,&mdc,0,0,SRCCOPY);

				mdc.SelectObject(tmp_object);
				win.Detach();
			}
			break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            {
                // [終了] ボタンを作成し、サイズを指定します。  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;

    }
    return (INT_PTR)FALSE;
}
