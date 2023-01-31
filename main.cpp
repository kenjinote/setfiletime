#pragma comment(lib,"comctl32.lib")
#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#define IDU_BUTTON1 100

CHAR szClassName[]="window";
const static char *name[] = {"作成日時:", "更新日時:", "アクセス日時:"};

void Get_balance(SYSTEMTIME *temp)//うるう年を解析して９時間のズレを修正する
{
	int cDay = 0;
	
	//日数を借りてくる前月の日数が何日かを求める
	if(temp->wMonth - 1 == 4|| temp->wMonth - 1 == 6 || temp->wMonth - 1 == 9 || temp->wMonth - 1 == 11)
	{
		cDay = 30;
	}
	else if(temp->wMonth - 1 == 2)
	{
		if((!(temp->wYear % 4) && temp->wYear % 100) || !(temp->wYear % 400))
		{
			cDay = 29;
		}
		else
		{
			cDay = 28;
		}
	}
	else
	{
		cDay = 31;
	}
	
	if((temp->wHour - 9) < 0)
	{
		if((temp->wDay - 1) < 1)
		{
			if((temp->wMonth - 1) < 1)
			{
				temp->wYear--;
				temp->wMonth = 12;
				temp->wDay = cDay;
				temp->wHour = (temp->wHour + 24) - 9;
			}
			else
			{
				temp->wMonth--;
				temp->wDay = (temp->wDay + cDay) - 1;
				temp->wHour = (temp->wHour + 24) - 9;
			}
		}
		else
		{
			temp->wDay--;
			temp->wHour = (temp->wHour + 24) - 9;
		}
	}
	else
	{
		temp->wHour = temp->wHour - 9;
	}
}


VOID RunSetFileTime(LPCSTR szFilePath,  CONST FILETIME *lpCreationTime,CONST FILETIME *lpLastWriteTime,CONST FILETIME *lpLastAccessTime)
{
	HANDLE hFile;
	hFile=CreateFile(szFilePath,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE){
		return;
	}
	SetFileTime(hFile,lpCreationTime,lpLastAccessTime,lpLastWriteTime);
	CloseHandle(hFile);
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HWND hDateTime[3],hStatic[3];
	HDROP hDrop; 
	CHAR szFileName[_MAX_PATH]; 
	UINT iFile,nFiles;
	switch(msg){
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDU_BUTTON1:
			SendMessage(hWnd,WM_CLOSE,0,0);
			break;
		}
		break;
		case WM_DROPFILES:
			hDrop = (HDROP)wParam; 
			nFiles = ::DragQueryFile((HDROP)hDrop, 0xFFFFFFFF, NULL, 0);
			{int i;for(i=0;i<3;i++){EnableWindow(hDateTime[i],FALSE);EnableWindow(hStatic[i],FALSE);}}
			for (iFile=0; iFile<nFiles; ++iFile) {
				int i;
				SYSTEMTIME g_stmGetDateTime;
				FILETIME Time[3];
				::DragQueryFile(hDrop, iFile, szFileName, sizeof(szFileName));
				for(i=0;i<3;i++){
					memset(&g_stmGetDateTime, 0, sizeof(SYSTEMTIME));
					memset(&Time[i], 0, sizeof(FILETIME));
					DateTime_GetSystemtime(hDateTime[i], &g_stmGetDateTime);
					Get_balance(&g_stmGetDateTime);//うるう年を解析して９時間のズレを修正する
					SystemTimeToFileTime(&g_stmGetDateTime,&Time[i]);
				}
				RunSetFileTime(szFileName,&Time[0],&Time[1],&Time[2]);
			}
			DragFinish(hDrop);
			{int i;for(i=0;i<3;i++){EnableWindow(hDateTime[i],TRUE);EnableWindow(hStatic[i],TRUE);}}
			break; 
			
		case WM_CREATE:
			{
				INITCOMMONCONTROLSEX iccex;
				iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);
				iccex.dwICC = ICC_DATE_CLASSES;
				InitCommonControlsEx(&iccex);
				int i;
				for(i=0;i<3;i++){
					hStatic[i] = CreateWindow("STATIC",name[i],WS_CHILD|WS_VISIBLE,10,10 + 30*i,120,28,hWnd,NULL,((LPCREATESTRUCT)lParam)->hInstance,NULL);
					hDateTime[i] = CreateWindowEx (WS_EX_CLIENTEDGE, DATETIMEPICK_CLASS ,  NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | DTS_TIMEFORMAT, 130, 10 + 30*i, 256, 25, hWnd, NULL, NULL, NULL);
					SendMessage(hDateTime[i], DTM_SETFORMAT, 0, (LPARAM)"yyy'年'MM'月'dd'日'tthh'時'mm'分'ss'秒'"); 
				}
				DragAcceptFiles(hWnd, TRUE);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return(DefWindowProc(hWnd,msg,wParam,lParam));
	}
	return (0L);
}

int WINAPI WinMain(HINSTANCE hinst,HINSTANCE hPreInst,
				   LPSTR pCmdLine,int nCmdShow)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS wndclass;
	if(!hPreInst){
		wndclass.style=CS_HREDRAW|CS_VREDRAW;
		wndclass.lpfnWndProc=WndProc;
		wndclass.cbClsExtra=0;
		wndclass.cbWndExtra=0;
		wndclass.hInstance =hinst;
		wndclass.hIcon=LoadIcon(hinst,MAKEINTRESOURCE(IDI_ICON1));
		wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
		wndclass.hbrBackground=(HBRUSH)(COLOR_3DFACE+1);
		wndclass.lpszMenuName=NULL;
		wndclass.lpszClassName=szClassName;
		if(!RegisterClass(&wndclass))
			return FALSE;
	}
	
	RECT rt;
	SetRect(&rt,0,0,396,135);
	AdjustWindowRect(&rt,WS_SYSMENU,0);
	
	hWnd=CreateWindow(szClassName,
		"タイムスタンプ変更ツール",
		WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
        rt.right-rt.left,
        rt.bottom-rt.top,
		NULL,
		NULL,
		hinst,
		NULL);
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (msg.wParam);
}


