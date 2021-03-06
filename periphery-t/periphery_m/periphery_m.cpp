// periphery_m.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <tlhelp32.h>
#include <commctrl.h>
#include <shellapi.h>
#include <string>
#include <fstream>
#include "periphery_m.h"
#include "initialize_ui.h"
#include "constant.h"
#include "set_autorun.h"


using namespace std;

#define MAX_LOADSTRING 100


//#pragma comment ( lib, "comctl32.lib")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	//设置库的搜索路径
	SetDllDirectory(L"lib");

	static WCHAR szAppName[] = TEXT("gpatm");

	HWND		hwnd;
	MSG			msg;
	WNDCLASS	wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_LEFT));
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = CreateSolidBrush(RGB(236, 233, 216));
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("窗口类注册失败!"), TEXT("错误"), MB_OK | MB_ICONERROR);
		return 0;
	}

	InitCommonControls();

	hwnd = CreateWindow(
		szAppName, TEXT(""),
		WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT,
		300, 250,
		NULL, NULL, hInstance, NULL
	);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;

}



//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hBtn[7];
	LOGFONT		lf;
	HDC			hdc;
	PAINTSTRUCT ps;
	WCHAR		btnLab[10];
	HFONT		hFont;

	wcscpy_s(lf.lfFaceName, TEXT("Arial"));
	lf.lfWidth = 6;
	lf.lfHeight = 12;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = GB2312_CHARSET;

	switch (message)
	{
	case WM_CREATE:						//创建创建时的相关初始化
		regVirtualMouseHotKey(hWnd);
		wcscpy_s(lf.lfFaceName, TEXT("Times New Roman"));
		lf.lfHeight = 16;
		lf.lfWidth = 8;
		lf.lfWeight = FW_NORMAL;
		hFont = CreateFontIndirect(&lf);
		InitWndUI(hWnd, ((LPCREATESTRUCT)lParam)->hInstance, hBtn, hFont, lf);
		if (checkAutorun())
			SendMessage(hBtn[3], BM_SETCHECK, 1, 0);
		return 0;

	case WM_COMMAND:					//处理按钮消息
		dealWithBtnMsg(hWnd, wParam, lParam, hBtn);	return 0;

	case WM_HOTKEY:						//处理热键消息
		dealWithHotKey(hWnd, wParam);			return 0;

	case WM_PAINT:						//处理重绘消息
		hdc = BeginPaint(hWnd, &ps);
		hFont = CreateFontIndirect(&lf);
		SelectObject(hdc, hFont);
		SetBkColor(hdc, RGB(236, 233, 216));
		drawTipText(hdc);
		DeleteObject(hFont);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_NOTIFY:						//处理超链接消息
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_CLICK:
		case NM_RETURN:
			//ShellExecute(NULL, TEXT("open"), TEXT("http://www.dtcoming.com/"), NULL, NULL, SW_SHOW);
			break;
		}
		return 0;

	case WM_DESTROY:					//退出程序
		GetWindowText(hBtn[0], btnLab, 10);
		if (wcscmp(btnLab, TEXT("停止")) == 0)			//当热键没有被注销时
		{
			destroyRegedHotKey(hWnd);					//注销热键

		}
		PostQuitMessage(0);		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}




//注册热键
void regVirtualMouseHotKey(HWND hwnd)
{

	RegisterHotKey(hwnd, ID_HOT_WND_HIDE, MOD_CONTROL, VK_F12);			//注册 Ctrl + f12 , 显示窗口
	RegisterHotKey(hwnd, ID_HOT_STOP, MOD_CONTROL, VK_F10);			//注册 Ctrl + f10 , 启动\停止热键
	RegisterHotKey(hwnd, ID_HOT_QUICK_STOP, MOD_CONTROL, VK_F11);			//注册 Ctrl + f11 , 快速启动\停止热键
	RegisterHotKey(hwnd, ID_HOT_PAUSE, MOD_CONTROL, VK_F9);			//注册 Ctrl + f9 , 运行\暂停热键
}

