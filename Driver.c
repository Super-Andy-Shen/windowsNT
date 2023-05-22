#include<wdm.h>
#include<ntddk.h>
#include<ntstrsafe.h>
#define Device_name L"\\Device\\myfirst"
#define symbol_name L"\\??\\myfirst123"

UNICODE_STRING str1 = RTL_CONSTANT_STRING(L"I am the first thread");
UNICODE_STRING str2 = RTL_CONSTANT_STRING(L"I am the second thread");
VOID drvUnload(PDRIVER_OBJECT Driver)
{
	PDEVICE_OBJECT deviceObject = Driver->DeviceObject;
	UNICODE_STRING uniWin32NameString;

	PAGED_CODE();
	//
	// Create counted string version of our Win32 device name.
	//

	RtlInitUnicodeString(&uniWin32NameString, symbol_name);
	//
	// Delete the link from our device name to a name in the Win32 namespace.
	//
	IoDeleteSymbolicLink(&uniWin32NameString);

	if (deviceObject != NULL)
	{
		IoDeleteDevice(deviceObject);
	}

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
	UNREFERENCED_PARAMETER(device);

	PAGED_CODE();

	pirp->IoStatus.Status = STATUS_SUCCESS;
	pirp->IoStatus.Information = 0;

	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
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
	pirp->IoStatus.Information = strlen("Hello Andy");
	IoCompleteRequest(pirp, IO_NO_INCREMENT);
	DbgPrint("success to read at kernel: %s\n", "Hello Andy");
	return status;
}
NTSTATUS DMyread(PDEVICE_OBJECT device, PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STACK_LOCATION* pstack = IoGetCurrentIrpStackLocation(pirp);
	ULONG readsize = pstack->Parameters.Read.Length;
	PVOID buffer = MmGetMdlVirtualAddress(pirp->MdlAddress);
	RtlCopyMemory(buffer, "Hello Andy", strlen("Hello Andy"));
	//complete routine
	pirp->IoStatus.Status = status;
	pirp->IoStatus.Information = strlen("Hello Andy");
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	DbgPrint("success to read at kernel: %s\n",buffer);
	return status;
}
NTSTATUS stringtest()
{
	UNICODE_STRING a = { 0 };
	PWCHAR b = L"AB";
	RtlUnicodeStringInit(&a, b);
	DbgPrint("Length %d and MaxLength %d, %wZ\n", a.Length, a.MaximumLength, a);
	//ascii->unicode
	STRING s = { 0 };
	PWCHAR t = "ABC";
	RtlInitAnsiString(&s, t);
	RtlAnsiStringToUnicodeString(&a, &s, TRUE);
	DbgPrint("Length %d and MaxLength %d, %wZ\n", a.Length, a.MaximumLength, a);
	//Copy Unicodestring 1
	UNICODE_STRING c = { 0 };
	c.Buffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(UNICODE_STRING), 't2');
	c.Length = a.Length; c.MaximumLength = a.MaximumLength;
	c.Buffer = a.Buffer;
	RtlCopyUnicodeString(&c, &a);
	DbgPrint("Length %d and MaxLength %d, %wZ\n", c.Length, c.MaximumLength, c);
	ExFreePoolWithTag(c.Buffer, 't2');
	//Copy Unicodestring 2
	UNICODE_STRING d = { 0 };
	d.Length = c.Length; d.MaximumLength = c.MaximumLength; d.Buffer = c.Buffer;
	DbgPrint("Length %d and MaxLength %d, %wZ\n", d.Length, d.MaximumLength, d);
	DbgPrint("Length %d and MaxLength %d, %wZ\n", d.Length, d.MaximumLength, d);


	PWCHAR e = ExAllocatePoolWithTag(NonPagedPool, 0x1000, "t3");
	RtlZeroMemory(e, 0x1000);
	UNICODE_STRING f = { 0 };
	RtlStringCbCopyW(e, 0x1000, L"ABCDE");
	DbgPrint("string is %ws", e);
	RtlInitUnicodeString(&f, e);
	DbgPrint("string is %wZ", f);
	ExFreePoolWithTag(e, 't3');
	return 0;
}
NTSTATUS Mywrite(PDEVICE_OBJECT device, PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;

	IO_STACK_LOCATION* pstack = IoGetCurrentIrpStackLocation(pirp);

	ULONG wsize = pstack->Parameters.Write.Length;

	PVOID buffer = ExAllocatePoolWithTag(NonPagedPool, 2000, 't1');
	if (buffer == NULL)
	{
		DbgPrint("buffer is NULL.\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	if (wsize > 2000)
	{
		DbgPrint("writesize is too big.\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	RtlCopyMemory(buffer, pirp->AssociatedIrp.SystemBuffer, wsize);
	DbgPrint("success to write: %s\n",(PCHAR)buffer);
	//complete routine
	pirp->IoStatus.Status = status;
	pirp->IoStatus.Information = wsize;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);
	ExFreePoolWithTag(buffer, 't1');
	return status;
}
NTSTATUS lookasideallocate()
{
	NTSTATUS status = STATUS_SUCCESS;
	PAGED_LOOKASIDE_LIST lookside;

	ExInitializePagedLookasideList(&lookside, NULL, NULL, PagedPool, sizeof(UNICODE_STRING), 'A', 0);
	
	PUNICODE_STRING pstr = ExAllocateFromPagedLookasideList(&lookside);
	RtlZeroMemory(pstr, sizeof(UNICODE_STRING));
	
	PWCHAR oristring = L"Hi";
	UNICODE_STRING string = { 0 };
	RtlInitUnicodeString(&string, oristring);
	RtlCopyMemory(pstr, &string, sizeof(UNICODE_STRING));
	ExFreeToPagedLookasideList(&lookside, pstr);
}
VOID MyThreadProc1(PVOID context)
{
	PUNICODE_STRING str = (PUNICODE_STRING)context;
	for (int i = 0; i < 10; i++)
	{
		DbgPrint("----- %wZ\n", str);
	}
	PsTerminateSystemThread(STATUS_SUCCESS);
}
VOID MyThreadProc2(PVOID context)
{
	PUNICODE_STRING str = (PUNICODE_STRING)context;
	for (int i = 0; i < 10; i++)
	{
		DbgPrint("----- %wZ\n", str);
	}
	PsTerminateSystemThread(STATUS_SUCCESS);
}
NTSTATUS DriverEntry(PDRIVER_OBJECT Driver, UNICODE_STRING str)
{
	DbgPrint("Hello\n");
	Driver->DriverUnload = drvUnload;
	NTSTATUS status = STATUS_SUCCESS;
	
	PDEVICE_OBJECT Mydivice = NULL;
	//create device
	UNICODE_STRING dname = { 0 };
	
	RtlInitUnicodeString(&dname, Device_name);

	status = IoCreateDevice(Driver, 0, &dname, FILE_DEVICE_UNKNOWN, 0, TRUE, &Mydivice);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("failed to create device\n");
		return status;
	}
	DbgPrint("success to create device\n");
	//symbol link
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
	Driver->MajorFunction[IRP_MJ_READ] = Myread;
    Driver->MajorFunction[IRP_MJ_WRITE] = Mywrite;
	DbgPrint("success to use MajorFunction\n");
	HANDLE thread1 = NULL;
	HANDLE thread2 = NULL;
	status = PsCreateSystemThread(&thread1, GENERIC_ALL, NULL, NULL, NULL, MyThreadProc1, (PVOID)&str1);
	if (status != STATUS_SUCCESS)
	{
		DbgPrint("fail to create thread1\n");
		return 0;
	}
	KIRQL irql = 0;
	KeRaiseIrql(2, &irql);
	status = PsCreateSystemThread(&thread2, GENERIC_ALL, NULL, NULL, NULL, MyThreadProc2, (PVOID)&str2);
	if (status != STATUS_SUCCESS)
	{
		DbgPrint("fail to create thread2\n");
		return 0;
	}
	KeLowerIrql(&irql);
	ZwClose(thread1);
	ZwClose(thread2);
	return 0;
}
