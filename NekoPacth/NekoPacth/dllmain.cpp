#include "stdafx.h"
#include "detours.h"
#include <iostream>
#include <mutex>
#pragma comment(lib, "detours.lib")
using namespace std;

//hook����û��Ч����Ҫȥyscfg.ybn�����޸ģ�ע�Ᵽ��ͷ����һ�¡�hook msgbox�ᵼ���޷���ʾ��
//�߽������飬�ҵ�˫�ֽڼ�����81��FE��λ��ȫ����01���ɡ�
typedef HFONT(WINAPI* Func_CreateFontIndirectA)(LOGFONTA *lplf);
PVOID p_CreateFontIndirectA = CreateFontIndirectA;
PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND(WINAPI *pfuncCreateWindowExA)(
	DWORD dwExStyle,
	LPCTSTR lpClassName,
	LPCTSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam);
typedef int (WINAPI *fnMessageboxA)(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType
	);
fnMessageboxA MessageBoxAOLD;

HFONT WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	LOGFONTA lf;
	memcpy(&lf, lplf, sizeof(LOGFONTA));
	strcpy(lf.lfFaceName, "����");
	lf.lfCharSet = GB2312_CHARSET;
	return (Func_CreateFontIndirectA(p_CreateFontIndirectA))(&lf);
}


int WINAPI MessageBoxAEx(_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType)
{
	if (lpText == ("���`���K�ˤ��ޤ�������Ǥ�����"))
	{
		return MessageBoxAOLD(hWnd, "ȷ��Ҫ������?", lpCaption, uType);
	}
	if (lpText == ("������Ǥ�����Ǥ�������;�ФǱ���ϳ����ޤ���"))
	{
		return MessageBoxAOLD(hWnd, "���������𣿣���Ϸ;�в��ܸı����֣�", lpCaption, uType);
	}
}


void BeginDetour()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&p_CreateFontIndirectA, NewCreateFontIndirectA);;
	MessageBoxAOLD = (fnMessageboxA)GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxA");
	DetourAttach((void**)&MessageBoxAOLD, MessageBoxAEx);
	if (DetourTransactionCommit() != NO_ERROR)
	{
		MessageBoxW(NULL, L"����ʧ��", L"YukiYuki-System Ver0.94", MB_OK | MB_ICONWARNING);
	}
	else
	{
		MessageBoxW(NULL, L"�������ɡ��׾�ľѧԺ������\n��ֹһ��¼��ֱ������ҵ��Ϊ", L"YukiYuki-System Ver0.94", MB_OK | MB_ICONINFORMATION);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//must load texts before Hook!
		BeginDetour();
	case DLL_THREAD_ATTACH:
		//Unload();
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void dummy(void)
{
	return;
}