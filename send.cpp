// send.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winuser.h>
#include "globals.h"
#include "regexp.h"
#include <math.h>
#include "WaitKey.h"

#ifndef VK_NULL
#define VK_NULL 0
#endif

void Usage()
{
    _tprintf(_T("\
Send, version 1.53\n\
\n\
Usage:    send [options] \"app\" \"keys\" [time]\n\
\n\
Example:  send \"virtualdub\" \"\\{F6}\" 7:55pm\n\
\n\
Options:  -a     Send the command to every matching window\n\
          -b     Leave the selected window in the background (WM/pos only)\n\
          -c     Use a case-sensitive search\n\
          -d     Do not restore the forground window afterwards\n\
          -D t   Delay \"t\" ms between each key/mouse event\n\
          -e     Search every window in the system (even child windows)\n\
          -f     Repeatedly send the string (forever)\n\
          -F     Process keys only if \"app\" is currently in the foreground\n\
          -g     Use the foreground window (ignores the specified app)\n\
          -h     Include hidden windows in the search\n\
          -i     Use the dialog item ID specified\n\
          -I     Print the dialog item ID specified\n\
          -j     Interpret \"app\" as a specific window handle (hex or dec)\n\
          -J     Read a window handle from STDIN instead of using \"app\"\n\
          -k     Send commands only if \"waitkeys\" are held down\n\
          -K     Send commands only if \"waitkeys\" are released\n\
          -l     List captions of all windows not matching \"application\"\n\
          -m t   Pause for \"t\" ms between moving the mouse and clicking\n\
          -M     Send keyboard and mouse commands as windows messages\n\
          -n     Give the found window a new caption\n\
          -o     Print the handle of the first window that matches \"app\"\n\
          -O     Find all windows that match \"app\" and print a report\n\
          -p     Send windows messages to the parent of the control\n\
          -P t   Use \"t\" as the polling time in ms (default: %d)\n\
          -q     Quiet mode; don't display messages\n\
          -r     Use regular expression matching on \"application\"\n\
          -R t   Repeat the sequence of keys \"t\" times\n\
          -s     Also match the window class specified\n\
          -S     Use the scan codes when sending keyboard messages\n\
          -t t   Wait a maximum of \"t\" ms for -w/-W/-x/-X/-f options\n\
          -u     Send commands only if \"waitkeys\" are pressed and released\n\
          -v     Verbose output (debug)\n\
          -V     Verbose output (counters)\n\
          -w     Wait for the window to exist\n\
          -W     Wait for the window to be enabled\n\
          -x     Wait for the window to not exist\n\
          -X     Wait for the window to be disabled\n\
          time   Wait until specified time to send the keys\n\
\n\
          --fullhelp  Print the full documentation\n"), SLEEPTIME);
}

BOOL g_bEnabledDisabledFlag;

std::wstring WrapNewline(LPCTSTR sz);

std::wstring UnquoteString(const wchar_t *wsz)
{
    if (*wsz != '\'')
        return wsz;

    std::wstring wsResult = L"";

    while (1)
    {
        wsz++;
        if (*wsz == '\\')
        {
            wsz++;
            wsResult += *wsz;
        }
        else if (*wsz == '\'')
            break;
        else if (*wsz == '\0')
            break;
        else
            wsResult.append(1, *wsz);
    }

    return wsResult;
}

void MakeLower(const TCHAR *sz, TCHAR *szlc, int iMax)
{
    int i = 0;
    while (*sz != '\0')
    {
        *szlc = tolower(*sz);
        sz++;
        szlc++;
        i++;
        if (i >= iMax)
        {
            *szlc = '\0';
            return;
        }
    }
    *szlc = '\0';
}

BOOL CharInString(LPTSTR sz, TCHAR c)
{
    c = tolower(c);

    while (*sz != '\0')
    {
        if (tolower(*sz) == c)
            return TRUE;
        sz++;
    }

    return FALSE;
}

class CSendInfo
{
public:
    std::wstring m_strKeys;
    std::wstring m_strApp;
    std::wstring m_strVariable;
    bool m_bTopLevel;
};

DWORD GetNormalKey(TCHAR c)
{
    short iKey = VkKeyScan(c);
    if (iKey == -1)
    {
        return (DWORD)c | SVK_UNICODE;
    }

    DWORD dwResult = (BYTE)(iKey & 0xff);

    if (iKey & 0x0100)
        dwResult |= SVK_SHIFT;

    if (iKey & 0x0200)
        dwResult |= SVK_CONTROL;

    if (iKey & 0x0400)
        dwResult |= SVK_MENU;

    return dwResult;
}

DWORD GetQuotedKey(TCHAR c)
{
    switch (c)
    {
    case 'a':
        return VK_MENU;
    case 'A':
        return VK_MENU | SVK_RELEASE;
    case 'b':
    case 'B':
        return VK_BACK;
    case 'c':
        return VK_CONTROL;
    case 'C':
        return VK_CONTROL | SVK_RELEASE;
    case 'd':
    case 'D':
        return VK_DOWN;
    case 'e':
    case 'E':
        return VK_ESCAPE;
    case 'k':
    case 'K':
        return VK_LBUTTON;
    case 'l':
    case 'L':
        return VK_LEFT;
    case 's':
        return VK_SHIFT;                    // and zero for the VK
    case 'S':
        return VK_SHIFT | SVK_RELEASE;      // and zero for the VK
    case 't':
    case 'T':
        return VK_TAB;
    case 'n':
    case 'N':
        return VK_RETURN;
    case 'p':
        return VK_APPS;
    case 'P':
        return VK_APPS | SVK_RELEASE;
    case 'r':
    case 'R':
        return VK_RIGHT;
    case 'u':
    case 'U':
        return VK_UP;
    case 'w':
        return VK_LWIN;
    case 'W':
        return VK_LWIN + SVK_RELEASE;
    default:
        return GetNormalKey(c);
    }
}

DWORD GetMouseCommand(LPTSTR sz, MouseEventStruct *pEvent)
{
    TCHAR szKey[MAX_PATH];
    int i = 0;
    int iStr = 0;
    enum { Mouse, Button, Action, PointX, PointY, Options, Pause };
    int iEventPart = Mouse;

    pEvent->iAction = MOUSE_ACTION_NONE;
    pEvent->iButton = MOUSE_BUTTON_NONE;
    pEvent->iOptions = MOUSE_GLOBAL;
    pEvent->iX = 0;
    pEvent->iY = 0;
    pEvent->dwFlags = 0;
    pEvent->iPause = 0;

    int iLen;

    while (*sz != '}')
    {
        if (*sz == '\0')
            break;
        iStr = 0;
        while (*sz != ',')
        {
            if (*sz == '\0')
                break;
            if (*sz == ']')
                break;
            szKey[iStr] = *sz;
            i++;
            iStr++;
            sz++;
        }
        szKey[iStr] = '\0';
        if (*sz != '\0')
            sz++;
        i++;

        switch (iEventPart)
        {
        case Mouse:
            break;
        case Button:
            switch (szKey[0])
            {
            case 'l':
            case 'L':
                pEvent->iButton = MOUSE_BUTTON_LEFT;
                break;
            case 'r':
            case 'R':
                pEvent->iButton = MOUSE_BUTTON_RIGHT;
                break;
            case 'm':
            case 'M':
                pEvent->iButton = MOUSE_BUTTON_MIDDLE;
                break;
            case '1':
                pEvent->iButton = MOUSE_BUTTON_X1;
                break;
            case '2':
                pEvent->iButton = MOUSE_BUTTON_X2;
                break;
            case 'n':
            case 'N':
                pEvent->iButton = MOUSE_BUTTON_NONE;
                break;
            case 'w':
            case 'W':
                pEvent->iButton = MOUSE_WHEEL;
                break;
            default:
                break;
            }
            break;
        case Action:
            switch (szKey[0])
            {
            case 'd':
            case 'D':
                pEvent->iAction = MOUSE_ACTION_DOWN;
                break;
            case 'u':
            case 'U':
                pEvent->iAction = MOUSE_ACTION_UP;
                break;
            case 'c':
                pEvent->iAction = MOUSE_ACTION_CLICK;
                break;
            case 'C':
                pEvent->iAction = MOUSE_ACTION_CLICK | MOUSE_ACTION_RETURN;
                break;
            case 'n':
            case 'N':
                pEvent->iAction = MOUSE_ACTION_NONE;
                break;
            default:
                break;
            }
            break;
        case PointX:
            pEvent->iX = (int)ExpressionToDouble(szKey);
            iLen = _tcslen(szKey);
            if (iLen > 0)
            {
                if (tolower(szKey[iLen - 1]) == 'r')
                {
                    pEvent->dwFlags |= MES_RELATIVE_X;
                }
            }
            break;
        case PointY:
            pEvent->iY = (int)ExpressionToDouble(szKey);
            iLen = _tcslen(szKey);
            if (iLen > 0)
            {
                if (tolower(szKey[iLen - 1]) == 'r')
                {
                    pEvent->dwFlags |= MES_RELATIVE_Y;
                }
            }
            break;
        case Options:
            switch (szKey[0])
            {
            case 'l':
            case 'L':
            case 'c':
            case 'C':
                pEvent->iOptions = MOUSE_LOCAL;
                break;
            case 's':
            case 'S':
            case 'g':
            case 'G':
                pEvent->iOptions = MOUSE_GLOBAL;
                break;
            default:
                break;
            }
            break;
        case Pause:
            pEvent->iPause = (int)StringToInteger(szKey);
            break;
        default:;
        }
        iEventPart++;
    }

    return SVK_MOUSE;
}