//////////////////////////////////////////////////////////////////////////

//撤销注册的热键
void destroyRegedHotKey(HWND hwnd)
{
	int hotID = ID_HOT_WND_HIDE;
	for (hotID; hotID <= ID_HOT_QUICK_STOP; hotID++)
		UnregisterHotKey(hwnd, hotID);
}

//////////////////////////////////////////////////////////////////////////

//处理热键消息
void dealWithHotKey(HWND hwnd, WPARAM wParam)
{

	switch (wParam)
	{

		case ID_HOT_WND_HIDE:		//呼出主界面
		{
			ShowWindow(hwnd, SW_SHOWNORMAL);
			SetForegroundWindow(hwnd);
			break;
		}
		case ID_HOT_STOP:			//停止\启动热键
		{
			SendMessage(hwnd, WM_COMMAND, ID_BTN_STOP | BN_CLICKED, 0);
			break;
		}
		case ID_HOT_PAUSE:			//暂停\运行热键
		{
			SendMessage(hwnd, WM_COMMAND, ID_BTN_PAUSE | BN_CLICKED, 0);
			break;
		}
		case ID_HOT_QUICK_STOP:			//停止\快速启动热键
		{
			SendMessage(hwnd, WM_COMMAND, ID_BTN_STOP | BN_CLICKED, 1);
			break;
		}
	}

}

//////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK CosonleHandler(DWORD ev)
{
	BOOL bRet = TRUE;
	switch (ev)
	{
	case CTRL_C_EVENT:
	{
		bRet = TRUE;
		break;
	}
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	{
		bRet = FALSE;
		break;
	}
	case CTRL_BREAK_EVENT:
	{
		bRet = TRUE;
		break;
	}
	default:
		break;
	}
	return bRet;
}
// Inspired from http://stackoverflow.com/a/15281070/1529139
// and http://stackoverflow.com/q/40059902/1529139
bool send_sigint(DWORD dwProcessId)
{
	bool success = false;
	DWORD thisConsoleId = GetCurrentProcessId();
	// Leave current console if it exists
	// (otherwise AttachConsole will return ERROR_ACCESS_DENIED)
	bool consoleDetached = (FreeConsole() != FALSE);
	if (AttachConsole(dwProcessId) != FALSE)
	{
		// Add a fake Ctrl-C handler for avoid instant kill is this console
		// WARNING: do not revert it or current program will be also killed
		SetConsoleCtrlHandler(nullptr, true);
		success = (GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0) != FALSE);
		SetConsoleCtrlHandler(nullptr, false);
		FreeConsole();
	}
#if 0
	if (consoleDetached)
	{
		// Create a new console if previous was deleted by OS
		if (AttachConsole(thisConsoleId) == FALSE)
		{
			int errorCode = GetLastError();
			if (errorCode == 31) // 31=ERROR_GEN_FAILURE
			{
				AllocConsole();
			}
		}
	}
#endif
	return success;
}
bool send_sigbreak(DWORD dwProcessId)
{
	bool success = false;
	DWORD thisConsoleId = GetCurrentProcessId();
	// Leave current console if it exists
	// (otherwise AttachConsole will return ERROR_ACCESS_DENIED)
	bool consoleDetached = (FreeConsole() != FALSE);
	if (AttachConsole(dwProcessId) != FALSE)
	{
		// Add a fake Ctrl-C handler for avoid instant kill is this console
		// WARNING: do not revert it or current program will be also killed
		SetConsoleCtrlHandler(CosonleHandler, true);
		success = (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0) != FALSE);
		//SetConsoleCtrlHandler(nullptr, false);
		FreeConsole();
	}
#if 0
	if (consoleDetached)
	{
		// Create a new console if previous was deleted by OS
		if (AttachConsole(thisConsoleId) == FALSE)
		{
			int errorCode = GetLastError();
			if (errorCode == 31) // 31=ERROR_GEN_FAILURE
			{
				AllocConsole();
			}
		}
	}
