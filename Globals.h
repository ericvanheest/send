// Globals.h

#pragma once

#include "stdafx.h"
#include <minwindef.h>
#include <string>

#ifdef UNICODE
#define TString std::wstring
#else
#define TString std::string
#endif

// Global definitions
#define SVK_LSHIFT		0x0100
#define SVK_SHIFT		SVK_LSHIFT
#define SVK_LCONTROL	0x0200
#define SVK_CONTROL		SVK_LCONTROL
#define SVK_LMENU		0x0400
#define SVK_MENU		SVK_LMENU

#define SVK_RSHIFT		0x1000
#define SVK_RCONTROL	0x2000
#define SVK_RMENU		0x4000

#define SVK_RELEASE		0x8000
#define SVK_APPS        0x0800

#define SVK_LWIN        0x00010000
#define SVK_RWIN        0x00020000
#define SVK_WIN         SVK_LWIN

#define SVK_SLEEP       0x00040000
#define SVK_MESSAGE     0x00080000
#define SVK_MOUSE       0x00100000
#define SVK_WINDOW      0x00200000

#define SVK_UNICODE     0x00400000
#define SVK_CLIPBOARD   0x00800000

#define SVK_ANYMOD  (SVK_LSHIFT | SVK_RSHIFT | SVK_LCONTROL | SVK_RCONTROL | SVK_LMENU | SVK_RMENU | SVK_APPS | SVK_LWIN | SVK_RWIN)

#define MAX_INPUTS      20

#define SLEEPTIME		10

#define MAX_DIALOG_ID   16
#define WINDOW_NEXT     -2
#define WINDOW_PREV     -3
#define WINDOW_FIRST    -4
#define WINDOW_LAST     -5
#define WINDOW_CHILD    -6
#define WINDOW_OWNER    -7
#define WINDOW_TITLE    -8

// Global functions
extern DWORD g_dwSVKModOrder[];
extern DWORD g_dwVKModOrder[];
extern std::map<std::wstring, DWORD> g_mapWM;
extern void InitWM();
extern DWORD GetWM(std::wstring strMessage);

// Global variables
extern BOOL g_bRegEx;
extern BOOL g_bCaseSensitive;
extern DATE g_dtWait;
extern TCHAR g_szEXEString[MAX_PATH];
extern BOOL g_bSent;
extern BOOL g_bWaitDate;
extern BOOL g_bListFailedWindows;
extern int g_iDialogItemID[MAX_DIALOG_ID];
extern std::wstring g_strDialogItemText[MAX_DIALOG_ID];
extern BOOL g_bWaitForSuccess;
extern BOOL g_bWaitForEnabled;
extern BOOL g_bRepeatForever;
extern BOOL g_bCheckForeground;
extern BOOL g_bRestoreForeground;
extern BOOL g_bIncludeHidden;
extern BOOL g_bSendInputAsWM;
extern BOOL g_bFullHelp;
extern BOOL g_bRenameWindow;
extern TCHAR g_szRename[];
extern BOOL g_bMatchForegroundWindow;
extern BOOL g_bLeaveInBackground;
extern BOOL g_bQuiet;
extern BOOL g_bMatchClass;
extern BOOL g_bWaitPressed;
extern BOOL g_bWaitBoth;
extern BOOL g_bSendToEveryWindow;
extern BOOL g_bSendScanCodes;
extern BOOL g_bWaitForWindowNoExist;
extern BOOL g_bSendToParent;
extern BOOL g_bWaitForWindowDisabled;
extern BOOL g_bSearchEveryWindow;
extern BOOL g_bFindWindowOnly;
extern BOOL g_bPrintDialogItemID;
extern BOOL g_bHandleReport;
extern BOOL g_bAppIsHWND;
extern BOOL g_bReadHWNDFromSTDIN;
extern BOOL g_bVerbose;
extern BOOL g_bVerboseCounters;
extern int g_iRepeatAll;
extern int g_iEventDelay;
extern int g_iMouseDelay;
extern int g_iPoll;
extern int g_iTimeout;
extern TCHAR g_szMatchClass[];
extern TString g_strWaitForKeys;
extern BOOL g_bPrintedSingleHandle;

#define MOUSE_BUTTON_LEFT         0x00000001
#define MOUSE_BUTTON_MIDDLE       0x00000002
#define MOUSE_BUTTON_RIGHT        0x00000004
#define MOUSE_BUTTON_X1           0x00000008
#define MOUSE_BUTTON_X2           0x00000010
#define MOUSE_WHEEL               0x00000100
#define MOUSE_BUTTON_NONE         0x00000000
#define MOUSE_ACTION_DOWN         0x00010000
#define MOUSE_ACTION_UP           0x00020000
#define MOUSE_ACTION_CLICK        0x00040000
#define MOUSE_ACTION_RETURN       0x00080000
#define MOUSE_ACTION_NONE         0x00000000
#define MOUSE_LOCAL              1
#define MOUSE_GLOBAL             2

#define MES_RELATIVE_X           0x00000001
#define MES_RELATIVE_Y           0x00000002

#define WES_RELATIVE_X           0x00000001
#define WES_RELATIVE_Y           0x00000002
#define WES_RELATIVE_WIDTH       0x00000004
#define WES_RELATIVE_HEIGHT      0x00000008
#define WES_OPPOSITE_X           0x00000010
#define WES_OPPOSITE_Y           0x00000020
#define WES_OPPOSITE_WIDTH       0x00000040
#define WES_OPPOSITE_HEIGHT      0x00000080

typedef struct
{
    int iButton;
    int iAction;
    int iX;
    int iY;
    int iOptions;
    DWORD dwFlags;
    int iPause;
    int iRepeat;
    int iRepeatDelay;
} MouseEventStruct;

typedef struct
{
    int iX;
    int iY;
    int iWidth;
    int iHeight;
    DWORD dwFlags;
    int iRepeat;
    int iRepeatDelay;
} WindowEventStruct;

typedef struct
{
    MSG msg;
    std::wstring strWParam; 
    std::wstring strLParam;
    BOOL bSendMessage;
    int iRepeat;
    int iRepeatDelay;
}  WrapMsg;

extern __int64 StringToInteger(std::wstring str);
extern double ExpressionToDouble(std::wstring str, HWND hwnd = NULL);
extern POINT g_mouseOrigin;

extern void FullHelp();
extern WORD CodeForVKey(BYTE vKey);
