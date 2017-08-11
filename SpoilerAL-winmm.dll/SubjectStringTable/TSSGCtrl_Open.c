#include <windows.h>
#define USING_NAMESPACE_BCB6_STD
#include "bcb6_std_string.h"
#include "TProcessCtrl.h"
#include "TSSGActionListner.h"

BOOLEAN __fastcall TSSGCtrl_IsRemoteProcess(LPCSTR p);
string * __fastcall SubjectStringTable_GetString(string *s);

enum ssgCtrl {
	stNONE = 0,
	stDIR,
	stCALC,
	stTOGGLE,
	stLIST,
	stSTRING,
	stBITLIST,
	stB_TOGGLE,
	stB_CALC,
	stB_LIST,
	stD_LIST,
	stTRACE,
	stD_TOGGLE,
	stCOPY,
	stFLOAT_CALC,
	stB_FLOAT_CALC,
	stSPLIT,
};

static __inline const string *GetAddressStrPointer(TSSGSubject *SSGS)
{
	string *s;

	switch (SSGS->type)
	{
	case stCALC:
	case stLIST:
	case stB_CALC:
	case stB_LIST:
		s = (string *)((LPBYTE)SSGS + 136);
		break;
	case stTOGGLE:
	case stB_TOGGLE:
		s = (string *)((LPBYTE)SSGS + 120);
		break;
	case stSTRING:
	case stCOPY:
	case stBITLIST:
		s = (string *)((LPBYTE)SSGS + 128);
		break;
	case stD_LIST:
	case stD_TOGGLE:
	case stFLOAT_CALC:
	case stB_FLOAT_CALC:
		s = (string *)((LPBYTE)SSGS + 152);
		break;
	case stTRACE:
		s = (string *)((LPBYTE)SSGS + 96);
		break;
	default:
		return NULL;
	}
	return SubjectStringTable_GetString(s);
}

HANDLE __cdecl TSSGCtrl_Open(TSSGCtrl *_this, TSSGSubject *SSGS, DWORD Mode)
{
	HANDLE       hProcess;
	const string *addressStr;

	addressStr = GetAddressStrPointer(SSGS);
	hProcess = (!addressStr || TSSGCtrl_IsRemoteProcess(addressStr->_M_start)) ?
		TProcessCtrl_Open(&_this->processCtrl, Mode) :
		GetCurrentProcess();
	if (_this->ssgActionListner)
		if (hProcess)
			TSSGActionListner_OnProcessOpen(_this->ssgActionListner, SSGS, Mode);
		else
			TSSGActionListner_OnProcessOpenError(_this->ssgActionListner, SSGS);
	return hProcess;
}
