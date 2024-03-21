#include<windows.h>
#include<winioctl.h>
#include <stdio.h>
#include <wchar.h>

#define DEVICE_SEND CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_WRITE_DATA)
#define DEVICE_REC CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_READ_DATA)


int main() {
	HANDLE 	devicehandle = CreateFile("\\\\.\\MydeviceLink123",GENERIC_ALL,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_SYSTEM,0);
	if (devicehandle == INVALID_HANDLE_VALUE) {
		MessageBox(NULL,"not valid value", "Notice", MB_OK);
		return 0;
	}
	MessageBox(NULL, "Create devicehandle successfly", "Notice", MB_OK);

	//Sleep(1000);
	int pid;
	wchar_t wcharArrayPID[20]; // ×ã¹»´æ´¢×ª»»ºóµÄ×Ö·û´®
	while (true) {
		printf("Please enter the PID to terminate: ");
		scanf("%d", &pid);
		swprintf(wcharArrayPID, 20, L"%d", pid);
		printf("int convert to  wchar_t array: %ls\n", wcharArrayPID);

		ULONG returnLength = 0;
		char wr[4] = { 0 };
		if (devicehandle != INVALID_HANDLE_VALUE && devicehandle != NULL) {
			if (!DeviceIoControl(devicehandle, DEVICE_SEND, wcharArrayPID, (wcslen(wcharArrayPID) + 1) * 2, NULL, 0, &returnLength, 0))
			{
				MessageBox(NULL, "send data,DeviceIoControl  error", "Notice", MB_OK);
			}
			else {
				_itoa_s(returnLength, wr, 10);
				MessageBox(NULL, "kill successful","Notice", MB_OK);
			}

		}

		//Sleep(5000);
	}
	//WCHAR recbuffer[1024] = { 0 };
	//ULONG returnLenthrecv = 0;
	//char wrre[4] = { 0 };

	//if (devicehandle != INVALID_HANDLE_VALUE && devicehandle != NULL) {
	//	if (!DeviceIoControl(devicehandle, DEVICE_REC, NULL,0, recbuffer,1024, &returnLenthrecv, 0))
	//	{
	//		MessageBoxA(NULL, "send data,DeviceIoControl  error", 0,0 );
	//	}
	//	else {
	//		MessageBoxW(NULL,recbuffer , 0, 0);
	//		_itoa_s(returnLenthrecv, wrre, 10);
	//		MessageBoxA(NULL,wrre , 0, 0);
	//	}

	//}

	CloseHandle(devicehandle);

}