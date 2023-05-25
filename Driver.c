#include<wdm.h>
#include<ntddk.h>
#include<ntstrsafe.h>
#include<windef.h>
#define Device_name L"\\Device\\myfirst"
#define symbol_name L"\\??\\myfirst123"
#define IOCTL		\
		CTL_CODE(FILE_DEVICE_UNKNOWN,		\
				 0x9000,					\
				 METHOD_BUFFERED,			\
				 FILE_ANY_ACCESS)


UNICODE_STRING str1 = RTL_CONSTANT_STRING(L"I am the first thread");
UNICODE_STRING str2 = RTL_CONSTANT_STRING(L"I am the second thread");
KSPIN_LOCK spinlock;
BYTE mmcode[100] = { 0 };
BOOLEAN block = FALSE;
KEVENT kevent = { 0 };
KEVENT* pevent = NULL;
PCHAR PsGetProcessImageFileName(IN PEPROCESS process);
VOID MyThreadProc2(PVOID context);
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
	//窄字符-》宽字符
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
	//创建内存列表
	ExInitializePagedLookasideList(&lookside, NULL, NULL, PagedPool, sizeof(UNICODE_STRING), 'A', 0);
	//分配内存并初始化
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
	HANDLE thread2 = NULL;
	KeInitializeEvent(&kevent, NotificationEvent, FALSE);
	NTSTATUS status = PsCreateSystemThread(&thread2, GENERIC_ALL, NULL, NULL, NULL, MyThreadProc2, (PVOID)&kevent);
	int i = 0;
	while (i<10)
	{
		KeWaitForSingleObject(&kevent, Executive, KernelMode, FALSE, NULL);
		for (int i = 0; i < 10; i++)
		{
			DbgPrint("mmcode: %c \n",mmcode[i]);
		}
		i++;
		KeResetEvent(&kevent);
	}
	PsTerminateSystemThread(STATUS_SUCCESS);
}
VOID MyThreadProc2(PVOID context)
{
	LARGE_INTEGER sleeptime = { 0 };
	sleeptime.QuadPart = -10 * 1000 * 1000;
	PKEVENT pevent = (PKEVENT)context;
	UNICODE_STRING apiname = { 0 };
	RtlInitUnicodeString(&apiname, L"ntoskrnl.exe");
	int i = 0;
	while (i<10)
	{
		KeDelayExecutionThread(KernelMode, FALSE, &sleeptime);
		RtlZeroMemory(mmcode, 100);
		RtlCopyMemory(mmcode, apiname.Buffer, sizeof(L"Hello Andy"));
		DbgPrint("Welcome\n");
		i++;
		KeSetEvent(pevent, IO_NO_INCREMENT, FALSE);
	}
	PsTerminateSystemThread(STATUS_SUCCESS);
}
VOID MyThreadProc3(PVOID context)
{
	NTSTATUS status = STATUS_SUCCESS;
	int i = 0;
	while (i<10)
	{
		status = KeWaitForSingleObject(pevent, Executive, KernelMode, FALSE, NULL);
		DbgPrint("Hello from kernel\n");
		i++;
	}
	PsTerminateSystemThread(STATUS_SUCCESS);
}
NTSTATUS Mycontrol(PDEVICE_OBJECT pdevice, PIRP pirp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pirp);
	ULONG iocode = stack->Parameters.DeviceIoControl.IoControlCode;
	DbgPrint("-------%d-------%d\n",iocode,IOCTL);
	switch (iocode)
	{
	case IOCTL:
	{
		//DWORD indata = *(PWORD)pirp->AssociatedIrp.SystemBuffer;
		HANDLE hevent = *(HANDLE*)pirp->AssociatedIrp.SystemBuffer;
		status = ObReferenceObjectByHandle(hevent, EVENT_MODIFY_STATE, *ExEventObjectType, KernelMode, &pevent, NULL);
		if (NT_SUCCESS(status))
		{
			ObDereferenceObject(pevent);
			HANDLE hthread = NULL;
			status = PsCreateSystemThread(&hthread, GENERIC_ALL, NULL, NULL, NULL, MyThreadProc3, NULL);

		}
	}
	}
	return status;
}
VOID ProcessFun(HANDLE pid, HANDLE pid2, BOOLEAN create)
{
	if (create)
	{
		if (pid == PsGetCurrentProcess())
		{
			DbgPrint("process create, PID : %d\n",pid2);
			PEPROCESS temp = NULL;
			PsLookupProcessByProcessId(pid2, &temp);
			PUCHAR processname = PsGetProcessImageFileName(temp);
			DbgPrint("Process Name is %s\n",processname);
		}
	}
}
NTSTATUS DriverEntry(PDRIVER_OBJECT Driver, UNICODE_STRING str)
{
	DbgPrint("Hello\n");
	Driver->DriverUnload = drvUnload;
	NTSTATUS status = STATUS_SUCCESS;
	PsSetCreateProcessNotifyRoutine(ProcessFun, FALSE);
	
	
	
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
	Driver->MajorFunction[IRP_MJ_READ] = Myread;
    Driver->MajorFunction[IRP_MJ_WRITE] = Mywrite;
	//Driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Mycontrol;
	DbgPrint("success to use MajorFunction\n");


	//HANDLE thread1 = NULL;
	/*test for event (example 1)
	status = PsCreateSystemThread(&thread1, GENERIC_ALL, NULL, NULL, NULL, MyThreadProc1, (PVOID)&kevent);
	ZwClose(thread1);*/
	return 0;
}