#endif
	return success;
}
#if 0
void send_sigint(DWORD p_pid)
{
	AttachConsole(p_pid);
	//SetConsoleCtrlHandler(NULL, TRUE);
	GenerateConsoleCtrlEvent(CTRL_C_EVENT, p_pid);
	FreeConsole();
}

void send_sigbreak(DWORD p_pid)
{
	AttachConsole(p_pid);
	//SetConsoleCtrlHandler(NULL, TRUE);
	GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, p_pid);
	FreeConsole();
}
#endif
bool PauseInterpreter(PROCESS_INFORMATION& pi, bool &runState)
{
	if (runState == true)
	{
		DWORD processId = pi.dwProcessId;
		send_sigbreak(processId);
	}
	return true;
}




bool StopInterpreter(PROCESS_INFORMATION& pi, bool &runState)
{
	if (runState == true)
	{
		runState = false;
		DWORD processId = pi.dwProcessId;
#if 0
		PROCESSENTRY32 processEntry = { 0 };
		processEntry.dwSize = sizeof(PROCESSENTRY32);
		//给系统内的所有进程拍一个快照
		HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		//遍历每个正在运行的进程
		if (Process32First(handleSnap, &processEntry)) {
			BOOL isContinue = TRUE;

			//终止子进程
			do {
				if (processEntry.th32ParentProcessID == processId) {
					HANDLE hChildProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processEntry.th32ProcessID);
					if (hChildProcess) {
						TerminateProcess(hChildProcess, 0);
						CloseHandle(hChildProcess);
					}
				}
				isContinue = Process32Next(handleSnap, &processEntry);
			} while (isContinue);

			HANDLE hBaseProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
			if (hBaseProcess) {
				TerminateProcess(hBaseProcess, 0);
				CloseHandle(hBaseProcess);
			}
		}
#endif
		send_sigint(processId);
		WaitForSingleObject(pi.hProcess, 5000);
		DWORD exitCode = 0;
		GetExitCodeProcess(pi.hProcess, &exitCode);
		if (exitCode == STILL_ACTIVE) {
			return false;
		}
		runState = false;
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);

		//****************************
		PROCESS_INFORMATION pii;
		TCHAR szCommandLine[512] = TEXT("gpatc.exe -run=false -enc -script=gp.script -sconfig=gps.conf -uconfig=gpu.conf -lic=gp.lic -role=1");//或者WCHAR
		//LPWSTR szCommandLine = TEXT("gpatc");//错误
		//STARTUPINFO si = { sizeof(si) };
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pii, sizeof(pii));

		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES),NULL,TRUE };
		HANDLE cmdOutput = CreateFile(TEXT("run_stop.log"),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			&sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (cmdOutput != INVALID_HANDLE_VALUE)
		{
			si.hStdOutput = cmdOutput;
			si.hStdError = cmdOutput;
		}



		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;  // 指定wShowWindow成员有效
		si.wShowWindow = SW_HIDE;          // 此成员设为TRUE的话则显示新建进程的主窗口，
							// 为FALSE的话则不显示
		BOOL bRet = ::CreateProcess(
			NULL,           // 不在此指定可执行文件的文件名
			szCommandLine,      // 命令行参数
			NULL,           // 默认进程安全性
			NULL,           // 默认线程安全性
			TRUE,          // 指定当前进程内的句柄不可以被子进程继承
			CREATE_NEW_CONSOLE, // 为新进程创建一个新的控制台窗口
			NULL,           // 使用本进程的环境变量
			NULL,           // 使用本进程的驱动器和目录
			&si,
			&pii);

		if (bRet)
		{
			//WaitForSingleObject(pi.hProcess, INFINITE);
			// 既然我们不使用两个句柄，最好是立刻将它们关闭
			//::CloseHandle(pi.hThread);
			//::CloseHandle(pi.hProcess);
			if (cmdOutput != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(cmdOutput);
			}
			//printf(" 新进程的进程ID号：%d \n", pi.dwProcessId);
			//printf(" 新进程的主线程ID号：%d \n", pi.dwThreadId);
		}



	}
	return true;
}