DWORD GetWindowCommand(LPTSTR sz, WindowEventStruct *pEvent, HWND hwnd)
{
    TCHAR szKey[MAX_PATH];
    int i = 0;
    int iStr = 0;
    enum { Window, H1, V1, H2, V2 };
    int iEventPart = Window;

    pEvent->dwFlags = 0;
    pEvent->iHeight = 0;
    pEvent->iWidth = 0;
    pEvent->iX = 0;
    pEvent->iY = 0;

    bool bPosition = true;
    int iTemp;

    int iLen;

    while (*sz != '}')
    {
        if (*sz == '\0')
            break;
        iStr = 0;
        while (*sz != ',')
        {
            if (*sz == '\0')
                break;
            if (*sz == ']')
                break;
            szKey[iStr] = *sz;
            i++;
            iStr++;
            sz++;
        }
        szKey[iStr] = '\0';
        if (*sz != '\0')
            sz++;
        i++;

        switch (iEventPart)
        {
        case H1:
        case V1:
        case H2:
        case V2:
            iTemp = (int)ExpressionToDouble(szKey, hwnd);
            iLen = _tcslen(szKey);
            break;
        default:
            break;
        }

        switch (iEventPart)
        {
        case Window:
            switch (szKey[1])
            {
            case 'p':
            case 'P':
                bPosition = true;
                break;
            case 's':
            case 'S':
                bPosition = false;
                break;
            }
            break;
        case H1:
            if (iLen > 0)
            {
                switch (szKey[iLen - 1])
                {
                case 'r':
                case 'R':
                    pEvent->dwFlags |= (bPosition ? WES_RELATIVE_X : WES_RELATIVE_WIDTH);
                    break;
                case 'x':
                case 'X':
                    pEvent->dwFlags |= (bPosition ? WES_OPPOSITE_X : WES_OPPOSITE_WIDTH);
                    break;
                }
            }
            if (bPosition)
                pEvent->iX = iTemp;
            else
                pEvent->iWidth = iTemp;
            break;
        case V1:
            if (iLen > 0)
            {
                switch (szKey[iLen - 1])
                {
                case 'r':
                case 'R':
                    pEvent->dwFlags |= (bPosition ? WES_RELATIVE_Y : WES_RELATIVE_HEIGHT);
                    break;
                case 'x':
                case 'X':
                    pEvent->dwFlags |= (bPosition ? WES_OPPOSITE_Y : WES_OPPOSITE_HEIGHT);
                    break;
                }
                if (tolower(szKey[iLen - 1]) == 'r')
                {
                }
            }
            if (bPosition)
                pEvent->iY = iTemp;
            else
                pEvent->iHeight = iTemp;
            break;
        case H2:
            if (iLen > 0)
            {
                if (tolower(szKey[iLen - 1]) == 'r')
                {
                    pEvent->dwFlags |= (bPosition ? WES_RELATIVE_WIDTH : WES_RELATIVE_X);
                }
            }
            if (!bPosition)
                pEvent->iX = iTemp;
            else
                pEvent->iWidth = iTemp;
            break;
        case V2:
            if (iLen > 0)
            {
                if (tolower(szKey[iLen - 1]) == 'r')
                {
                    pEvent->dwFlags |= (bPosition ? WES_RELATIVE_HEIGHT : WES_RELATIVE_Y);
                }
            }
            if (!bPosition)
                pEvent->iY = iTemp;
            else
                pEvent->iHeight = iTemp;
            break;
        default:;
        }
        iEventPart++;
    }

    if (!bPosition && iEventPart == H2)
    {
        pEvent->dwFlags |= (WES_RELATIVE_X | WES_RELATIVE_Y);
    }

    return SVK_WINDOW;
}

DWORD GetSpecialKey(LPCTSTR sz, int *piNew, WrapMsg *pMsg, MouseEventStruct *pMouseEventStruct, WindowEventStruct *pWindowEvent, HWND hwnd)
{
    TCHAR szKey[MAX_PATH];
    int i = 0;

    while (*sz != '}')
    {
        if (*sz == '\0')
            break;
        szKey[i] = *sz;
        i++;
        sz++;
    }
    szKey[i] = '\0';
    if (piNew)
        *piNew = i + 2;

    int iRepeat = 1;
    int iRepeatDelay = 0;
    int iIndex = 0;

    if (tolower(szKey[0]) == 'r')
    {
        iIndex++;
        int i = 0;
        while (isdigit(szKey[iIndex + i]))
            i++;

        TCHAR sz[32];
        memset(sz, 0, 32);
        _tcsncpy_s(sz, 32, &szKey[iIndex], i);
        iRepeat = (int)StringToInteger(sz);

        if (szKey[iIndex + i] == ':')
        {
            iIndex = iIndex + i + 1;
            i = 0;
            while (isdigit(szKey[iIndex + i]))
                i++;
            memset(sz, 0, 32);
            _tcsncpy_s(sz, 32, &szKey[iIndex], i);
            iRepeatDelay = (int)StringToInteger(sz);
        }

        iIndex = iIndex + i + 1;
    }

    if (pMsg != NULL)
    {
        pMsg->iRepeat = iRepeat;
        pMsg->iRepeatDelay = iRepeatDelay;
    }
    if (pMouseEventStruct != NULL)
    {
        pMouseEventStruct->iRepeat = iRepeat;
        pMouseEventStruct->iRepeatDelay = iRepeatDelay;
    }
    if (pWindowEvent != NULL)
    {
        pWindowEvent->iRepeat = iRepeat;
        pWindowEvent->iRepeatDelay = iRepeatDelay;
    }

    switch (szKey[iIndex])
    {
        // "Delete" key is anything that starts with "d"
    case 'd':
    case 'D':
        return VK_DELETE;

        // "End" key is anything that starts with "e"
    case 'e':
    case 'E':
        return VK_END;

        // Function keys F1-F24 (undefined for above F24)
    case 'F':
    case 'f':
        i = _ttol(&szKey[iIndex + 1]);
        if (i < 1)
            return 0;
        return VK_F1 + (i - 1);

        // "Home" key is anything that starts with "h"
    case 'h':
    case 'H':
        return VK_HOME;

        // "Insert" key is anything that starts with "i"
    case 'i':
    case 'I':
        return VK_INSERT;

        // Mouse movements are handled separately
    case 'm':
    case 'M':
        return GetMouseCommand(&szKey[iIndex], pMouseEventStruct);

        // Window movements are also a separate event
    case 'w':
    case 'W':
        return GetWindowCommand(&szKey[iIndex], pWindowEvent, hwnd);

        // Numeric keypad keys begin with "n" (i.e. \{n*} is the numeric "multiply" key)
    case 'n':
    case 'N':
        switch (szKey[iIndex + 1])
        {
        case '0':   return VK_NUMPAD0;
        case '1':   return VK_NUMPAD1;
        case '2':   return VK_NUMPAD2;
        case '3':   return VK_NUMPAD3;
        case '4':   return VK_NUMPAD4;
        case '5':   return VK_NUMPAD5;
        case '6':   return VK_NUMPAD6;
        case '7':   return VK_NUMPAD7;
        case '8':   return VK_NUMPAD8;
        case '9':   return VK_NUMPAD9;
        case '*':   return VK_MULTIPLY;
        case '+':   return VK_ADD;
        case '-':   return VK_SUBTRACT;
        case '/':   return VK_DIVIDE;
        case '.':   return VK_DECIMAL;
        default:    return VK_NUMLOCK;
        }

        // PageUp, PageDown, PrintScreen, and Pause all begin with "p" - so
        // check what follows to determine which key to send
    case 'p':
    case 'P':
        if (CharInString(&szKey[iIndex + 1], 'u'))
            return VK_PRIOR;    // PageUp
        else if (CharInString(&szKey[iIndex + 1], 'd'))
            return VK_NEXT;     // PageDown
        else if (CharInString(&szKey[iIndex + 1], 'c'))
            return VK_SNAPSHOT; // PrintScreen
        else
            return VK_PAUSE;    // Pause

                                // "Scroll Lock" is "s" followed by anything other than a number.
                                // "s" followed by a number means "sleep for N milliseconds"
    case 's':
    case 'S':
        i = _ttol(&szKey[iIndex + 1]);
        if (i < 1)
            return VK_SCROLL;   // Scroll Lock
        if (pMsg)
            pMsg->msg.lParam = (DWORD)i;
        return SVK_SLEEP;

        // Unicode characters can be input by \{uXXXX} where XXXX is a
        // hexadecimal number.
    case 'u':
    case 'U':
        if (_stscanf_s(&szKey[iIndex + 1], _T("%x"), &i) < 1)
            return VK_NULL; // Invalid hex code

        return VK_NULL; // Not currently supported

                        // Clipboard is anything that begins with 'C'
    case 'c':
    case 'C':
        return SVK_CLIPBOARD;

    default:
        return 0;   // No idea what this key is
    }

    return 0;
}

DWORD GetVariableOrWM(std::wstring ws, HWND hwnd)
{
    if (ws == L"id")
        return GetDlgCtrlID(hwnd);
    else if (ws == L"hwnd")
        return (DWORD)hwnd;
    return GetWM(ws.c_str());
}

DWORD EvaluateExpression(LPCTSTR sz, HWND hwnd)
{
    // Evaluate expressions such as "BN_CLICKED:id"
    std::wstring str = sz;
    size_t iColon = str.find(L":");
    if (iColon == str.npos)
        return GetVariableOrWM(sz, hwnd);

    DWORD dw1 = GetVariableOrWM(str.substr(0, iColon), hwnd);
    DWORD dw2 = GetVariableOrWM(str.substr(iColon + 1), hwnd);

    DWORD dwMsg = MAKELONG(dw2, dw1);

    return dwMsg;
}

