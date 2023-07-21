// Globals.cpp

#include "stdafx.h"
#include "globals.h"

DWORD g_dwSVKModOrder[] =
{
    SVK_APPS,
    SVK_WIN,
    SVK_RWIN,
    SVK_CONTROL,
    SVK_RCONTROL,
    SVK_SHIFT,
    SVK_RSHIFT,
    SVK_MENU,
    SVK_RMENU,
    0
};

DWORD g_dwVKModOrder[] =
{
    VK_APPS,
    VK_LWIN,
    VK_RWIN,
    VK_CONTROL,
    VK_RCONTROL,
    VK_SHIFT,
    VK_RSHIFT,
    VK_MENU,
    VK_RMENU,
    0
};

// Global variables
BOOL g_bRegEx;
BOOL g_bWaitDate;
DATE g_dtWait;
BOOL g_bCaseSensitive;
TCHAR g_szEXEString[MAX_PATH];
BOOL g_bSent;
BOOL g_bListFailedWindows;
BOOL g_bWaitForSuccess;
BOOL g_bWaitForEnabled;
BOOL g_bRepeatForever;
BOOL g_bCheckForeground;
BOOL g_bRestoreForeground;
BOOL g_bIncludeHidden;
BOOL g_bSendInputAsWM;
BOOL g_bRenameWindow;
BOOL g_bMatchForegroundWindow;
BOOL g_bLeaveInBackground;
BOOL g_bFullHelp;
BOOL g_bQuiet;
BOOL g_bMatchClass;
BOOL g_bWaitPressed;
BOOL g_bWaitBoth;
BOOL g_bSendToEveryWindow;
BOOL g_bSendScanCodes;
BOOL g_bWaitForWindowNoExist;
BOOL g_bSendToParent;
BOOL g_bWaitForWindowDisabled;
BOOL g_bSearchEveryWindow;
BOOL g_bFindWindowOnly;
BOOL g_bPrintDialogItemID;
BOOL g_bHandleReport;
BOOL g_bPrintedSingleHandle;
BOOL g_bAppIsHWND;
BOOL g_bReadHWNDFromSTDIN;
BOOL g_bVerbose;
BOOL g_bVerboseCounters;
int g_iRepeatAll;
int g_iMouseDelay;
int g_iEventDelay;
int g_iDialogItemID[MAX_DIALOG_ID];
std::wstring g_strDialogItemText[MAX_DIALOG_ID];
TCHAR g_szRename[256];
TCHAR g_szMatchClass[256];
TString g_strWaitForKeys;
int g_iPoll;
int g_iTimeout;
POINT g_mouseOrigin;

enum Operators { OP_NONE, OP_ADD, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE, OP_MOD };

struct GetMonitorInfoStruct
{
    int Requested;
    int Current;
    int Width;
    int Height;
};

__int64 StringToInteger(std::wstring str)
{
    TCHAR * szEnd;
    if (str.substr(0, 2).compare(_T("0x")) == 0)
    {
        return _tcstoul(str.substr(2).c_str(), &szEnd, 16); 
    }
    return _tcstol(str.c_str(), &szEnd, 10);
}

double CalculateNext(double fCurrent, DWORD op, double fNew)
{
    switch(op)
    {
        case OP_NONE:      return fNew;
        case OP_ADD:       return fCurrent + fNew;
        case OP_SUBTRACT:  return fCurrent - fNew;
        case OP_MULTIPLY:  return fCurrent * fNew;
        case OP_DIVIDE:    return (fNew == 0.0 ? fCurrent : fCurrent / fNew);
        case OP_MOD:       return (int) fCurrent % (int) fNew;
        default:           return fNew;
    }
}

