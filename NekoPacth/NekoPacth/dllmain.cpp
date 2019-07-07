#include "stdafx.h"
#include "detours.h"
#include <iostream>
#include <mutex>
#pragma comment(lib, "detours.lib")
using namespace std;

//hook窗口没有效果，要去yscfg.ybn里面修改，注意保持头部的一致。hook msgbox会导致无法显示。
//边界是数组，找到双字节检查表，把81到FE的位置全部填01即可。
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
	strcpy(lf.lfFaceName, "黑体");
	lf.lfCharSet = GB2312_CHARSET;
	return (Func_CreateFontIndirectA(p_CreateFontIndirectA))(&lf);
}


int WINAPI MessageBoxAEx(_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType)
{
	if (lpText == ("ゲームを終了します。よろしですか？"))
	{
		return MessageBoxAOLD(hWnd, "确定要结束吗?", lpCaption, uType);
	}
	if (lpText == ("こちらでよろしいですか？（途中で变更は出来ません）"))
	{
		return MessageBoxAOLD(hWnd, "这样可以吗？（游戏途中不能改变名字）", lpCaption, uType);
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
		MessageBoxW(NULL, L"启动失败", L"YukiYuki-System Ver0.94", MB_OK | MB_ICONWARNING);
	}
	else
	{
		MessageBoxW(NULL, L"本补丁由【白井木学院】制作\n禁止一切录播直播和商业行为", L"YukiYuki-System Ver0.94", MB_OK | MB_ICONINFORMATION);
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