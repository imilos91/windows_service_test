// ATLProject1.cpp : Implementation of WinMain


#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "ATLProject1_i.h"


using namespace ATL;

#include <stdio.h>

class CATLProject1Module : public ATL::CAtlServiceModuleT< CATLProject1Module, IDS_SERVICENAME >
{
public :
	DECLARE_LIBID(LIBID_ATLProject1Lib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ATLPROJECT1, "{650fcc14-8dde-4036-b10f-ef457f572296}")
	HRESULT InitializeSecurity() throw()
	{
		// TODO : Call CoInitializeSecurity and provide the appropriate security settings for your service
		// Suggested - PKT Level Authentication,
		// Impersonation Level of RPC_C_IMP_LEVEL_IDENTIFY
		// and an appropriate non-null Security Descriptor.

		return S_OK;
	}
};

CATLProject1Module _AtlModule;



//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
								LPTSTR /*lpCmdLine*/, int nShowCmd)
{
	return _AtlModule.WinMain(nShowCmd);
}

