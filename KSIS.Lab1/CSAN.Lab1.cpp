// KSIS.Lab1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "mpr.lib")

BOOL WINAPI EnumerateFunc(LPNETRESOURCE lpnr);
void DisplayStruct(int i, LPNETRESOURCE lpnrLocal);
void PrintMACaddress(byte* address);
void GetMACaddress(void);


void PrintMACaddress(byte* address){
	printf("> Successfully received Local MAC Address : %02x:%02x:%02x:%02x:%02x:%02x\n",
		(unsigned char)address[0],
		(unsigned char)address[1],
		(unsigned char)address[2],
		(unsigned char)address[3],
		(unsigned char)address[4],
		(unsigned char)address[5]);
}

// Fetches the MAC address and prints it
void GetMACaddress(void)
{
	IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information 
	// for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
		AdapterInfo,                 // [out] buffer to receive data
		&dwBufLen);                  // [in] size of receive data buffer
	assert(dwStatus == ERROR_SUCCESS);  // Verify return value is 
	// valid, no buffer overflow

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to
	// current adapter info
	do {
		PrintMACaddress(pAdapterInfo->Address); // Print MAC address
		pAdapterInfo = pAdapterInfo->Next;    // Progress through 
		// linked list
	} while (pAdapterInfo);                    // Terminate if last adapter
}

BOOL WINAPI EnumerateFunc(LPNETRESOURCE lpnr)
{
	DWORD dwResult, dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;     // 16K is a good size
	DWORD cEntries = -1;        // enumerate all possible entries
	LPNETRESOURCE lpnrLocal;    // pointer to enumerated structures
	DWORD i;
	//
	// Call the WNetOpenEnum function to begin the enumeration.
	//
	dwResult = WNetOpenEnum(RESOURCE_GLOBALNET, // all network resources
		RESOURCETYPE_ANY,   // all resources
		0,  // enumerate all resources
		lpnr,       // NULL first time the function is called
		&hEnum);    // handle to the resource

	if (dwResult != NO_ERROR) {
		printf("WnetOpenEnum failed with error %d\n", dwResult);
		return FALSE;
	}
	//
	// Call the GlobalAlloc function to allocate resources.
	//
	lpnrLocal = (LPNETRESOURCE)GlobalAlloc(GPTR, cbBuffer);
	if (lpnrLocal == NULL) {
		printf("WnetOpenEnum failed with error %d\n", dwResult);
		return FALSE;
	}

	do {
		//
		// Initialize the buffer.
		//
		ZeroMemory(lpnrLocal, cbBuffer);
		//
		// Call the WNetEnumResource function to continue
		//  the enumeration.
		//
		dwResultEnum = WNetEnumResource(hEnum,  // resource handle
			&cEntries,      // defined locally as -1
			lpnrLocal,      // LPNETRESOURCE
			&cbBuffer);     // buffer size
		//
		// If the call succeeds, loop through the structures.
		//
		if (dwResultEnum == NO_ERROR) {
			for (i = 0; i < cEntries; i++) {
				// Call an application-defined function to
				//  display the contents of the NETRESOURCE structures.
				//
				DisplayStruct(i, &lpnrLocal[i]);

				// If the NETRESOURCE structure represents a container resource, 
				//  call the EnumerateFunc function recursively.

				if (RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage
					& RESOURCEUSAGE_CONTAINER))
					if (!EnumerateFunc(&lpnrLocal[i]))
						printf("EnumerateFunc returned FALSE\n");
			}
		}
		// Process errors.
		//
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS) {
			printf("WNetEnumResource failed with error %d\n", dwResultEnum);
			break;
		}
	}
	//
	// End do.
	//
	while (dwResultEnum != ERROR_NO_MORE_ITEMS);
	//
	// Call the GlobalFree function to free the memory.
	//
	GlobalFree((HGLOBAL)lpnrLocal);
	//
	// Call WNetCloseEnum to end the enumeration.
	//
	dwResult = WNetCloseEnum(hEnum);

	if (dwResult != NO_ERROR) {
		//
		// Process errors.
		//
		printf("WNetCloseEnum failed with error %d\n", dwResult);
		//    NetErrorHandler(hwnd, dwResult, (LPSTR)"WNetCloseEnum");
		return FALSE;
	}

	return TRUE;
}

void DisplayStruct(int i, LPNETRESOURCE lpnrLocal)
{
	printf("NETRESOURCE[%d] Scope: ", i);
	switch (lpnrLocal->dwScope) {
	case (RESOURCE_CONNECTED) :
		printf("connected\n");
		break;
	case (RESOURCE_GLOBALNET) :
		printf("all resources\n");
		break;
	case (RESOURCE_REMEMBERED) :
		printf("remembered\n");
		break;
	default:
		printf("unknown scope %d\n", lpnrLocal->dwScope);
		break;
	}

	printf("NETRESOURCE[%d] Type: ", i);
	switch (lpnrLocal->dwType) {
	case (RESOURCETYPE_ANY) :
		printf("any\n");
		break;
	case (RESOURCETYPE_DISK) :
		printf("disk\n");
		break;
	case (RESOURCETYPE_PRINT) :
		printf("print\n");
		break;
	default:
		printf("unknown type %d\n", lpnrLocal->dwType);
		break;
	}

	printf("NETRESOURCE[%d] DisplayType: ", i);
	switch (lpnrLocal->dwDisplayType) {
	case (RESOURCEDISPLAYTYPE_GENERIC) :
		printf("generic\n");
		break;
	case (RESOURCEDISPLAYTYPE_DOMAIN) :
		printf("domain\n");
		break;
	case (RESOURCEDISPLAYTYPE_SERVER) :
		printf("server\n");
		break;
	case (RESOURCEDISPLAYTYPE_SHARE) :
		printf("share\n");
		break;
	case (RESOURCEDISPLAYTYPE_FILE) :
		printf("file\n");
		break;
	case (RESOURCEDISPLAYTYPE_GROUP) :
		printf("group\n");
		break;
	case (RESOURCEDISPLAYTYPE_NETWORK) :
		printf("network\n");
		break;
	default:
		printf("unknown display type %d\n", lpnrLocal->dwDisplayType);
		break;
	}

	printf("NETRESOURCE[%d] Usage: 0x%x = ", i, lpnrLocal->dwUsage);
	if (lpnrLocal->dwUsage & RESOURCEUSAGE_CONNECTABLE)
		printf("connectable ");
	if (lpnrLocal->dwUsage & RESOURCEUSAGE_CONTAINER)
		printf("container ");
	printf("\n");

	printf("NETRESOURCE[%d] Localname: %S\n", i, lpnrLocal->lpLocalName);
	printf("NETRESOURCE[%d] Remotename: %S\n", i, lpnrLocal->lpRemoteName);
	printf("NETRESOURCE[%d] Comment: %S\n", i, lpnrLocal->lpComment);
	printf("NETRESOURCE[%d] Provider: %S\n", i, lpnrLocal->lpProvider);
	printf("\n");
}





int _tmain(int argc, _TCHAR* argv[])
{
	GetMACaddress();
	LPNETRESOURCE lpnr = NULL;

	if (EnumerateFunc(lpnr) == FALSE) {
		printf("Call to EnumerateFunc failed\n");
	}
	system("pause");
	return 0;

}