DWORD GetWindowMessage(LPCTSTR sz, int *piNew, WrapMsg *pMsg, HWND hwnd)
{
    TCHAR szKey[4096];
    int i = 0;
    int iStr = 0;
    enum { Message, WParam, LParam, SendOrPost };
    int iMsgPart = Message;
    DWORD dwValue;

    pMsg->msg.hwnd = NULL;
    pMsg->msg.message = WM_NULL;
    pMsg->msg.lParam = 0;
    pMsg->msg.wParam = 0;
    GetCursorPos(&(pMsg->msg.pt));
    pMsg->msg.time = 0;
    pMsg->bSendMessage = TRUE;

    bool bContinue = true;

    int iRepeat = 1;
    int iRepeatDelay = 0;

    if (tolower(*sz) == 'r')
    {
        sz++;
        int iRepeatIndex = 0;
        while (isdigit(sz[iRepeatIndex]))
            iRepeatIndex++;

        TCHAR szNum[32];
        memset(szNum, 0, 32);
        _tcsncpy_s(szNum, 32, sz, iRepeatIndex);
        iRepeat = (int)StringToInteger(szNum);

        if (sz[iRepeatIndex] == ':')
        {
            iRepeatIndex++;
            sz += iRepeatIndex;

            i = iRepeatIndex;

            iRepeatIndex = 0;

            while (isdigit(sz[iRepeatIndex]))
                iRepeatIndex++;

            memset(szNum, 0, 32);
            _tcsncpy_s(szNum, 32, sz, iRepeatIndex);
            iRepeatDelay = (int)StringToInteger(szNum);
        }

        sz += iRepeatIndex + 1;
        i += iRepeatIndex + 2;
    }

    if (pMsg != NULL)
    {
        pMsg->iRepeat = iRepeat;
        pMsg->iRepeatDelay = iRepeatDelay;
    }

    while (bContinue)
    {
        if (*sz == '\0')
            break;
        iStr = 0;
        bool bInString = false;
        while (*sz != ',' || bInString)
        {
            if (*sz == '\\')
            {
                szKey[iStr] = '\\';
                sz++;
                iStr++;
                i++;
                szKey[iStr] = *sz;
            }
            else if (bInString)
            {
                if (*sz == '\'')
                    bInString = false;
                if (*sz == '\0')
                {
                    bContinue = false;
                    break;
                }
            }
            else
            {
                if (*sz == '\'')
                    bInString = true;
                else if ((*sz == '\0') || (*sz == ']'))
                {
                    bContinue = false;
                    break;
                }
            }
            szKey[iStr] = *sz;
            i++;
            iStr++;
            sz++;
        }
        szKey[iStr] = '\0';
        if (*sz != '\0')
            sz++;
        i++;

        if (*szKey == '\'')
        {
            if (iMsgPart == LParam)
            {
                pMsg->strLParam = UnquoteString(szKey);
                if (g_bVerbose)
                    _tprintf(_T("LParam: %s\n"), pMsg->strLParam.c_str());
            }
            else if (iMsgPart == WParam)
            {
                pMsg->strWParam = UnquoteString(szKey);
                if (g_bVerbose)
                    _tprintf(_T("WParam: %s\n"), pMsg->strWParam.c_str());
            }
        }
        else if (iMsgPart == SendOrPost)
        {
            if (tolower(*szKey) == 'p')
                pMsg->bSendMessage = FALSE;
            iMsgPart++;
        }
        else
        {
            dwValue = EvaluateExpression(szKey, hwnd);

            switch (iMsgPart)
            {
            case Message:
                if (g_bVerbose)
                    _tprintf(_T("Message: %d\n"), dwValue);
                pMsg->msg.message = dwValue;
                break;
            case LParam:
                if (g_bVerbose)
                    _tprintf(_T("LParam: %d\n"), dwValue);
                pMsg->msg.lParam = dwValue;
                break;
            case WParam:
                if (g_bVerbose)
                    _tprintf(_T("WParam: %d\n"), dwValue);
                pMsg->msg.wParam = dwValue;
                break;
            default:;
            }
        }

        iMsgPart++;
    }

    if (piNew)
        *piNew = i + 1;

    return SVK_MESSAGE;
}

int ReleaseMods(INPUT *pInputs, DWORD dwModifiers, int iIndex)
{
    int iMod = 0;

    while (g_dwSVKModOrder[iMod] != 0)
    {
        if (dwModifiers & g_dwSVKModOrder[iMod])
        {
            pInputs[iIndex].ki.wVk = (WORD)g_dwVKModOrder[iMod];
            pInputs[iIndex].ki.dwFlags = KEYEVENTF_KEYUP;
            iIndex++;
        }
        iMod++;
    }

    return iIndex;
}

int GetMods(INPUT *pInputs, DWORD dwModifiers, int iIndex)
{
    // Add keyboard modifiers (Control, Alt, Shift, Windows key, Apps key)
    // to the input array.

    DWORD dwFlags = (dwModifiers & SVK_RELEASE) ? KEYEVENTF_KEYUP : 0;

    int iMod = 0;

    while (g_dwSVKModOrder[iMod] != 0)
    {
        if (dwModifiers & g_dwSVKModOrder[iMod])
        {
            pInputs[iIndex].ki.wVk = (WORD)g_dwVKModOrder[iMod];
            pInputs[iIndex].ki.dwFlags = dwFlags;
            iIndex++;
        }
        iMod++;
    }

    return iIndex;
}

DWORD GetNextKey(LPCTSTR *psz, WrapMsg *pMsg, MouseEventStruct *pMouseEventStruct, WindowEventStruct *pWindowEvent, HWND hwnd)
{
    LPCTSTR sz = *psz;
    DWORD dwVKey;
    int i;

    switch (*sz)
    {
    case '\\':
        switch (sz[1])
        {
        case '{':
            dwVKey = GetSpecialKey(&sz[2], &i, pMsg, pMouseEventStruct, pWindowEvent, hwnd);
            sz += i;
            break;
        case '[':
            dwVKey = GetWindowMessage(&sz[2], &i, pMsg, hwnd);
            sz += i;
            break;
        default:
            dwVKey = GetQuotedKey(sz[1]);
            sz++;
        }
        break;
    default:
        dwVKey = GetNormalKey(*sz);
    }
    sz++;

    *psz = sz;
    return dwVKey;
}

void WindowPos(HWND hwnd, WindowEventStruct *pEvent)
{
    int iX = pEvent->iX;
    int iY = pEvent->iY;
    int iHeight = pEvent->iHeight;
    int iWidth = pEvent->iWidth;

    DWORD flags = SWP_NOZORDER;

    RECT rc;
    GetWindowRect(hwnd, &rc);

    if (pEvent->dwFlags & (WES_RELATIVE_X | WES_RELATIVE_Y | WES_OPPOSITE_X | WES_OPPOSITE_Y) && (iX == 0) && (iY == 0))
        flags |= SWP_NOMOVE;
    else
    {
        if (pEvent->dwFlags & WES_RELATIVE_X)
            iX += rc.left;

        if (pEvent->dwFlags & WES_RELATIVE_Y)
            iY += rc.top;

        if (pEvent->dwFlags & WES_OPPOSITE_X)
            iX -= (rc.right - rc.left);

        if (pEvent->dwFlags & WES_OPPOSITE_Y)
            iY -= (rc.bottom - rc.top);
    }

    if (iWidth == 0 && iHeight == 0)
    {
        flags |= SWP_NOSIZE;
    }
    else
    {
        if (pEvent->dwFlags & WES_RELATIVE_WIDTH)
            iWidth += (rc.right - rc.left);

        if (pEvent->dwFlags & WES_RELATIVE_HEIGHT)
            iHeight += (rc.bottom - rc.top);

        if (iWidth == 0)
            iWidth = (rc.right - rc.left);

        if (iHeight == 0)
            iHeight = (rc.bottom - rc.top);
    }

    SetWindowPos(hwnd, 0, iX, iY, iWidth, iHeight, flags);
    //    MoveWindow(hwnd, iX, iY, iWidth, iHeight, TRUE);
}

