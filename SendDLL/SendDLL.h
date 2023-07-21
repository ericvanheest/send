// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SENDDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SENDDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SENDDLL_EXPORTS
#define SENDDLL_API __declspec(dllexport)
#else
#define SENDDLL_API __declspec(dllimport)
#endif

// This class is exported from the SendDLL.dll
class SENDDLL_API CSendDLL {
public:
	CSendDLL(void);
	// TODO: add your methods here.
};

extern SENDDLL_API int nSendDLL;

SENDDLL_API int fnSendDLL(void);

SENDDLL_API void CallSend(int argc, LPCSTR * argv);
