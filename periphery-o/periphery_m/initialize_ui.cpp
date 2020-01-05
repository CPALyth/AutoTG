
#include "stdafx.h"
#include "initialize_ui.h"
#include "constant.h"
#include <CommCtrl.h>

//////////////////////////////////////////////////////////////////////////

//��ʼ��������
void InitWndUI( HWND hwnd, HINSTANCE hInstance, HWND *hwndBtn, HFONT hFont, LOGFONT lf )
{
	//////////////////////////////////////////////////////////////////////////
	hwndBtn[5] = CreateWindow(
		TEXT("edit"), TEXT(""),
		WS_CHILD | WS_VISIBLE ,
		80, 10, 20, 20,
		hwnd, (HMENU)ID_EDIT_ROLE, hInstance, NULL
	);
	SetWindowText(hwndBtn[5], TEXT("1"));
	SendMessage(hwndBtn[5], WM_SETFONT, (WPARAM)hFont, 0);

	//�ĸ���ť
	hwndBtn[0] = CreateWindow(
		TEXT("button"), TEXT("����"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		190, 10, 80, 40,
		hwnd, (HMENU)ID_BTN_STOP, hInstance, NULL
	);
	SendMessage( hwndBtn[0], WM_SETFONT, (WPARAM)hFont, 0 );

	hwndBtn[6] = CreateWindow(
		TEXT("button"), TEXT("����"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		190, 60, 80, 40,
		hwnd, (HMENU)ID_BTN_PAUSE, hInstance, NULL
	);
	SendMessage(hwndBtn[6], WM_SETFONT, (WPARAM)hFont, 0);

	hwndBtn[1] = CreateWindow(
		TEXT("button"), TEXT("���ش���"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		190, 110, 80, 40,
		hwnd, (HMENU)ID_BTN_HIDE, hInstance, NULL
	);
	SendMessage( hwndBtn[1], WM_SETFONT, (WPARAM)hFont, 0 );

	hwndBtn[2] = CreateWindow(
		TEXT("button"), TEXT("�˳�"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		190, 160, 80, 40,
		hwnd, (HMENU)ID_BTN_EXIT, hInstance, NULL
	);
	SendMessage( hwndBtn[2], WM_SETFONT, (WPARAM)hFont, 0 );

	//////////////////////////////////////////////////////////////////////////
	//��ѡ��
	hwndBtn[3] = CreateWindow(
		TEXT("button"), TEXT("��ʾ������־"),
		WS_CHILD|WS_VISIBLE|BS_CHECKBOX,
		10, 190, 120, 20,
		hwnd, (HMENU)ID_ID_AUTORUN, hInstance, NULL
	);
	lf.lfWidth = 7;
	lf.lfHeight = 16;
	SendMessage( hwndBtn[3], WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0 );

	//////////////////////////////////////////////////////////////////////////
	//������
#if 0
	lf.lfWidth			= 6;
	lf.lfHeight			= 16;
	//lf.lfUnderline		= 1;
	hwndBtn[4] = CreateWindowEx(0, WC_LINK,
		TEXT("<a href=\"\">http://www.dtcoming.com</a>"),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP,
		10, 220, 180, 20,
		hwnd, (HMENU)ID_ID_HYPERLINK, hInstance, NULL);
		
		/*
		CreateWindow(
		TEXT("syslink"), TEXT("����"),
		WS_CHILD|WS_VISIBLE|SS_NOTIFY,
		10, 230, 180, 20,
		hwnd, (HMENU)ID_ID_HYPERLINK, hInstance, 0	);
		*/
	SetWindowText( hwndBtn[4], TEXT("<a href=\"\">http://www.dtcoming.com</a>") );
	SendMessage( hwndBtn[4], WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0 );
#endif
}