UINT SendInputAsWM(HWND hWnd, UINT nInputs, LPINPUT pInputs, int iPause)
{
    MOUSEINPUT *pMouse;
    KEYBDINPUT *pKB;
    RECT rcClient;
    POINT pt;
    GetClientRect(hWnd, &rcClient);

    UINT iCount = 0;
    for (size_t iInput = 0; iInput < nInputs; iInput++)
    {
        if (iInput == iPause)
            Sleep(g_iEventDelay);

        switch (pInputs[iInput].type)
        {
        case INPUT_MOUSE:
            pMouse = (MOUSEINPUT *)&pInputs[iInput].mi;
            if (pMouse->dwFlags & MOUSEEVENTF_ABSOLUTE)
            {
                pt.x = pMouse->dx;
                pt.y = pMouse->dy;
            }
            else
            {
                GetCursorPos(&pt);
                pt.x += pMouse->dx;
                pt.y += pMouse->dy;
            }
            ScreenToClient(hWnd, &pt);
            if (pMouse->dwFlags & MOUSEEVENTF_MOVE)
                PostMessage(hWnd, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_LEFTDOWN)
                PostMessage(hWnd, WM_LBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_LEFTUP)
                PostMessage(hWnd, WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_MIDDLEDOWN)
                PostMessage(hWnd, WM_MBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_MIDDLEUP)
                PostMessage(hWnd, WM_MBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_RIGHTDOWN)
                PostMessage(hWnd, WM_RBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_RIGHTUP)
                PostMessage(hWnd, WM_RBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_XDOWN)
                PostMessage(hWnd, WM_XBUTTONDOWN, MAKEWPARAM(0, pMouse->mouseData), MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_XUP)
                PostMessage(hWnd, WM_XBUTTONUP, MAKEWPARAM(0, pMouse->mouseData), MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_WHEEL)
                PostMessage(hWnd, WM_MOUSEWHEEL, MAKEWPARAM(0, pMouse->mouseData), MAKELPARAM(pt.x, pt.y));
            if (pMouse->dwFlags & MOUSEEVENTF_HWHEEL)
                PostMessage(hWnd, WM_MOUSEHWHEEL, MAKEWPARAM(0, pMouse->mouseData), MAKELPARAM(pt.x, pt.y));
            break;
        case INPUT_KEYBOARD:
            pKB = (KEYBDINPUT *)&pInputs[iInput].ki;
            if (pKB->dwFlags & KEYEVENTF_KEYUP)
                PostMessage(hWnd, WM_KEYUP, pKB->wVk, MAKELPARAM(1, 0xc000 | (g_bSendScanCodes ? pKB->wScan : 0)));
            else
                PostMessage(hWnd, WM_KEYDOWN, pKB->wVk, MAKELPARAM(1, g_bSendScanCodes ? pKB->wScan : 0));
            iCount++;
            break;
        default:
            break;
        }
    }

    return iCount;
}

std::wstring GetClipboardText()
{
    if (!OpenClipboard(nullptr))
        return L"";

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == nullptr)
        return L"";

    // Lock the handle to get the actual text pointer
    wchar_t *pwszText = static_cast<wchar_t *>(GlobalLock(hData));
    if (pwszText == nullptr)
        return L"";

    std::wstring text(pwszText);
    GlobalUnlock(hData);
    CloseClipboard();

    return text;
}

