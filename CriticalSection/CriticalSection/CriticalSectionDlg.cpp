#include "CriticalSectionDlg.h"
#include <iostream>
#include <fstream>
#include <string>

CriticalSectionDlg* CriticalSectionDlg::ptr = NULL;

CRITICAL_SECTION cs;

CriticalSectionDlg::CriticalSectionDlg(void)
{
	ptr = this;
}

CriticalSectionDlg::~CriticalSectionDlg(void)
{
	DeleteCriticalSection(&cs);
}

void CriticalSectionDlg::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL CriticalSectionDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	hEdit1 = GetDlgItem(hwnd, IDC_EDIT1);
	InitializeCriticalSection(&cs);
	return TRUE;
}

void MessageAboutError(DWORD dwError)
{
	LPVOID lpMsgBuf = NULL;
	TCHAR szBuf[300]; 

	BOOL fOK = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	if(lpMsgBuf != NULL)
	{
		wsprintf(szBuf, TEXT("Ошибка %d: %s"), dwError, lpMsgBuf); 
		MessageBox(0 ,szBuf, TEXT("Сообщение об ошибке"), MB_OK | MB_ICONSTOP); 
		LocalFree(lpMsgBuf); 
	}
}

DWORD WINAPI Write(LPVOID lp)
{
	EnterCriticalSection(&cs);
	ifstream in;
	
	int index = 0;
	HWND hEdit = HWND(lp);
	_TCHAR str[50];
	GetWindowText(hEdit, (LPWSTR)str, 50);
	index = std::stoi(str);
	char tc[50];
	ofstream copy;
	
	for (int i = 0; i < index; i++)
	{
		
		wsprintf(str, TEXT("array%d.txt"), i);
		
		copy.open(str);
		in.open("array.txt");
		if (!copy.is_open())
		{
			MessageAboutError(GetLastError());
			LeaveCriticalSection(&cs);
			return 1;
		}
		if (!in.is_open())
		{
			MessageAboutError(GetLastError());
			LeaveCriticalSection(&cs);
			return 1;
		}
		while (!in.eof())
		{
			in >> tc;
			copy << tc;
			copy << '\n';
		}
		copy.close();
		in.close();

	}
	in.close();
	LeaveCriticalSection(&cs);
	MessageBox(0, TEXT("Поток записал информацию в файл"), TEXT("Критическая секция"), MB_OK);
	return 0;
}

DWORD WINAPI Read(LPVOID lp)
{
	EnterCriticalSection(&cs);
	ofstream of(TEXT("arrayNew.txt"));
	ifstream copy;
	int index = 0;
	char tc[50];
	if(!of.is_open())
	{
		MessageAboutError(GetLastError());
		return 1;
	}
	while (true)
	{
		_TCHAR str[50];
		wsprintf(str, TEXT("array%d.txt"), index);
		copy.open(str);
		if (!copy.is_open())
		{
			LeaveCriticalSection(&cs);
			return 1;
		}
		while (!copy.eof())
		{
			copy >> tc;
			of << tc;
			of << '\n';
		}
		index++;
		copy.close();
		
	}
	LeaveCriticalSection(&cs);

	return 0;
}


void CriticalSectionDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if(id == IDC_BUTTON1)
	{
		HANDLE hThread = CreateThread(NULL, 0, Write, hEdit1, 0, NULL); 
		CloseHandle(hThread);
	}
	if (id == IDC_BUTTON2)
	{
		HANDLE hThread = CreateThread(NULL, 0, Read, 0, 0, NULL);
		CloseHandle(hThread);
	}
}

BOOL CALLBACK CriticalSectionDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}