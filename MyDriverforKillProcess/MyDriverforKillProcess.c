#include <stdio.h>
#include<ntifs.h>


#define DEVICE_SEND CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_WRITE_DATA)
#define DEVICE_REC CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_READ_DATA)


UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Mydevice123");
PDEVICE_OBJECT DeviceObject = NULL;
UNICODE_STRING SymLinkName = RTL_CONSTANT_STRING(L"\\??\\MydeviceLink123");




//����ж��
VOID DriverUnload(PDRIVER_OBJECT DriverObject) {

	IoDeleteSymbolicLink(&SymLinkName);
	IoDeleteDevice(DeviceObject);
	DbgPrint("MyDriver is unloading\\n");
}
//Irp������
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
		// ��wchar_t����ת��ΪUnicode�ַ���
		RtlInitUnicodeString(&unicodeString, buffer);

		DbgPrint("data form app is : %ws", buffer);
		returnLength = (wcsnlen(buffer, 511) + 1) * 2;
		// ��Unicode�ַ���ת��Ϊ����
		if (NT_SUCCESS(RtlUnicodeStringToInteger(&unicodeString, 10, &intPID))) {
			DbgPrint("after convert: %d\n", intPID);
		}
		else {
			// ת��ʧ�ܣ��������
			DbgPrint("can not convert\n");
		}



		// �������̾��
		HANDLE hProcess;
		OBJECT_ATTRIBUTES objAttr;
		CLIENT_ID clientId;
		NTSTATUS status;

		// ���ý���ID
		clientId.UniqueProcess = (HANDLE)intPID;

		InitializeObjectAttributes(&objAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

		// �򿪽���
		status = ZwOpenProcess(&hProcess, GENERIC_ALL, &objAttr, &clientId);
		if (!NT_SUCCESS(status)) {
			DbgPrint("can not open processhandle: 0x%X\n", status);
			return status;
		}

		// ��ֹ����
		status = ZwTerminateProcess(hProcess, STATUS_SUCCESS);
		if (!NT_SUCCESS(status)) {
			DbgPrint("can not terminate process : 0x%X\n", status);
			ZwClose(hProcess); // �رվ��
			return status;
		}

		// �رս��̾��
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

//�������
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegPath) {

	NTSTATUS status;
	DriverObject->DriverUnload = DriverUnload;  

	//����һ��DeviceObject
	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Creating DeviceObject Failed\r\n");
		return status;
	}
	//Ӧ�ó�����ʲ���DeviceObject����Ҫһ��SymlicLink
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