void SendKeys(HWND hwnd, std::wstring strKeys, LPCTSTR szText, bool bRecurse = false)
{
    HWND hwndPrev = GetForegroundWindow();

    if (strKeys.length() < 1)
        return; // Nothing to do

    if (!g_bLeaveInBackground && !bRecurse)
        SetForegroundWindow(hwnd);

    if ((!g_bRepeatForever) && (!g_bQuiet) && (!bRecurse))
    {
        TCHAR szRepeat[32];
        memset(szRepeat, 0, 32);
        if (g_iRepeatAll > 1)
            _stprintf_s(szRepeat, 32, _T(" %d times"), g_iRepeatAll);

        if (g_bMatchForegroundWindow)
            _tprintf(_T("Sending \"%s\" to foreground window%s\n"), strKeys.c_str(), szRepeat);
        else if (_tcslen(g_szMatchClass) == 0)
            _tprintf(_T("Sending \"%s\" to \"%s\"%s\n"), strKeys.c_str(), szText, szRepeat);
        else
            _tprintf(_T("Sending \"%s\" to \"%s\" (%s)%s\n"), strKeys.c_str(), szText, g_szMatchClass, szRepeat);
    }

    int i = 0;
    int iLen = strKeys.length();
    LPCTSTR sz = strKeys.c_str();
    int vkey;
    BYTE bVK;
    DWORD dwShift;
    WrapMsg msg;
    MouseEventStruct mouseEventStruct;
    WindowEventStruct windowEvent;
    INPUT inputs[MAX_INPUTS];
    bool bUnicode = false;

    //    RECT rcWorkArea;
    //    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
    //    int iScreenX = rcWorkArea.right - rcWorkArea.left;
    //    int iScreenY = rcWorkArea.bottom - rcWorkArea.top;

    int iScreenX = GetSystemMetrics(SM_CXSCREEN);
    int iScreenY = GetSystemMetrics(SM_CYSCREEN);

    for (i = 0; i < MAX_INPUTS; i++)
    {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wVk = 0;
        inputs[i].ki.wScan = 0;
        inputs[i].ki.dwFlags = 0;
        inputs[i].ki.time = 0;
    }

    // If the EventDelay is set, this will the the index into the inputs array after which we want a delay
    int iPause = -1;

    for (int iRepeatAll = 0; iRepeatAll < g_iRepeatAll; iRepeatAll++)
    {
        if (g_bVerboseCounters)
        {
            printf("Repeat: %d\r", iRepeatAll);
        }
        sz = strKeys.c_str();
        while (*sz != '\0')
        {
            msg.iRepeat = 1;
            msg.iRepeatDelay = 0;
            vkey = GetNextKey(&sz, &msg, &mouseEventStruct, &windowEvent, hwnd);
            bUnicode = (vkey & SVK_UNICODE ? true : false);

            bVK = vkey & 0xff;
            dwShift = vkey & 0xffffff00;

            // Sleep if desired
            if (dwShift & SVK_SLEEP)
                Sleep(msg.msg.lParam);

            // Send the contents of the clipboard if requested
            if (dwShift & SVK_CLIPBOARD)
            {
                // Avoid recursive bomb if clipboard text has "\{Clipboard}" in it
                if (!bRecurse)
                    SendKeys(hwnd, GetClipboardText(), szText, true);
            }

            // Send a windows message if requested
            if (dwShift & SVK_MESSAGE)
            {
                for (int iRepeatCount = 0; iRepeatCount < msg.iRepeat; iRepeatCount++)
                {
                    // Some hacks
                    if (msg.msg.message == WM_SHOWWINDOW)
                    {
                        ShowWindow(hwnd, msg.msg.wParam ? SW_SHOW : SW_HIDE);
                    }
                    else
                    {
                        if (msg.bSendMessage)
                            SendMessage(g_bSendToParent ? GetParent(hwnd) : hwnd, msg.msg.message, msg.strWParam.length() == 0 ? msg.msg.wParam : (WPARAM)msg.strWParam.c_str(), msg.strLParam.length() == 0 ? msg.msg.lParam : (LPARAM)msg.strLParam.c_str());
                        else
                            PostMessage(g_bSendToParent ? GetParent(hwnd) : hwnd, msg.msg.message, msg.strWParam.length() == 0 ? msg.msg.wParam : (WPARAM)msg.strWParam.c_str(), msg.strLParam.length() == 0 ? msg.msg.lParam : (LPARAM)msg.strLParam.c_str());
                    }

                    if (msg.iRepeatDelay > 0)
                        Sleep(msg.iRepeatDelay);
                }
            }

            if (vkey == SVK_MOUSE)
            {
                for (int iRepeatCount = 0; iRepeatCount < mouseEventStruct.iRepeat; iRepeatCount++)
                {
                    BOOL bRelative = FALSE;

                    INPUT iMouse;
                    iMouse.type = INPUT_MOUSE;
                    iMouse.mi.time = 0;
                    iMouse.mi.dwFlags = 0;
                    iMouse.mi.mouseData = 0;

                    POINT pt;
                    POINT ptCurrent;
                    GetCursorPos(&ptCurrent);

                    pt.x = mouseEventStruct.iX;
                    pt.y = mouseEventStruct.iY;

                    if (mouseEventStruct.iOptions == MOUSE_LOCAL)
                    {
                        ClientToScreen(hwnd, &pt);
                    }

                    bRelative = (((mouseEventStruct.dwFlags & MES_RELATIVE_X) > 0) ||
                        ((mouseEventStruct.dwFlags & MES_RELATIVE_Y) > 0));
                    /*
                    if ((mouseEventStruct.dwFlags & MES_RELATIVE_X) > 0)
                    {
                    pt.x += ptCurrent.x;
                    }

                    if ((mouseEventStruct.dwFlags & MES_RELATIVE_Y) > 0)
                    {
                    pt.y += ptCurrent.y;
                    }
                    */

                    if (bRelative)
                    {
                        iMouse.mi.dwFlags &= ~MOUSEEVENTF_ABSOLUTE;
                    }
                    else
                    {
                        iMouse.mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
                        pt.x = (pt.x + 1) * 65536 / iScreenX;
                        pt.y = (pt.y + 1) * 65536 / iScreenY;
                    }

                    iMouse.mi.dx = pt.x;
                    iMouse.mi.dy = pt.y;

                    BOOL bDown = (mouseEventStruct.iAction & MOUSE_ACTION_DOWN);
                    BOOL bClick = (mouseEventStruct.iAction & MOUSE_ACTION_CLICK);
                    BOOL bUp = (mouseEventStruct.iAction & MOUSE_ACTION_UP);
                    BOOL bReturn = (mouseEventStruct.iAction & MOUSE_ACTION_RETURN);

                    switch (mouseEventStruct.iButton)
                    {
                    case MOUSE_BUTTON_LEFT:
                        if (bUp)
                            iMouse.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
                        else if (bDown || bClick)
                            iMouse.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
                        break;
                    case MOUSE_BUTTON_MIDDLE:
                        if (bUp)
                            iMouse.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;
                        else if (bDown || bClick)
                            iMouse.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
                        break;
                    case MOUSE_BUTTON_RIGHT:
                        if (bUp)
                            iMouse.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
                        else if (bDown || bClick)
                            iMouse.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
                        break;
                    case MOUSE_WHEEL:
                        iMouse.mi.dwFlags |= MOUSEEVENTF_WHEEL;
                        if (bUp)
                            iMouse.mi.mouseData = WHEEL_DELTA;
                        else if (bDown)
                            iMouse.mi.mouseData = -WHEEL_DELTA;
                        break;
                    case MOUSE_BUTTON_X1:
                    case MOUSE_BUTTON_X2:
                        if (bDown || bClick)
                        {
                            iMouse.mi.dwFlags |= MOUSEEVENTF_XDOWN;
                        }
                        else if (bUp)
                        {
                            iMouse.mi.dwFlags |= MOUSEEVENTF_XUP;
                        }
                        iMouse.mi.mouseData = (mouseEventStruct.iButton == MOUSE_BUTTON_X2 ? XBUTTON2 : XBUTTON1);
                        break;
                    default:
                        break;
                    }

                    // If the co-ordinates aren't (-32767,-32767), move the mouse to the given
                    // location before clicking
                    if ((mouseEventStruct.iX != -32767) || (mouseEventStruct.iY != -32767))
                    {
                        iMouse.mi.dwFlags |= MOUSEEVENTF_MOVE;

                        if (g_iMouseDelay > 0)
                        {
                            DWORD dwTemp = iMouse.mi.dwFlags;
                            iMouse.mi.dwFlags &= ~(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_XDOWN);
                            // Move first, delay, then click
                            SendInput(1, &iMouse, sizeof(INPUT));
                            iMouse.mi.dwFlags = dwTemp;
                            Sleep(g_iMouseDelay);
                        }
                    }
                    else
                    {
                        POINT ptSet;
                        ptSet.x = (ptCurrent.x + 1) * 65536 / iScreenX;
                        ptSet.y = (ptCurrent.y + 1) * 65536 / iScreenY;
                        iMouse.mi.dx = ptSet.x;
                        iMouse.mi.dy = ptSet.y;
                    }

                    SendInput(1, &iMouse, sizeof(INPUT));

                    if (bClick)
                    {
                        if (mouseEventStruct.iPause > 0)
                            Sleep(mouseEventStruct.iPause);

                        iMouse.mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
                        switch (mouseEventStruct.iButton)
                        {
                        case MOUSE_BUTTON_LEFT:
                            iMouse.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
                            break;
                        case MOUSE_BUTTON_MIDDLE:
                            iMouse.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;
                            break;
                        case MOUSE_BUTTON_RIGHT:
                            iMouse.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
                            break;
                        case MOUSE_BUTTON_X1:
                            iMouse.mi.dwFlags |= MOUSEEVENTF_XUP;
                            iMouse.mi.mouseData = XBUTTON1;
                            break;
                        case MOUSE_BUTTON_X2:
                            iMouse.mi.dwFlags |= MOUSEEVENTF_XUP;
                            iMouse.mi.mouseData = XBUTTON2;
                            break;
                        default:
                            break;
                        }
                        if (g_iEventDelay)
                            Sleep(g_iEventDelay);
                        SendInput(1, &iMouse, sizeof(INPUT));
                        if (g_iEventDelay)
                            Sleep(g_iEventDelay);
                    }

                    if (bReturn)
                    {
                        Sleep(25);
                        iMouse.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
                        iMouse.mi.dx = (ptCurrent.x + 1) * 65536 / iScreenX;
                        iMouse.mi.dy = (ptCurrent.y + 1) * 65536 / iScreenY;

                        SendInput(1, &iMouse, sizeof(INPUT));
                    }

                    if (mouseEventStruct.iRepeatDelay > 0)
                        Sleep(mouseEventStruct.iRepeatDelay);
                }
            }
            else if (vkey == SVK_WINDOW)
            {
                for (int i = 0; i < windowEvent.iRepeat; i++)
                {
                    WindowPos(hwnd, &windowEvent);

                    if (windowEvent.iRepeatDelay > 0)
                        Sleep(windowEvent.iRepeatDelay);
                }
            }
            else
            {
                for (int iRepeatCount = 0; iRepeatCount < msg.iRepeat; iRepeatCount++)
                {
                    if (bVK == VK_LBUTTON)
                    {
                        // Click the mouse button
                        POINT pt;
                        GetCursorPos(&pt);
                        INPUT iMouse;
                        iMouse.type = INPUT_MOUSE;
                        iMouse.mi.dx = pt.x;
                        iMouse.mi.dy = pt.y;
                        iMouse.mi.time = 0;
                        iMouse.mi.mouseData = 0;
                        iMouse.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                        iMouse.mi.dwExtraInfo = 0;

                        SendInput(1, &iMouse, sizeof(INPUT));
                        iMouse.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                        if (g_iEventDelay)
                            Sleep(g_iEventDelay);
                        SendInput(1, &iMouse, sizeof(INPUT));
                        if (g_iEventDelay)
                            Sleep(g_iEventDelay);

                        continue;
                    }

                    // Apply any modifier keys necessary
                    i = 0;
                    if (dwShift & SVK_ANYMOD)
                        i = GetMods(inputs, dwShift, i);

                    bool bRelease = dwShift & SVK_RELEASE;
                    inputs[i].ki.dwExtraInfo = 0;

                    // Apply the actual virtual-key code desired
                    if (bVK && !bUnicode)
                    {
                        inputs[i].ki.wVk = bVK;
                        inputs[i].ki.dwFlags = bRelease ? KEYEVENTF_KEYUP : 0;
                        if (g_bSendScanCodes)
                        {
                            inputs[i].ki.dwFlags |= KEYEVENTF_SCANCODE;
                            DWORD code = CodeForVKey(bVK);
                            inputs[i].ki.wScan = code & 0xff;
                            if (code > 0xff) {
                                // This extended key requires multiple scan codes
                                inputs[i].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
                                inputs[i].ki.wVk = 0;
                                inputs[i].ki.wScan = (code & 0xff00) >> 8;
                                i++;
                                inputs[i].ki.wVk = bVK;
                                inputs[i].ki.dwFlags = bRelease ? KEYEVENTF_KEYUP : 0;
                                inputs[i].ki.dwFlags |= (KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY);
                                inputs[i].ki.wScan = code & 0xff;
                                inputs[i].ki.dwExtraInfo = 0;
                                printf("extended: %x %x\n", inputs[i - 1].ki.wScan, inputs[i].ki.wScan);
                            }
                        }
                        i++;
                    }
                    else if (bUnicode)
                    {
                        inputs[i].ki.wScan = (vkey & 0xffff);
                        inputs[i].ki.wVk = 0;
                        inputs[i].ki.dwFlags = bRelease ? KEYEVENTF_KEYUP : 0;
                        inputs[i].ki.dwFlags |= KEYEVENTF_UNICODE;
                        i++;
                    }

                    // We don't want to pause between each modifier key (shift, etc.), just the "real" desired keypress
                    if (!bRelease)  // No need for an extra pause if is just a release and not a press-release set
                        iPause = i;

                    if ((bVK || bUnicode) && !bRelease)
                    {
                        switch (bVK)
                        {
                        case VK_LSHIFT:
                        case VK_RSHIFT:
                        case VK_SHIFT:
                        case VK_LCONTROL:
                        case VK_RCONTROL:
                        case VK_CONTROL:
                        case VK_LMENU:
                        case VK_RMENU:
                        case VK_MENU:
                            // We don't want to release this key
                            break;
                        default:
                            inputs[i].ki.dwExtraInfo = 0;
                            if (bUnicode)
                            {
                                inputs[i].ki.wScan = (vkey & 0xffff);
                                inputs[i].ki.wVk = 0;
                                inputs[i].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
                            }
                            else
                            {
                                inputs[i].ki.wVk = bVK;
                                inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
                                if (g_bSendScanCodes)
                                {
                                    inputs[i].ki.dwFlags |= KEYEVENTF_SCANCODE;
                                    DWORD code = CodeForVKey(bVK);
                                    inputs[i].ki.wScan = code & 0xff;
                                    if (code > 0xff) {
                                        // This extended key requires multiple scan codes
                                        inputs[i].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
                                        inputs[i].ki.wVk = 0;
                                        inputs[i].ki.wScan = (code & 0xff00) >> 8;
                                        i++;
                                        inputs[i].ki.wVk = bVK;
                                        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
                                        inputs[i].ki.wScan = code & 0xff;
                                        inputs[i].ki.dwExtraInfo = 0;
                                    }
                                }
                            }
                            i++;
                        }
                    }

                    // Release any modifier keys that were "pressed down" prior to
                    // sending the virtual-key.
                    if ((dwShift & SVK_ANYMOD) && !bRelease)
                        i = ReleaseMods(inputs, dwShift, i);

                    // Re-acquire the foreground window in case something (like an Instant Message)
                    // decided to rudely pop up while we were sending keys.
                    if (!g_bLeaveInBackground)
                        SetForegroundWindow(hwnd);

                    if (i > 0)
                    {
                        // Send the stream of modifiers and keystroke to the foreground application
                        if (g_bSendInputAsWM)
                        {
                            // Turn inputs into WM_KEYDOWN, WM_KEYUP, etc. messages and send those instead.
                            SendInputAsWM(hwnd, i, inputs, iPause);
                        }
                        else if (g_iEventDelay == 0 || iPause == -1)
                        {
                            SendInput(i, inputs, sizeof(INPUT));
                        }
                        else
                        {
                            SendInput(iPause, inputs, sizeof(INPUT));
                            Sleep(g_iEventDelay);
                            SendInput(i - iPause, &inputs[iPause], sizeof(INPUT));
                        }

                        if (msg.iRepeatDelay > 0)
                            Sleep(msg.iRepeatDelay);
                    }
                    if (g_iEventDelay > 0)
                        Sleep(g_iEventDelay);
                }
            }
        }
    }
    if (g_bVerboseCounters)
        printf("\n");

    if ((g_bRestoreForeground) && (!g_bLeaveInBackground))
    {
        // Wait a few milliseconds before yanking focus away (prevents
        // keystrokes from being "eaten" in a lot of cases).
        Sleep(50);

        if (IsWindow(hwndPrev))
            SetForegroundWindow(hwndPrev);
    }
}

