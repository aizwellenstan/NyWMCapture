// DLLTest.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
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
	�T���v�����󂯂�R�[���o�b�N�I�u�W�F�N�g


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
	HRESULT STDMETHODCALLTYPE OnSample(
		/* [in] */ INySample  __RPC_FAR *i_sample)
	{
		/*	Window�Ƃ̕`�擮�@���킩��Ȃ�����ASendMessage�œ���
			SendMessage�Ń��C���X���b�h�Ɠ��������Ƃ��́A���C���X���b�h�Ɠ��������̂ŁA
			���ɏI�����Ƀf�b�g���b�N�������Ȃ��悤�ɒ��ӂ��邱�ƁI
		*/
		int len=i_sample->GetDataSize();
		void* data=i_sample->GetData();
		if(!this->m_is_terminated){
			SendMessage(this->m_target,WM_USER+100,0,(LPARAM)data);
		}
		return S_OK;
	}
};









// �O���[�o���ϐ�:
INyWMCapture*       g_capture;
HINSTANCE			g_hInst;			// ���݂̃C���^�[�t�F�C�X
HWND				g_hWndMenuBar;		// ���j���[ �o�[ �n���h��
CNySampleCB*        g_cb;

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂�:
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

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DLLTEST));

	// ���C�� ���b�Z�[�W ���[�v:
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
//  �֐� : MyRegisterClass()
//
//  �ړI : �E�B���h�E �N���X��o�^���܂��B
//
//  �R�����g:
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
//   �֐�: InitInstance(HINSTANCE, int)
//
//   �ړI : �C���X�^���X �n���h����ۑ����āA���C�� �E�B���h�E���쐬���܂��B
//
//   �R�����g:
//
//        ���̊֐��ŁA�O���[�o���ϐ��ŃC���X�^���X �n���h����ۑ����A
//        ���C�� �v���O���� �E�B���h�E���쐬����ѕ\�����܂��B
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// �^�C�g�� �o�[ �e�L�X�g
    TCHAR szWindowClass[MAX_LOADSTRING];	// ���C�� �E�B���h�E �N���X��

    g_hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

    // CAPEDIT ����� SIPPREF �̂悤�ȃf�o�C�X�ŗL�̃R���g���[��������������ɂ́A�A�v���P�[�V������
    // ���������� SHInitExtraControls ����x�Ăяo���K�v������܂��B
    SHInitExtraControls();

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_DLLTEST, szWindowClass, MAX_LOADSTRING);

    //���Ɏ��s���Ă���ꍇ�́A�E�B���h�E�Ƀt�H�[�J�X��^���A�I�����܂��B
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // �ŉ��ʂ̎q�E�B���h�E�Ƀt�H�[�J�X��ݒ肵�܂��B
        // ���L���邷�ׂẴE�B���h�E��O�ɔz�u���āA�A�N�e�B�u�ɂ��邽�߂� "| 0x00000001"
        // ���g�p����܂����B
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

    // CW_USEDEFAULT ���g�p���ă��C�� �E�B���h�E���쐬����ꍇ�Amenubar �̍����͍l����
    // ������܂���Bmenubar �����݂���ꍇ�́A�쐬������ŁA�E�B���h�E�̃T�C�Y��
    // �w�肵�����܂��B
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
//  �֐�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  �ړI :  ���C�� �E�B���h�E�̃��b�Z�[�W���������܂��B
//
//  WM_COMMAND	- �A�v���P�[�V���� ���j���[�̏���
//  WM_PAINT	- ���C�� �E�B���h�E�̕`��
//  WM_DESTROY	- ���~���b�Z�[�W��\�����Ė߂�
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
            // �I�����ꂽ���j���[�̉��:
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

            // shell �A�N�e�B�x�[�g���̃X�g���N�`�������������܂��B
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: �`��R�[�h�������ɒǉ����Ă�������...
            
            EndPaint(hWnd, &ps);
            break;
		case WM_CLOSE:
			/*�R�[���o�b�N�֐��̃��b�Z�[�W�n�o���~�߂�*/
			g_cb->Terminate();
			/*�����҂��̕`�惁�b�Z�[�W���폜*/
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
            // ���b�Z�[�W�̃A�N�e�B�x�[�g�� shell �ɒʒm���܂��B
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

// �o�[�W�������{�b�N�X�̃��b�Z�[�W �n���h���ł��B
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            {
                // [�I��] �{�^�����쐬���A�T�C�Y���w�肵�܂��B  
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
