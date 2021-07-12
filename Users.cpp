#include "Users.h"
#include "pch.h"

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef UNICODE  
#define UNICODE  
#endif  

#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <lm.h>
#include <wtsapi32.h>
#include <userenv.h>

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv.lib")  

//  
// Length of name strings  
//  
#define MAX_NAME_STRING   1024  


//--------------------------------------------------------------------  
//  
// DisplayLocalLogons  
//   
// Scans the HKEY_USERS key of the specified computer to see who  
// has their profile loaded.  
//  
//--------------------------------------------------------------------  
VOID DisplayLocalLogons(LPTSTR ServerName)
{
	BOOLEAN     first = TRUE;
	TCHAR       errorMessage[1024];
	TCHAR       userName[MAX_NAME_STRING], domainName[MAX_NAME_STRING];
	TCHAR       subKeyName[MAX_PATH];
	DWORD       subKeyNameSize, index;
	DWORD       userNameSize, domainNameSize;
	FILETIME    lastWriteTime;
	HKEY        usersKey;
	PSID        sid;
	SID_NAME_USE sidType;
	SID_IDENTIFIER_AUTHORITY authority;
	BYTE        subAuthorityCount;
	DWORD       authorityVal, revision;
	DWORD       subAuthorityVal[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	//  
	// Use RegConnectRegistry so that we work with remote computers  
	//  
	if (ServerName) {

		wprintf(L"Connecting to Registry of %s...", ServerName);
		fflush(stdout);

		if (RegConnectRegistry(ServerName, HKEY_USERS, &usersKey) != ERROR_SUCCESS) {

			wprintf(L"\r                                                      \r");
			wprintf(L"Error opening HKEY_USERS for %s\n", ServerName);
			return;
		}
		wprintf(L"\r                                                      \r");

	}
	else {

		if (RegOpenKey(HKEY_USERS, NULL, &usersKey) != ERROR_SUCCESS) {

			wprintf(errorMessage, L"Error opening HKEY_USERS");
			return;
		}
	}

	//  
	// Enumerate keys under HKEY_USERS  
	//  
	index = 0;
	subKeyNameSize = sizeof(subKeyName);
	while (RegEnumKeyEx(usersKey, index, subKeyName, &subKeyNameSize,
		NULL, NULL, NULL, &lastWriteTime) == ERROR_SUCCESS) {

		//  
		// Ignore the default subkey and win2K user class subkeys  
		//  
		if (_wcsicmp(subKeyName, L".default") &&
			!wcsstr(subKeyName, L"Classes")) {

			//  
			// Convert the textual SID into a binary SID  
			//  
			subAuthorityCount = swscanf_s(subKeyName, L"S-%d-%x-%lu-%lu-%lu-%lu-%lu-%lu-%lu-%lu",
				&revision, &authorityVal,
				&subAuthorityVal[0],
				&subAuthorityVal[1],
				&subAuthorityVal[2],
				&subAuthorityVal[3],
				&subAuthorityVal[4],
				&subAuthorityVal[5],
				&subAuthorityVal[6],
				&subAuthorityVal[7]);

			if (subAuthorityCount >= 3) {

				subAuthorityCount -= 2;

				//  
				// Note: we can only deal with authority values  
				// of 4 bytes in length  
				//  
				authority.Value[5] = *(PBYTE)& authorityVal;
				authority.Value[4] = *((PBYTE)& authorityVal + 1);
				authority.Value[3] = *((PBYTE)& authorityVal + 2);
				authority.Value[2] = *((PBYTE)& authorityVal + 3);
				authority.Value[1] = 0;
				authority.Value[0] = 0;

				//  
				// Initialize variables for subsequent operations  
				//  
				sid = NULL;
				userNameSize = MAX_NAME_STRING;
				domainNameSize = MAX_NAME_STRING;

				if (AllocateAndInitializeSid(&authority,
					subAuthorityCount,
					subAuthorityVal[0],
					subAuthorityVal[1],
					subAuthorityVal[2],
					subAuthorityVal[3],
					subAuthorityVal[4],
					subAuthorityVal[5],
					subAuthorityVal[6],
					subAuthorityVal[7],
					&sid)) {

					//  
					// We can finally lookup the account name  
					//  
					if (LookupAccountSid(ServerName,
						sid,
						userName,
						&userNameSize,
						domainName,
						&domainNameSize,
						&sidType)) {

						//  
						// We've successfully looked up the user name  
						//  
						if (first) {

							wprintf(L"Users logged on locally:\n");
							first = FALSE;
						}
						wprintf(L"     %s\\%s\n", domainName, userName);
					}
				}
				if (sid) FreeSid(sid);
			}
		}
		subKeyNameSize = sizeof(subKeyName);
		index++;
	}
	RegCloseKey(usersKey);

	if (first) wprintf(L"No one is logged on locally.\n");
}



//--------------------------------------------------------------------  
//  
// DisplaySessionLogons  
//   
// Uses the NetEnumSession API to see who is logged on via access  
// to a resource share.  
//  
//--------------------------------------------------------------------  
VOID DisplaySessionLogons(LPTSTR ServerName)
{
	LPSESSION_INFO_10 pBuf = NULL;
	LPSESSION_INFO_10 pTmpBuf;
	DWORD        dwLevel = 10;
	DWORD        dwPrefMaxLen = 0xFFFFFFFF;
	DWORD        dwEntriesRead = 0;
	DWORD        dwTotalEntries = 0;
	DWORD        dwResumeHandle = 0;
	DWORD        i;
	DWORD        dwTotalCount = 0;
	LPTSTR       pszClientName = NULL;
	LPTSTR       pszUserName = NULL;
	NET_API_STATUS nStatus;
	PSID         sid;
	DWORD        sidSize, domainNameSize;
	BYTE         sidBuffer[MAX_SID_SIZE];
	TCHAR        domainName[MAX_NAME_STRING];
	SID_NAME_USE sidType;
	BOOLEAN      first = TRUE;

	//  
	// Now display session logons  
	//   
	do
	{
		nStatus = NetSessionEnum(ServerName,
			pszClientName,
			pszUserName,
			dwLevel,
			(LPBYTE*)& pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				for (i = 0; (i < dwEntriesRead); i++)
				{
					assert(pTmpBuf != NULL);

					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}

					//  
					// Take the name and look up a SID so that we can get full domain/user  
					// information  
					//  
					sid = (PSID)sidBuffer;
					sidSize = sizeof(sidBuffer);
					domainNameSize = sizeof(domainName);

					if (pTmpBuf->sesi10_username[0]) {

						if (first) {

							wprintf(L"\nUsers logged on via resource shares:\n");
							first = FALSE;
						}
						if (LookupAccountName(pTmpBuf->sesi10_cname,
							pTmpBuf->sesi10_username,
							sid,
							&sidSize,
							domainName,
							&domainNameSize,
							&sidType)) {

							wprintf(L"     %s\\%s\n", domainName, pTmpBuf->sesi10_username);

						}
						else {

							wprintf(L"      %s\\%s\n", ServerName, pTmpBuf->sesi10_username);
						}
					}

					pTmpBuf++;
					dwTotalCount++;
				}
			}
		}
		else {

			wprintf(L"Unable to query resource logons\n");
			first = FALSE;
		}

		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	} while (nStatus == ERROR_MORE_DATA);

	if (pBuf != NULL)
		NetApiBufferFree(pBuf);
	if (first) wprintf(L"\nNo one is logged on via resource shares.\n");
}