std::wstring ParseString(LPTSTR szParse, CSendInfo *pInfo)
{
    std::wstring strResult = szParse;

    // Replace the following:
    // \1    pInfo->m_strVariable
    // \1.h  High word of variable
    // \1.l  Low word of variable

    if (pInfo->m_strVariable == L"")
        return strResult;

    size_t iBackslash = strResult.find(L'\\');
    if (iBackslash == strResult.npos)
        return strResult;

    do
    {
        if (strResult[iBackslash + 1] == L'\\')
        {
            iBackslash = strResult.find(L'\\', iBackslash + 2);
            continue;
        }

        if (strResult[iBackslash + 1] == L'1')
        {
            if (strResult.substr(iBackslash + 1, 3) == L"1.h")
            {
                DWORD dw = (DWORD)StringToInteger(pInfo->m_strVariable);
                TCHAR sz[16];
                _ltot_s(HIWORD(dw), sz, 16, 10);
                strResult = strResult.replace(iBackslash, 4, sz);
            }
            else if (strResult.substr(iBackslash + 1, 3) == L"1.l")
            {
                DWORD dw = (DWORD)StringToInteger(pInfo->m_strVariable);
                TCHAR sz[16];
                _ltot_s(LOWORD(dw), sz, 16, 10);
                strResult = strResult.replace(iBackslash, 4, sz);
            }
            else
            {
                strResult = strResult.replace(iBackslash, 2, pInfo->m_strVariable);
            }
        }
        iBackslash = strResult.find(L'\\', iBackslash + 2);
    } while (iBackslash != strResult.npos);

    if (g_bVerbose)
    {
        _tprintf(_T("Replaced variable \"%s\" in string: %s\n"), pInfo->m_strVariable.c_str(), strResult.c_str());
    }
    return strResult;
}

HWND GetDlgItemRecursive(HWND hwnd, int item)
{
    HWND hwndResult = GetDlgItem(hwnd, item);
    if (hwndResult != NULL)
        return hwndResult;

    hwnd = GetWindow(hwnd, GW_CHILD);
    while (hwnd != NULL)
    {
        hwndResult = GetDlgItemRecursive(hwnd, item);
        if (hwndResult != NULL)
            return hwndResult;

        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }

    return NULL;
}

HWND GetDlgItemRecursive(HWND hwnd, std::wstring strText)
{
    if (hwnd == NULL)
        return NULL;

    TCHAR szText[256];
    GetWindowText(hwnd, szText, 255);
    if (wcsstr(szText, strText.c_str()) != NULL)
        return hwnd;

    hwnd = GetWindow(hwnd, GW_CHILD);
    while (hwnd != NULL)
    {
        HWND hwndResult = GetDlgItemRecursive(hwnd, strText);
        if (hwndResult != NULL)
            return hwndResult;

        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }

    return NULL;
}

HWND GetSpecifiedChildWindow(HWND hwnd)
{
    for (int iID = 0; iID < MAX_DIALOG_ID; iID++)
    {
        HWND hwndChild = hwnd;

        if (g_iDialogItemID[iID] == -1)
            break;

        if (g_iDialogItemID[iID] >= 0)
        {
            hwndChild = GetDlgItemRecursive(hwnd, g_iDialogItemID[iID]);
        }
        else switch (g_iDialogItemID[iID])
        {
        case WINDOW_NEXT:
            hwndChild = GetWindow(hwnd, GW_HWNDNEXT);
            break;
        case WINDOW_PREV:
            hwndChild = GetWindow(hwnd, GW_HWNDPREV);
            break;
        case WINDOW_FIRST:
            hwndChild = GetWindow(hwnd, GW_HWNDFIRST);
            break;
        case WINDOW_LAST:
            hwndChild = GetWindow(hwnd, GW_HWNDLAST);
            break;
        case WINDOW_CHILD:
            hwndChild = GetWindow(hwnd, GW_CHILD);
            break;
        case WINDOW_OWNER:
            hwndChild = GetWindow(hwnd, GW_OWNER);
            break;
        default:
            hwndChild = GetDlgItemRecursive(hwnd, g_strDialogItemText[WINDOW_TITLE - g_iDialogItemID[iID]]);
            break;
        }

        if (hwndChild != NULL)
            hwnd = hwndChild;
        else
            return NULL;   // can't find ID
    }

    return hwnd;
}

BOOL HandleReport(HWND hwnd)
{
    TCHAR szText[MAX_PATH];
    TCHAR szClass[MAX_PATH];
    if (g_bHandleReport)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        GetWindowText(hwnd, szText, MAX_PATH);
        GetClassName(hwnd, szClass, MAX_PATH);
        _tprintf(_T("0x%8.8x\t%d\t%d,%d %d,%d\t%dx%d\t%s\t%s\n"), (DWORD)hwnd, GetDlgCtrlID(hwnd), rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top, szClass, WrapNewline(szText).c_str());
        return TRUE;
    }
    if (!g_bPrintedSingleHandle)
    {
        if (g_bPrintDialogItemID)
            _tprintf(_T("%d\n"), GetDlgCtrlID(hwnd));
        else
            _tprintf(_T("0x%8.8x\n"), (DWORD)hwnd);
        g_bPrintedSingleHandle = TRUE;
    }
    return FALSE;
}

int SendToWindow(HWND hwnd, CSendInfo *pInfo, LPCTSTR szText)
{
    g_bSent = TRUE;

    if (g_bWaitForWindowNoExist)
        return FALSE;

    if (g_bCheckForeground)
    {
        HWND hwndForeground = GetForegroundWindow();
        if (hwndForeground != hwnd)
            return FALSE;
    }
    
    HWND hwndChild = GetSpecifiedChildWindow(hwnd);
    if (hwndChild == NULL)
        return FALSE;

    if (g_bWaitForWindowDisabled || g_bWaitForEnabled)
    {
        LONG lStyle = GetWindowLong(hwndChild, GWL_STYLE);
        if (g_bWaitForWindowDisabled && (lStyle & WS_DISABLED))
        {
            g_bEnabledDisabledFlag = FALSE;
            return FALSE;
        }
        else if (g_bWaitForEnabled && ((lStyle & WS_DISABLED) == 0))
        {
            g_bEnabledDisabledFlag = FALSE;
            return FALSE;
        }

        return TRUE;
    }

    if (g_bRenameWindow)
    {
        SetWindowText(hwndChild, g_szRename);
    }

    if (g_bFindWindowOnly)
        HandleReport(hwndChild);
    else
        SendKeys(hwndChild, pInfo->m_strKeys, szText);

    return 2;
}

std::wstring WrapNewline(LPCTSTR sz)
{
    std::wstring ws = sz;
    size_t iFind = ws.find(L'\n');

    if (iFind == ws.npos && sz[0] != L'"')
        return ws;

    iFind = ws.find(L'\\');
    while (iFind != ws.npos)
    {
        ws.replace(iFind, 1, L"\\\\");
        iFind = ws.find(L'\\', iFind + 2);
    }

    iFind = ws.find(L'"');
    while (iFind != ws.npos)
    {
        ws.replace(iFind, 1, L"\\\"");
        iFind = ws.find(L'"', iFind + 2);
    }

    iFind = ws.find(L'\n');
    while (iFind != ws.npos)
    {
        ws.replace(iFind, 1, L"\\n");
        iFind = ws.find(L'\n', iFind + 2);
    }

    return L'"' + ws + L'"';
}

BOOL CALLBACK EnumWindowsProc(
    HWND hwnd,      // handle to parent window
    LPARAM lParam   // application-defined value
)
{
    if (!g_bIncludeHidden)
    {
        if (!IsWindowVisible(hwnd))
            return TRUE;
    }

    TCHAR szText[MAX_PATH];
    TCHAR szClass[MAX_PATH];
    TCHAR szTextLC[MAX_PATH];
    TCHAR szClassLC[MAX_PATH];
    TCHAR szWindowLC[MAX_PATH];
    BOOL bMatch = FALSE;

    CSendInfo *pInfo = (CSendInfo *)lParam;
    GetWindowText(hwnd, szText, MAX_PATH);
    GetClassName(hwnd, szClass, MAX_PATH);

    if (g_bSearchEveryWindow && pInfo->m_bTopLevel)
    {
        pInfo->m_bTopLevel = false;
        EnumChildWindows(hwnd, EnumWindowsProc, (LPARAM)pInfo);
        pInfo->m_bTopLevel = true;
    }

    if (g_bCaseSensitive || g_bRegEx)
    {
        _tcscpy_s(szTextLC, MAX_PATH, szText);
        _tcscpy_s(szClassLC, MAX_PATH, szClass);
        _tcscpy_s(szWindowLC, MAX_PATH, pInfo->m_strApp.c_str());
    }
    else
    {
        MakeLower(szText, szTextLC, MAX_PATH);
        MakeLower(szClass, szClassLC, MAX_PATH);
        MakeLower(pInfo->m_strApp.c_str(), szWindowLC, MAX_PATH);
    }

    if (pInfo->m_strApp.length() > 0)
    {
        if (g_bRegEx)
        {
            Regexp re(pInfo->m_strApp.c_str(), !g_bCaseSensitive);
            bMatch = re.Match(szTextLC);
        }
        else
        {
            bMatch = (_tcsstr(szTextLC, szWindowLC) != NULL);
        }
    }
    else
        bMatch = TRUE;

    if (g_bMatchClass)
    {
        if (g_bRegEx)
        {
            Regexp re(g_szMatchClass, !g_bCaseSensitive);
            bMatch = bMatch && re.Match(szClassLC);
        }
        else
        {
            bMatch = bMatch && (_tcsicmp(g_szMatchClass, szClassLC) == 0);
        }
    }

    if (g_bMatchForegroundWindow)
    {
        hwnd = GetForegroundWindow();
        bMatch = TRUE;
    }

    if (bMatch)
    {
        if (_tcsstr(szTextLC, g_szEXEString))
            goto Ignore;

        if (g_bFindWindowOnly)
        {
            HWND hwndChild = GetSpecifiedChildWindow(hwnd);
            if (hwndChild != NULL)
                if (HandleReport(hwndChild))
                    return TRUE;
            return FALSE;
        }

        int iResult = SendToWindow(hwnd, pInfo, szText);
        switch (iResult)
        {
        case TRUE:
        case FALSE:
            return iResult;
        default:
            break;
        }

        if (g_bSendToEveryWindow)
            return TRUE;

        return FALSE;
    }

Ignore:
    if (g_bListFailedWindows)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        _tprintf(_T("0x%.8x \"%s\" (%s) [%d,%d-%d,%d : %dx%d]\n"), (long)hwnd, szText, szClass, rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top);
    }

    return TRUE;
}

