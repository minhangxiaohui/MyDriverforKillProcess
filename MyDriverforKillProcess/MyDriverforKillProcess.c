#include <stdio.h>
#include<ntifs.h>


#define DEVICE_SEND CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_WRITE_DATA)
#define DEVICE_REC CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_READ_DATA)


UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Mydevice123");
PDEVICE_OBJECT DeviceObject = NULL;
UNICODE_STRING SymLinkName = RTL_CONSTANT_STRING(L"\\??\\MydeviceLink123");




//驱动卸载
VOID DriverUnload(PDRIVER_OBJECT DriverObject) {

	IoDeleteSymbolicLink(&SymLinkName);
	IoDeleteDevice(DeviceObject);
	DbgPrint("MyDriver is unloading\\n");
}
//Irp处理函数
NTSTATUS DispathPassRThru(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status= STATUS_SUCCESS;
	switch (irpsp->MajorFunction) {
	case IRP_MJ_CREATE:
		DbgPrint("receive create rquest ");
		break;
	case IRP_MJ_READ:
		DbgPrint("receive read rquest ");
		break;
	//case IRP_MJ_CLOSE:
	//	DbgPrint("receive close rquest ");
	//	break;
	default:
		status = STATUS_INVALID_PARAMETER;
		break;
	}
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DispatchDevcCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	ULONG returnLength = 0;

	PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
	ULONG inLength = irpsp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG OutLength = irpsp->Parameters.DeviceIoControl.OutputBufferLength;
	WCHAR* demo = L"driver's sample data";

	int intPID;
	PUNICODE_STRING unicodeString;
	switch (irpsp->Parameters.DeviceIoControl.IoControlCode)
	{

	case DEVICE_SEND:
		// 将wchar_t数组转换为Unicode字符串
		RtlInitUnicodeString(&unicodeString, buffer);

		DbgPrint("data form app is : %ws", buffer);
		returnLength = (wcsnlen(buffer, 511) + 1) * 2;
		// 将Unicode字符串转换为整数
		if (NT_SUCCESS(RtlUnicodeStringToInteger(&unicodeString, 10, &intPID))) {
			DbgPrint("after convert: %d\n", intPID);
		}
		else {
			// 转换失败，处理错误
			DbgPrint("can not convert\n");
		}



		// 创建进程句柄
		HANDLE hProcess;
		OBJECT_ATTRIBUTES objAttr;
		CLIENT_ID clientId;
		NTSTATUS status;

		// 设置进程ID
		clientId.UniqueProcess = (HANDLE)intPID;

		InitializeObjectAttributes(&objAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

		// 打开进程
		status = ZwOpenProcess(&hProcess, GENERIC_ALL, &objAttr, &clientId);
		if (!NT_SUCCESS(status)) {
			DbgPrint("can not open processhandle: 0x%X\n", status);
			return status;
		}

		// 终止进程
		status = ZwTerminateProcess(hProcess, STATUS_SUCCESS);
		if (!NT_SUCCESS(status)) {
			DbgPrint("can not terminate process : 0x%X\n", status);
			ZwClose(hProcess); // 关闭句柄
			return status;
		}

		// 关闭进程句柄
		ZwClose(hProcess);
		DbgPrint("terminate successfued : 0x%X \n", status);

		break;
	case DEVICE_REC:
		wcsncpy(buffer, demo, 511);
		returnLength = (wcsnlen(buffer, 511) + 1) * 2;
		//DbgPrint("data form driver is : %ws", buffer);
		break;
	default:
		status = STATUS_INVALID_PARAMETER;


	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = returnLength;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);


}

//驱动入口
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegPath) {

	NTSTATUS status;
	DriverObject->DriverUnload = DriverUnload;  

	//创建一个DeviceObject
	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Creating DeviceObject Failed\r\n");
		return status;
	}
	//应用程序访问不到DeviceObject，需要一个SymlicLink
	status = IoCreateSymbolicLink(&SymLinkName, &DeviceName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Creating SymbolicLink False\r\n");
		IoDeleteDevice(DeviceObject);
		return status;
	}


	int i;
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DispathPassRThru;
		//DriverObject->MajorFunction[IRP_MJ_READ] = ReadFunc;
		//DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseFunc;
	}


	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDevcCTL;

	DbgPrint("Initialize Success\\n");

	return  status;
}
