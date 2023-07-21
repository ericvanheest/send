#pragma once

#include <conio.h>
#include "globals.h"

class CWaitKey
{
public:
    CWaitKey(void);
    ~CWaitKey(void);
    static int HexCharToInt(TCHAR c);
    static bool CheckKeys(TString strKeys, BOOL bPressed);
    static short NumpadVKEY(TCHAR c);
    static void WaitForKeys(TString strKeys, BOOL bPressed, BOOL bBoth, int iPoll);
};