void ParseFullCommand(LPTSTR szCommand)
{
    if (_tcsicmp(szCommand, _T("fullhelp")) == 0)
    {
        FullHelp();
        g_bFullHelp = TRUE;
    }
}

void GetDialogIDs(std::wstring str)
{
    int iID;
    int iNumTitles = 0;
    for (iID = 0; iID < MAX_DIALOG_ID; iID++)
    {
        size_t iEnd = str.find(',');
        if (iEnd == str.npos)
            iEnd = str.length();
        while (iEnd != str.npos && iEnd > 0 && str.at(iEnd - 1) == L'\\')
            iEnd = str.find(',', iEnd + 1);

        std::wstring strSub = str.substr(0, iEnd);

        if (strSub == _T("p"))
            g_iDialogItemID[iID] = WINDOW_PREV;
        else if (strSub == _T("n"))
            g_iDialogItemID[iID] = WINDOW_NEXT;
        else if (strSub == _T("f"))
            g_iDialogItemID[iID] = WINDOW_FIRST;
        else if (strSub == _T("l"))
            g_iDialogItemID[iID] = WINDOW_LAST;
        else if (strSub == _T("c"))
            g_iDialogItemID[iID] = WINDOW_CHILD;
        else if (strSub == _T("o"))
            g_iDialogItemID[iID] = WINDOW_OWNER;
        else if (strSub.at(0) == _T('\''))
        {
            g_iDialogItemID[iID] = WINDOW_TITLE - iNumTitles;
            g_strDialogItemText[iNumTitles] = UnquoteString(strSub.c_str());
            iNumTitles++;
        }
        else
            g_iDialogItemID[iID] = (int)StringToInteger(strSub);

        if (iEnd >= str.length())
        {
            iID++;
            break;
        }

        str = str.substr(iEnd + 1);
    }

    if (iID < MAX_DIALOG_ID)
        g_iDialogItemID[iID] = -1;
}

