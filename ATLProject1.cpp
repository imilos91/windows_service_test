// ATLProject1.cpp : Implementation of WinMain
// Author: Milos Ilic <ilkesd91@gmail.com>
// Date: 10.07.2021.


#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "ATLProject1_i.h"


using namespace ATL;
using namespace std;

#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <psapi.h>

#include "Microphone.h"
#include "Users.h"
#include "FocusApp.h"

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

#define INFO_BUFFER_SIZE 32767
TCHAR  name[INFO_BUFFER_SIZE];
DWORD  bufCharCount = INFO_BUFFER_SIZE;

void microphoneThread() {
	ofstream out;
	
	while (true) {
		out.open("c:\\log-camera.txt");

		if (IsMicrophoneRecording()) {
			out << "active microphone: yes" << '\n';
			out.flush();
		}
		else {
			out << "active microphone: no" << '\n';
			out.flush();
		}

		out.close();

		Sleep(1000);
	}
}

void usersThread() {
	GetComputerName(name, &bufCharCount);

	while (true) {
		DisplayLocalLogons(name);

		DisplaySessionLogons(name);

		Sleep(1000);
	}
}

void focusAppThread() {
	while (true) {
		getActiveApp();

		Sleep(1000);
	}
}

//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
								LPTSTR /*lpCmdLine*/, int nShowCmd)
{
	thread t1(microphoneThread);
	thread t2(usersThread);
	thread t3(focusAppThread);

	t3.join();
	t2.join();
	t1.join();


	return _AtlModule.WinMain(nShowCmd);
}

