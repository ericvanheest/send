#include "stdafx.h"
#include "WaitKey.h"

CWaitKey::CWaitKey(void)
{
}

CWaitKey::~CWaitKey(void)
{
}

int CWaitKey::HexCharToInt(TCHAR c)
{
    if (c >= _T('0') && c <= _T('9'))
        return (c - _T('0'));
    return tolower(c) - _T('a') + 10;
}

short CWaitKey::NumpadVKEY(TCHAR c)
{
    if (c >= _T('0') && c <= _T('9'))
        return VK_NUMPAD0 + HexCharToInt(c);
    switch(c)
    {
        case _T('/'):  return VK_DIVIDE;
        case _T('*'):  return VK_MULTIPLY;
        case _T('-'):  return VK_SUBTRACT;
        case _T('+'):  return VK_ADD;
        case _T('.'):  return VK_DECIMAL;
        case _T('n'):  return VK_SEPARATOR;
    }

    return 0;
}

bool CWaitKey::CheckKeys(TString strKeys, BOOL bPressed)
{
    bool bResult = true;

    short vkAny = 0;
    size_t iKey = 0;
    while (iKey < strKeys.length())
    {
        TCHAR c = strKeys.at(iKey);

        if (isalpha(c))
            c = toupper(c);

        short vk = c;
        if (c == '\\')
        {
            iKey++;
            c = strKeys.at(iKey);
            switch(c)
            {
                case _T('c'):
                case _T('C'):
                    vk = VK_LCONTROL;
                    break;
                case _T('a'):
                case _T('A'):
                    vk = VK_LMENU;
                    break;
                case _T('s'):
                case _T('S'):
                    vk = VK_LSHIFT;
                    break;
                case _T('t'):
                case _T('T'):
                    vk = VK_TAB;
                    break;
                case _T('e'):
                case _T('E'):
                    vk = VK_ESCAPE;
                    break;
                case _T('n'):
                case _T('N'):
                    vk = VK_RETURN;
                    break;
                case _T('l'):
                case _T('L'):
                    vk = VK_LEFT;
                    break;
                case _T('r'):
                case _T('R'):
                    vk = VK_RIGHT;
                    break;
                case _T('u'):
                case _T('U'):
                    vk = VK_UP;
                    break;
                case _T('d'):
                case _T('D'):
                    vk = VK_DOWN;
                    break;
                case _T('w'):
                case _T('W'):
                    vk = VK_LWIN;
                    break;
                case _T('F'):
                case _T('f'):
                    vk = VK_F1 + HexCharToInt(strKeys.at(iKey+1)) - 1;
                    iKey++;
                    break;
                case _T('P'):
                case _T('p'):
                    vk = NumpadVKEY(strKeys.at(iKey+1));
                    iKey++;
                    break;
                case _T('v'):
                case _T('V'):
                    if (strKeys.length() < iKey + 3)
                    {
                        vk = c;
                        break;
                    }
                    vk = HexCharToInt(strKeys.at(iKey+1)) << 4 | HexCharToInt(strKeys.at(iKey+2));
                    iKey += 2;
                    break;
                default:
                    vk = c;
            }
        }

        iKey++;

        if (vkAny != 0)
            vk = vkAny;

        unsigned short iKeyDown = GetAsyncKeyState(vk);

        switch(vk)
        {
            case VK_LCONTROL:
                iKeyDown |= GetAsyncKeyState(VK_RCONTROL);
                break;
            case VK_LMENU:
                iKeyDown |= GetAsyncKeyState(VK_RMENU);
                break;
            case VK_LSHIFT:
                iKeyDown |= GetAsyncKeyState(VK_RSHIFT);
                break;
            case VK_LWIN:
                iKeyDown |= GetAsyncKeyState(VK_RWIN);
                break;
        }

        if (!bPressed && (iKeyDown > 0x7fff))
            bResult = false;
        else if (bPressed && (iKeyDown < 0x8000))
            bResult = false;
    }

    while (_kbhit())
    {
        // eat keys
        _getch();
    }

    return bResult;
}

void CWaitKey::WaitForKeys(TString strKeys, BOOL bPressed, BOOL bBoth, int iPoll)
{
    if (strKeys.length() == 0)
        return;

    if (bPressed || bBoth)
    {
        while(!CWaitKey::CheckKeys(g_strWaitForKeys, TRUE))
            Sleep(iPoll);
    }
    if (!bPressed && bBoth)
    {
        while (!CWaitKey::CheckKeys(g_strWaitForKeys, FALSE))
            Sleep(iPoll);
    }
}



