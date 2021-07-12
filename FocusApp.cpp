#include "FocusApp.h"
#include "pch.h"

#include <iostream>
#include <fstream>
#include <psapi.h>

using namespace std;

void getActiveApp() {
	ofstream out;
	out.open("c:\\log-app.txt");
	HWND foreground = GetForegroundWindow();
	DWORD pid = 0;

	DWORD tid = GetWindowThreadProcessId(foreground, &pid);

	HANDLE Handle = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
		pid
	);
	if (Handle)
	{
		TCHAR app[MAX_PATH];
		if (GetModuleFileNameEx(Handle, 0, app, MAX_PATH))
		{
			wprintf(L"App: %s\n", app);
			out << app << endl;
			out.flush();
			out.close();
		}
		CloseHandle(Handle);
	}
}