std::wstring GetSubExpression(std::wstring str, size_t index, size_t * pEnd = NULL)
{
    std::wstring wsMatching = _T("()[]{}<>");
    wchar_t cBegin = str.at(index);
    wchar_t cEnd = cBegin;
    size_t iOpen = wsMatching.find(cBegin);
    if (iOpen != wsMatching.npos)
        cEnd = wsMatching.at(iOpen+1);

    size_t i = index + 1;
    while(i < str.length())
    {
        if (str.at(i) == cEnd)
        {
            if (pEnd != NULL)
                *pEnd = i;
            return str.substr(index+1, i - index - 1);
        }
        i++;
    }

    if (pEnd != NULL)
        *pEnd = i;
    return str.substr(index+1);
}

double ExtractNumber(std::wstring str, size_t index, size_t * pEnd = NULL)
{
    // Examples:
    // 0x12cf
    // .5
    // 7

    TCHAR * szEnd;
    LPCWSTR wsz;
    std::wstring strSub;

    if (str.length() > 2 && str.at(index) == _T('0') && toupper(str.at(index+1)) == _T('X'))
    {
        std::wstring strSub = str.substr(index+2);
        wsz = strSub.c_str();
        __int64 lResult = _tcstoi64(wsz, &szEnd, 16); 
        if (pEnd != NULL)
        {
            *pEnd = szEnd - wsz + index + 2;
            if (*pEnd > index)
                (*pEnd)--;
        }
        return (double) lResult;
    }

    double fResult = 0.0;
    strSub = str.substr(index);
    wsz = strSub.c_str();
    fResult = _tcstod(wsz, &szEnd); 
    if (pEnd != NULL)
    {
        *pEnd = szEnd - wsz + index;
        if (*pEnd > index)
            (*pEnd)--;
    }
    return fResult;
}

int GetWindowWidth(HWND hwnd)
{
    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);
    return rcWindow.right - rcWindow.left;
}

int GetWindowHeight(HWND hwnd)
{
    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);
    return rcWindow.bottom - rcWindow.top;
}

int GetWorkAreaWidth()
{
    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

    return rc.right - rc.left;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    GetMonitorInfoStruct * pInfo = (GetMonitorInfoStruct *) dwData;
    pInfo->Current++;
    if (pInfo->Current == pInfo->Requested)
    {
        pInfo->Width = lprcMonitor->right - lprcMonitor->left;
        pInfo->Height = lprcMonitor->bottom - lprcMonitor->top;
        return FALSE;
    }
    return TRUE;
}

int GetMonitorWidth(int iMonitor)
{
    GetMonitorInfoStruct info;
    info.Requested = iMonitor;
    info.Current = 0;
    info.Width = 0;
    info.Height = 0;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM) &info); 
    return info.Width;
}

int GetMonitorHeight(int iMonitor)
{
    GetMonitorInfoStruct info;
    info.Requested = iMonitor;
    info.Current = 0;
    info.Width = 0;
    info.Height = 0;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM) &info); 
    return info.Height;
}

int GetWorkAreaHeight()
{
    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

    return rc.bottom - rc.top;
}