int ParseArgs(int argc, TCHAR *argv[])
{
    int iArg = 1;
    int iChar;

    if (argc < 2)
        return iArg;

    while (argv[iArg][0] == '-')
    {
        iChar = 1;
        while (argv[iArg][iChar] != '\0')
        {
            switch (argv[iArg][iChar])
            {
            case 'd':
                g_bRestoreForeground = FALSE;
                break;
            case 'D':
                if (iArg < argc - 1)
                {
                    g_iEventDelay = _ttol(argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'h':
            case 'H':
                g_bIncludeHidden = TRUE;
                break;
            case 'R':
                if (iArg < argc - 1)
                {
                    g_iRepeatAll = _ttol(argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'm':
                if (iArg < argc - 1)
                {
                    g_iMouseDelay = _ttol(argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'M':
                g_bSendInputAsWM = TRUE;
                break;
            case 'r':
                g_bRegEx = TRUE;
                break;
            case 'C':
            case 'c':
                g_bCaseSensitive = TRUE;
                break;
            case 'F':
                g_bCheckForeground = TRUE;
                g_bRestoreForeground = FALSE;
                g_bLeaveInBackground = TRUE;
                break;
            case 'f':
                g_bRepeatForever = TRUE;
                break;
            case 'G':
            case 'g':
                g_bMatchForegroundWindow = TRUE;
                break;
            case 'B':
            case 'b':
                g_bLeaveInBackground = TRUE;
                break;
            case 'i':
                if (iArg < argc - 1)
                {
                    GetDialogIDs(argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'l':
            case 'L':
                g_bListFailedWindows = TRUE;
                break;
            case 'w':
                g_bWaitForSuccess = TRUE;
                break;
            case 'W':
                g_bWaitForEnabled = TRUE;
                break;
            case 'q':
            case 'Q':
                g_bQuiet = TRUE;
                break;
            case 'a':
            case 'A':
                g_bSendToEveryWindow = TRUE;
                break;
            case 's':
                if (iArg < argc - 1)
                {
                    g_bMatchClass = TRUE;
                    _tcscpy_s(g_szMatchClass, MAX_PATH, argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'S':
                g_bSendScanCodes = TRUE;
                break;
            case 'k':
                if (iArg < argc - 1)
                {
                    g_bWaitPressed = TRUE;
                    g_bRepeatForever = TRUE;
                    g_strWaitForKeys = argv[iArg + 1];
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'K':
                if (iArg < argc - 1)
                {
                    g_bWaitPressed = FALSE;
                    g_bRepeatForever = TRUE;
                    g_strWaitForKeys = argv[iArg + 1];
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'u':
                if (iArg < argc - 1)
                {
                    g_bWaitBoth = TRUE;
                    g_bRepeatForever = TRUE;
                    g_strWaitForKeys = argv[iArg + 1];
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'P':
                if (iArg < argc - 1)
                {
                    g_iPoll = _ttoi(argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 't':
            case 'T':
                if (iArg < argc - 1)
                {
                    g_iTimeout = _ttoi(argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'n':
            case 'N':
                if (iArg < argc - 1)
                {
                    g_bRenameWindow = TRUE;
                    _tcscpy_s(g_szRename, MAX_PATH, argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'p':
                g_bSendToParent = TRUE;
                break;
            case 'x':
                g_bWaitForWindowNoExist = TRUE;
                break;
            case 'X':
                g_bWaitForWindowDisabled = TRUE;
                break;
            case 'e':
            case 'E':
                g_bSearchEveryWindow = TRUE;
                break;
            case 'j':
                g_bAppIsHWND = TRUE;
                break;
            case 'J':
                g_bReadHWNDFromSTDIN = TRUE;
                break;
            case 'v':
                g_bVerbose = TRUE;
                break;
            case 'V':
                g_bVerboseCounters = TRUE;
                break;
            case 'O':
                g_bFindWindowOnly = TRUE;
                g_bHandleReport = TRUE;
                break;
            case 'I':
                g_bFindWindowOnly = TRUE;
                g_bPrintDialogItemID = TRUE;
                if (iArg < argc - 1)
                {
                    GetDialogIDs(argv[iArg + 1]);
                    iArg++;
                    iChar = _tcslen(argv[iArg]) - 1;
                }
                break;
            case 'o':
                g_bFindWindowOnly = TRUE;
                break;
            case '-':
                ParseFullCommand(&argv[iArg][iChar + 1]);
                iChar = _tcslen(argv[iArg]) - 1;
                break;
            default:
                _ftprintf(stderr, _T("Warning:  Ignoring invalid option \"-%c\"\n"), argv[iArg][iChar]);
            }
            iChar++;
        }
        iArg++;
        if (iArg >= argc)
            break;
    }

    return iArg;
}

std::wstring chomp(LPCTSTR sz)
{
    std::wstring ws = sz;
    if (ws.length() > 0)
    {
        bool bContinue = true;
        while (bContinue)
        {
            switch (ws[ws.length() - 1])
            {
            case L' ':
            case L'\t':
            case L'\r':
            case L'\n':
                ws = ws.substr(0, ws.length() - 1);
                break;
            default:
                bContinue = false;
                break;
            }
        }
    }
    return ws;
}

enum { ParamAppName, ParamKeys, ParamTime, ParamEnd };

int _tmain(int argc, TCHAR * argv[])
{
    g_bRegEx = FALSE;
    g_bCaseSensitive = FALSE;
    g_bMatchForegroundWindow = FALSE;
    g_bSent = FALSE;
    g_bWaitDate = FALSE;
    g_bListFailedWindows = FALSE;
    g_iDialogItemID[0] = -1;
    g_bWaitForSuccess = FALSE;
    g_bWaitForEnabled = FALSE;
    g_bRepeatForever = FALSE;
    g_bCheckForeground = FALSE;
    g_bRestoreForeground = TRUE;
    g_bIncludeHidden = FALSE;
    g_bSendInputAsWM = FALSE;
    g_bRenameWindow = FALSE;
    g_bFullHelp = FALSE;
    g_bQuiet = FALSE;
    g_bMatchClass = FALSE;
    g_bSendToEveryWindow = FALSE;
    g_bSendScanCodes = FALSE;
    g_bWaitForWindowNoExist = FALSE;
    g_bSendToParent = FALSE;
    g_bWaitForWindowDisabled = FALSE;
    g_bWaitPressed = FALSE;
    g_bWaitBoth = FALSE;
    g_strWaitForKeys = _T("");
    g_bSearchEveryWindow = FALSE;
    g_bFindWindowOnly = FALSE;
    g_bPrintDialogItemID = FALSE;
    g_bHandleReport = FALSE;
    g_bPrintedSingleHandle = FALSE;
    g_bAppIsHWND = FALSE;
    g_bReadHWNDFromSTDIN = FALSE;
    g_bVerbose = FALSE;
    g_bVerboseCounters = FALSE;
    g_iPoll = SLEEPTIME;
    g_iTimeout = 0;
    g_iRepeatAll = 1;
    g_iEventDelay = 0;
    g_iMouseDelay = 0;

    GetCursorPos(&g_mouseOrigin);

    BOOL bContinue = TRUE;
    int iArg = ParseArgs(argc, argv);

    if (g_bFullHelp)
    {
        return 0;
    }

    CSendInfo sendInfo;
    sendInfo.m_strApp = _T("");
    sendInfo.m_strKeys = _T("");
    sendInfo.m_strVariable = _T("");
    sendInfo.m_bTopLevel = true;

    int iState = ParamAppName;

    if (g_bReadHWNDFromSTDIN)
    {
        if (g_bVerbose)
            _tprintf(_T("Reading window handle from stdin: "));
        TCHAR sz[MAX_PATH];
        _getts_s(sz, MAX_PATH);
        if (g_bVerbose)
            _tprintf(_T("%s\n"), sz);
        sendInfo.m_strVariable = chomp(sz);
    }

    if (g_bMatchForegroundWindow)
        iState++;   // No app name if this option is present

    while (iState < ParamEnd)
    {
        if (argc > iArg)
        {
            switch (iState)
            {
            case ParamAppName:
                sendInfo.m_strApp = ParseString(argv[iArg], &sendInfo);
                break;
            case ParamKeys:
                sendInfo.m_strKeys = ParseString(argv[iArg], &sendInfo);
                break;
            case ParamTime:
                // User provided a time for processing
                g_bWaitDate = TRUE;
                if (FAILED(VarDateFromStr(ParseString(argv[iArg], &sendInfo).c_str(), LANG_USER_DEFAULT, 0, &g_dtWait)))
                {
                    _ftprintf(stderr, _T("Error:  Could not parse date/time: \"%s\"\n"), argv[iArg]);
                    bContinue = FALSE;
                }

                SYSTEMTIME stNow;
                double dtNow;
                double dtDiff;

                GetLocalTime(&stNow);
                SystemTimeToVariantTime(&stNow, &dtNow);

                if (g_dtWait < 1.0)
                {
                    // No day provided; use today
                    g_dtWait += floor(dtNow);
                }

                if (g_dtWait < dtNow)
                {
                    // Too early for today; use tomorrow
                    g_dtWait += 1.0;
                }

                // Wait until it's time to send the keys
                if (bContinue)
                {
                    while (dtNow < g_dtWait)
                    {
                        GetLocalTime(&stNow);
                        SystemTimeToVariantTime(&stNow, &dtNow);
                        dtDiff = (g_dtWait - dtNow) * (24 * 60);

                        _tprintf(_T("Waiting for %.0f min, %.0f sec       \r"), floor(dtDiff), (dtDiff - floor(dtDiff)) * 60);

                        Sleep(1000);
                    }
                    _tprintf(_T("\n"));
                }
                break;
            default:
                _ftprintf(stderr, _T("Warning:  Ignoring extra argument: %s"), argv[iArg]);
                break;
            }
        }
        iState++;
        iArg++;
    }

    if (sendInfo.m_strKeys == _T("") && (!g_bListFailedWindows) && (!g_bMatchForegroundWindow) && (!g_bWaitForSuccess) && (!g_bWaitForWindowNoExist) && (!g_bWaitForWindowDisabled) && (!g_bWaitForEnabled) && (!g_bFindWindowOnly))
    {
        _ftprintf(stderr, _T("Error:  Provide at least an application name and keystroke string\n\n"));
        Usage();
        return -1;
    }

    if (!bContinue)
        return -1;

    _stprintf_s(g_szEXEString, MAX_PATH, _T(" - %s"), argv[0]);

    if (g_bRepeatForever)
    {
        if (g_iTimeout > 0)
            _tprintf(_T("Sending keystrokes for %d ms...press Ctrl+Alt+Shift+X to terminate.\n"), g_iTimeout);
        else
            _tprintf(_T("Sending keystrokes repeatedly...press Ctrl+Alt+Shift+X to terminate.\n"));
    }

    if (g_bHandleReport)
    {
        _tprintf(_T("HWND\tID\tWindowRect\tSize\tClassName\tText\n"));
    }

    HWND hwndSendToThis = NULL;
    std::wstring wsHWNDName;
    if (g_bAppIsHWND)
    {
        hwndSendToThis = (HWND)StringToInteger(sendInfo.m_strApp);
        TCHAR sz[64];
        _stprintf_s(sz, 64, _T("[HWND:%8.8x]"), (DWORD)hwndSendToThis);
        wsHWNDName = sz;
        if (!IsWindow(hwndSendToThis))
        {
            _tprintf(_T("Could not find window matching HWND: %s\n"), sendInfo.m_strApp.c_str());
            return -1;
        }
        if (!g_bIncludeHidden && !IsWindowVisible(hwndSendToThis) && !g_bWaitForSuccess)
        {
            _tprintf(_T("%s is not visible.  Use -h to send to this window.\n"), sz);
            return -1;
        }
    }

    if (g_bWaitForWindowNoExist)
    {
        do
        {
            g_bSent = FALSE;
            CWaitKey::WaitForKeys(g_strWaitForKeys, g_bWaitPressed, g_bWaitBoth, g_iPoll);
            if (hwndSendToThis == NULL)
                EnumWindows(EnumWindowsProc, (LPARAM)&sendInfo);
            else
            {
                if (IsWindow(hwndSendToThis) && (g_bIncludeHidden || IsWindowVisible(hwndSendToThis)))
                    SendToWindow(hwndSendToThis, &sendInfo, wsHWNDName.c_str());
            }
        } while (g_bWaitForWindowNoExist && g_bSent);
    }
    else if (g_bWaitForWindowDisabled || g_bWaitForEnabled)
    {
        g_bEnabledDisabledFlag = TRUE;

        while (g_bEnabledDisabledFlag)
        {
            CWaitKey::WaitForKeys(g_strWaitForKeys, g_bWaitPressed, g_bWaitBoth, g_iPoll);
            if (hwndSendToThis == NULL)
                EnumWindows(EnumWindowsProc, (LPARAM)&sendInfo);
            else
            {
                if (IsWindow(hwndSendToThis) && (g_bIncludeHidden || IsWindowVisible(hwndSendToThis)))
                    if (SendToWindow(hwndSendToThis, &sendInfo, wsHWNDName.c_str()) != TRUE)
                        break;
            }
        }
    }
    else
    {
        if (g_bWaitBoth)
            CWaitKey::WaitForKeys(g_strWaitForKeys, g_bWaitPressed, g_bWaitBoth, g_iPoll);

        if (g_strWaitForKeys.length() == 0 || g_bWaitBoth || CWaitKey::CheckKeys(g_strWaitForKeys, g_bWaitPressed))
        {
            if (hwndSendToThis == NULL)
                EnumWindows(EnumWindowsProc, (LPARAM)&sendInfo);
            else
            {
                if (IsWindow(hwndSendToThis) && (g_bIncludeHidden || IsWindowVisible(hwndSendToThis)))
                    SendToWindow(hwndSendToThis, &sendInfo, wsHWNDName.c_str());
            }
        }
    }

    Sleep(g_iPoll);
    __int64 iStart = GetTickCount();

    while ((!g_bSent && g_bWaitForSuccess) || g_bRepeatForever)
    {
        g_bSent = FALSE;
        CWaitKey::WaitForKeys(g_strWaitForKeys, g_bWaitPressed, g_bWaitBoth, g_iPoll);
        if (hwndSendToThis == NULL)
            EnumWindows(EnumWindowsProc, (LPARAM)&sendInfo);
        else
        {
            if (IsWindow(hwndSendToThis) && (g_bIncludeHidden || IsWindowVisible(hwndSendToThis)))
                SendToWindow(hwndSendToThis, &sendInfo, wsHWNDName.c_str());
        }

        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))
            if ((GetAsyncKeyState(VK_MENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))
                if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
                    if (GetAsyncKeyState('X') & 0x8000)
                        break;

        if (g_iTimeout > 0)
        {
            __int64 iNow = GetTickCount();
            if (iNow < iStart)
                iNow += 4294967296;

            if (iNow >(iStart + g_iTimeout))
            {
                _tprintf(_T("Timed out after %d milliseconds.\n"), g_iTimeout);
                break;
            }
        }
    }

    if (!g_bSent && (argv[iArg] != NULL) && !g_bWaitForWindowNoExist && !g_bMatchForegroundWindow && !g_bFindWindowOnly)
    {
        if (_tcslen(g_szMatchClass) == 0)
            _tprintf(_T("Could not find window matching \"%s\"\n"), sendInfo.m_strApp.c_str());
        else
            _tprintf(_T("Could not find windows matching \"%s\" with class name \"%s\"\n"), sendInfo.m_strApp.c_str(), g_szMatchClass);
    }

    if (g_bListFailedWindows)
    {
        POINT pt;
        GetCursorPos(&pt);
        POINT ptLocal = pt;
        ScreenToClient(GetForegroundWindow(), &ptLocal);
        _tprintf(_T("Mouse cursor is at %d,%d (global), %d,%d (local)\n"), pt.x, pt.y, ptLocal.x, ptLocal.y);
    }

    return 0;
}