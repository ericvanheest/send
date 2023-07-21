// SendDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "SendDLL.h"

extern int main(int argc, LPCSTR argv[]);

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

// This is an example of an exported variable
SENDDLL_API int nSendDLL=0;

// This is an example of an exported function.
SENDDLL_API int fnSendDLL(void)
{
	return 42;
}

SENDDLL_API void CallSend(int argc, LPCSTR argv[])
{
    main(argc, argv);
}


// This is the constructor of a class that has been exported.
// see SendDLL.h for the class definition
CSendDLL::CSendDLL()
{ 
	return; 
}