void fillFile(const string &fileName)
{
	short count;
	fstream file;
	file.open(fileName, std::fstream::in | std::fstream::out | std::fstream::binary);
	if (file.is_open())
	{
		file.seekg(-2, fstream::end);
		file.read((char*)&count, sizeof(count));
		count++;
		file.seekg(-2, fstream::end);
		file.write((char*)&count, sizeof(count));
		file.close();
	}
	return;

}

int RunInterpreter(PROCESS_INFORMATION& pi, bool &runState,PWSTR role)
{
	fillFile("gpatc.exe");

	TCHAR szCommandLine[512] = TEXT("gpatc.exe -run -wait -enc -script=gp.script -sconfig=gps.conf -uconfig=gpu.conf -lic=gp.lic -role=");//或者WCHAR
	wcscat_s(szCommandLine, role);
	//LPWSTR szCommandLine = TEXT("gpatc");//错误
	//STARTUPINFO si = { sizeof(si) };
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES),NULL,TRUE };
	HANDLE cmdOutput = CreateFile(TEXT("run.log"),
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		&sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (cmdOutput != INVALID_HANDLE_VALUE)
	{
		si.hStdOutput = cmdOutput;
		si.hStdError = cmdOutput;
	}



	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;  // 指定wShowWindow成员有效
	si.wShowWindow = SW_HIDE;          // 此成员设为TRUE的话则显示新建进程的主窗口，
						// 为FALSE的话则不显示
	BOOL bRet = ::CreateProcess(
		NULL,           // 不在此指定可执行文件的文件名
		szCommandLine,      // 命令行参数
		NULL,           // 默认进程安全性
		NULL,           // 默认线程安全性
		TRUE,          // 指定当前进程内的句柄不可以被子进程继承
		CREATE_NEW_CONSOLE, // 为新进程创建一个新的控制台窗口
		NULL,           // 使用本进程的环境变量
		NULL,           // 使用本进程的驱动器和目录
		&si,
		&pi);

	if (bRet)
	{
		//WaitForSingleObject(pi.hProcess, INFINITE);
		// 既然我们不使用两个句柄，最好是立刻将它们关闭
		//::CloseHandle(pi.hThread);
		//::CloseHandle(pi.hProcess);
		if (cmdOutput != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(cmdOutput);
		}
		//printf(" 新进程的进程ID号：%d \n", pi.dwProcessId);
		//printf(" 新进程的主线程ID号：%d \n", pi.dwThreadId);
		runState = true;
	}
	return 0;
}
int QuickRunInterpreter(PROCESS_INFORMATION& pi, bool &runState, PWSTR role)
{
	fillFile("gpatc.exe");

	TCHAR szCommandLine[512] = TEXT("gpatc.exe -run -wait -enc -direct -step=6 -script=gp.script -sconfig=gps.conf -uconfig=gpu.conf -lic=gp.lic -role=");//或者WCHAR
	wcscat_s(szCommandLine, role);
	//LPWSTR szCommandLine = TEXT("gpatc");//错误
	//STARTUPINFO si = { sizeof(si) };
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES),NULL,TRUE };
	HANDLE cmdOutput = CreateFile(TEXT("run.log"),
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		&sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (cmdOutput != INVALID_HANDLE_VALUE)
	{
		si.hStdOutput = cmdOutput;
		si.hStdError = cmdOutput;
	}



	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;  // 指定wShowWindow成员有效
	si.wShowWindow = SW_HIDE;          // 此成员设为TRUE的话则显示新建进程的主窗口，
						// 为FALSE的话则不显示
	BOOL bRet = ::CreateProcess(
		NULL,           // 不在此指定可执行文件的文件名
		szCommandLine,      // 命令行参数
		NULL,           // 默认进程安全性
		NULL,           // 默认线程安全性
		TRUE,          // 指定当前进程内的句柄不可以被子进程继承
		CREATE_NEW_CONSOLE , // 为新进程创建一个新的控制台窗口
		NULL,           // 使用本进程的环境变量
		NULL,           // 使用本进程的驱动器和目录
		&si,
		&pi);

	if (bRet)
	{
		//WaitForSingleObject(pi.hProcess, INFINITE);
		// 既然我们不使用两个句柄，最好是立刻将它们关闭
		//::CloseHandle(pi.hThread);
		//::CloseHandle(pi.hProcess);
		if (cmdOutput != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(cmdOutput);
		}

		//printf(" 新进程的进程ID号：%d \n", pi.dwProcessId);
		//printf(" 新进程的主线程ID号：%d \n", pi.dwThreadId);
		runState = true;
	}
	return 0;
}

