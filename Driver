#include<wdm.h>
#include<ntddk.h>
#define Device_name L"\\Device\\myfirst"
#define symbol_name L"\\??\\myfirst123"

VOID drvUnload(PDRIVER_OBJECT Driver)
{
	DbgPrint("unloaded\n");
	return 0;
}
NTSTATUS Mycreate(PDEVICE_OBJECT device, PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;
	DbgPrint("Created!\n");
	return status;
}
NTSTATUS Myclose(PDEVICE_OBJECT device, PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;
	DbgPrint("Closed!\n");
	return status;
}
NTSTATUS Myclean(PDEVICE_OBJECT device, PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;
	DbgPrint("Clean up!\n");
	return status;
}
NTSTATUS Myread(PDEVICE_OBJECT device, PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STACK_LOCATION* pstack = IoGetCurrentIrpStackLocation(pirp); 
	ULONG readsize = pstack->Parameters.Read.Length;

	RtlCopyMemory(pirp->AssociatedIrp.SystemBuffer, "Hello Andy",strlen("Hello Andy"));
	
	//complete routine
	pirp->IoStatus.Status = status;
	pirp->IoStatus.Information = readsize;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);
	DbgPrint("success to read at kernel: %s\n", "Hello Andy");
	return status;
}
NTSTATUS DMyread(PDEVICE_OBJECT device, PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STACK_LOCATION* pstack = IoGetCurrentIrpStackLocation(pirp);
	ULONG readsize = pstack->Parameters.Read.Length;
	RtlCopyMemory(MmGetMdlVirtualAddress(pirp->MdlAddress), "Hello Andy", strlen("Hello Andy"));

	//complete routine
	pirp->IoStatus.Status = status;
	pirp->IoStatus.Information = strlen("Hello Andy");
	IoCompleteRequest(pirp, IO_NO_INCREMENT);
	DbgPrint("success to read at kernel: %s\n", "Hello Andy");
	return status;
}
NTSTATUS DriverEntry(PDRIVER_OBJECT Driver, UNICODE_STRING str)
{
	DbgPrint("Hello\n");
	Driver->DriverUnload = drvUnload;
	NTSTATUS status = STATUS_SUCCESS;
	
	PDEVICE_OBJECT Mydivice = NULL;
	//创建device
	UNICODE_STRING dname = { 0 };
	
	RtlInitUnicodeString(&dname, Device_name);

	status = IoCreateDevice(Driver, 0, &dname, FILE_DEVICE_UNKNOWN, 0, TRUE, &Mydivice);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("failed to create device\n");
		return status;
	}
	DbgPrint("success to create device\n");
	//创建符号链接
	UNICODE_STRING sname = { 0 };

	RtlInitUnicodeString(&sname, symbol_name);

	status = IoCreateSymbolicLink(&sname, &dname);
	
	if (!NT_SUCCESS(status))
	{
		DbgPrint("failed to create symbol_link\n");
		//删除device
		IoDeleteDevice(Mydivice);
		return status;
	}
	DbgPrint("success to create symbol_link\n");
	// 和R3交互
	Mydivice->Flags |= DO_BUFFERED_IO;
	Driver->MajorFunction[IRP_MJ_CREATE] = Mycreate;
	Driver->MajorFunction[IRP_MJ_CLOSE] = Myclose;
	Driver->MajorFunction[IRP_MJ_CLEANUP] = Myclean;
	Driver->MajorFunction[IRP_MJ_READ] = DMyread;
	DbgPrint("success to use MajorFunction\n");
	return 0;
}