double ExpressionToDouble(std::wstring str, HWND hwnd)
{
    // Understood symbols:
    //
    // ( ) + - * / % 
    //
    // Understood contants:
    //
    // w    Screen width
    // h    Screen height
    // W    Work area width
    // H    Work area height
    // ww   Window width
    // hh   Window height
    // o1   Original mouse X position
    // o2   Original mouse Y position
    // c1   Current mouse X position
    // c2   Current mouse Y position

    size_t i = 0;
    size_t end = 0;
    double fNum = 0;
    double fCurrentValue = 0.0;
    int iVariable = 0;
    DWORD lastOp = OP_NONE;
    POINT pt;

    while(i < str.length())
    {
        switch(str.at(i))
        {
            case _T(' '):
            case _T('\t'):
            case _T('\r'):
            case _T('\n'):
                break;

            case _T('w'):
                if (str.length() > i+1)
                {
                    if (isdigit(str.at(i+1)))
                    {
                        // Use monitor number specified by character
                        iVariable = GetMonitorWidth(str.at(i+1) - _T('0'));
                        i++;
                    }
                    else if (str.at(i+1) == 'w')
                    {
                        iVariable = GetWindowWidth(hwnd);
                        i++;
                    }
                    else
                        iVariable = GetSystemMetrics(SM_CXSCREEN);
                }
                else
                    iVariable = GetSystemMetrics(SM_CXSCREEN);

                fCurrentValue = CalculateNext(fCurrentValue, lastOp, iVariable);
                lastOp = OP_NONE;
                break;

            case _T('W'):
                fCurrentValue = CalculateNext(fCurrentValue, lastOp, GetWorkAreaWidth());
                lastOp = OP_NONE;
                break;

            case _T('o'):
                iVariable = 0;
                if (str.length() > i+1)
                {
                    if (tolower(str.at(i+1)) == '1')
                    {
                        iVariable = g_mouseOrigin.x;
                        i++;
                    }
                    else if (tolower(str.at(i+1)) == '2')
                    {
                        iVariable = g_mouseOrigin.y;
                        i++;
                    }
                }

                fCurrentValue = CalculateNext(fCurrentValue, lastOp, iVariable);
                lastOp = OP_NONE;
                break;

            case _T('c'):
                iVariable = 0;
                if (str.length() > i+1)
                {
                    GetCursorPos(&pt);
                    if (tolower(str.at(i+1)) == '1')
                    {
                        iVariable = pt.x;
                        i++;
                    }
                    else if (tolower(str.at(i+1)) == '2')
                    {
                        iVariable = pt.y;
                        i++;
                    }
                }

                fCurrentValue = CalculateNext(fCurrentValue, lastOp, iVariable);
                lastOp = OP_NONE;
                break;

            case _T('h'):
                if (str.length() > i+1)
                {
                    if (isdigit(str.at(i+1)))
                    {
                        // Use monitor number specified by character
                        iVariable = GetMonitorHeight(str.at(i+1) - _T('0'));
                        i++;
                    }
                    else if (str.at(i+1) == 'h')
                    {
                        iVariable = GetWindowHeight(hwnd);
                        i++;
                    }
                    else
                        iVariable = GetSystemMetrics(SM_CYSCREEN);
                }
                else
                    iVariable = GetSystemMetrics(SM_CYSCREEN);
                fCurrentValue = CalculateNext(fCurrentValue, lastOp, iVariable);
                lastOp = OP_NONE;
                break;

            case _T('H'):
                fCurrentValue = CalculateNext(fCurrentValue, lastOp, GetWorkAreaHeight());
                lastOp = OP_NONE;
                break;

            case _T('-'):
                if (lastOp != OP_NONE)
                {
                    // Already performing an operation; this will negate the next value instead of subtracting
                    fCurrentValue = CalculateNext(fCurrentValue, lastOp, ExtractNumber(str, i, &end));
                    i = end;
                    break;
                }

                lastOp = OP_SUBTRACT;
                break;

            case _T('+'):
                lastOp = OP_ADD;
                break;

            case _T('*'):
                lastOp = OP_MULTIPLY;
                break;

            case _T('/'):
                lastOp = OP_DIVIDE;
                break;

            case _T('%'):
                lastOp = OP_MOD;
                break;

            case _T('('):
                fCurrentValue = CalculateNext(fCurrentValue, lastOp, ExpressionToDouble(GetSubExpression(str, i, &end), hwnd));
                i = end;
                break;

            case _T(')'):
                return fCurrentValue;

            case _T('.'):
            case _T('0'):
            case _T('1'):
            case _T('2'):
            case _T('3'):
            case _T('4'):
            case _T('5'):
            case _T('6'):
            case _T('7'):
            case _T('8'):
            case _T('9'):
                fCurrentValue = CalculateNext(fCurrentValue, lastOp, ExtractNumber(str, i, &end));
                lastOp = OP_NONE;
                i = end;
                break;

            default:
                // ignore invalid values
                break;
        }

        i++;
    }

    return fCurrentValue;
}