//处理按钮消息
void dealWithBtnMsg(HWND hwnd, WPARAM wParam, LPARAM lParam, HWND *hBtn)
{
	WCHAR btnLab[10];
	WCHAR roleNum[10];
	static PROCESS_INFORMATION pi;
	static bool  runSate = false;
	switch (LOWORD(wParam))
	{
	case ID_BTN_HIDE:			//处理隐藏按钮消息
		ShowWindow(hwnd, SW_MINIMIZE);		//先最小化
		ShowWindow(hwnd, SW_HIDE);			//再隐藏
		return;

	case ID_BTN_STOP:			//处理停止\启动
		GetWindowText(hBtn[0], btnLab, 10);
		if (wcscmp(btnLab, TEXT("停止")) == 0)
		{
			if (StopInterpreter(pi, runSate))
			{
				SetWindowText(hBtn[0], TEXT("启动"));
				SetWindowText(hBtn[6], TEXT("运行"));
			}
		}
		else
		{
			GetWindowText(hBtn[5], roleNum, 10);
			//MessageBox(hwnd, roleNum, L"提示", MB_OK | MB_ICONINFORMATION);
			if (lParam == 1)
			{
				QuickRunInterpreter(pi, runSate, roleNum);
			}
			else
			{
				RunInterpreter(pi, runSate, roleNum);
			}
			SetWindowText(hBtn[0], TEXT("停止"));

		}
		return;
	case ID_BTN_PAUSE:			//处理暂停\运行
		if (runSate)
		{
			GetWindowText(hBtn[6], btnLab, 10);
			if (wcscmp(btnLab, TEXT("暂停")) == 0)
			{
				if (PauseInterpreter(pi, runSate))
				{
					SetWindowText(hBtn[6], TEXT("运行"));
				}
			}
			else
			{
				if (PauseInterpreter(pi, runSate))
				{
					SetWindowText(hBtn[6], TEXT("暂停"));
					SendMessage(hwnd, WM_COMMAND, ID_BTN_HIDE | BN_CLICKED, 0);
				}
			}
		}
		return;
	case ID_BTN_EXIT:			//处理退出消息
		if (runSate)
		{
			StopInterpreter(pi, runSate);
		}
		SendMessage(hwnd, WM_DESTROY, 0, 0);
		return;

	case ID_ID_AUTORUN:			//处理开机自启动消息
		VirtualMouseAutorun(hBtn[3]);
		return;
	case ID_EDIT_ROLE:			//第几个角色
		return;
	}
}

//////////////////////////////////////////////////////////////////////////

//绘制提示信息
void drawTipText(HDC hdc)
{
	int i = 1, x = 10, y = 30;

	TCHAR szTip[][128] = {
		TEXT(" 选择角色:"),
		TEXT("________________________"),
		TEXT(" 运行/暂停: Ctrl + F9"),
		TEXT(" 启动/停止: Ctrl + F10"),
		TEXT(" 快启/停止: Ctrl + F11"),
		TEXT(" 唤出 窗口: Ctrl + F12"),
		TEXT("________________________"),
	};
	TextOut(hdc, 10, 13, szTip[0], lstrlen(szTip[0]));
	for (i; i < sizeof(szTip) / sizeof(szTip[0]); i++)
	{
		TextOut(hdc, x, y, szTip[i], lstrlen(szTip[i]));
		y += 20;
	}
}
