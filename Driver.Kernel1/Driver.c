#include "Driver.h"

VOID  Unload(IN  PDRIVER_OBJECT  pDriverObject) {
   //this deletes the device
	IoDeleteDevice(pDriverObject->DeviceObject);

	return;
}



NTSTATUS FindGameProcessByName(CHAR* process_name, PEPROCESS* process, int range)
{
	PEPROCESS sys_process = PsInitialSystemProcess;
	PEPROCESS cur_entry = sys_process;

	CHAR image_name[300];

	do
	{
		RtlCopyMemory((PVOID)(&image_name), (PVOID)((uintptr_t)cur_entry + 0x450) /*EPROCESS->ImageFileName*/, sizeof(image_name));

		if (strstr(image_name, process_name))
		{
			DWORD64 active_threads;
			RtlCopyMemory((PVOID)&active_threads, (PVOID)((uintptr_t)cur_entry + 0x498) /*EPROCESS->ActiveThreads*/, sizeof(active_threads));
			if (active_threads)
			{
				*process = cur_entry;
				return STATUS_SUCCESS;
			}
		}

		PLIST_ENTRY list = (PLIST_ENTRY)((uintptr_t)(cur_entry)+0x2F0) /*EPROCESS->ActiveProcessLinks*/;
		cur_entry = (PEPROCESS)((uintptr_t)list->Flink - 0x2F0);

		range--;

	} while (cur_entry != sys_process && range > 0);

	return STATUS_NOT_FOUND;
}

// IOCTL handler for memory commands

NTSTATUS ProcessReadWriteMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T Size)
{
	SIZE_T Bytes = 0;

	if (NT_SUCCESS(MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, Size, UserMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

#define SIOCTL_TYPE 40000

#define IOCTL_CODE 0x800

#define IOCTL_MEMORY_COMMAND\
 CTL_CODE( SIOCTL_TYPE, IOCTL_CODE, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)

NTSTATUS Function_IRP_MJ_CREATE(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	DbgPrint("IRP MJ CREATE received.");
	return STATUS_SUCCESS;
}

NTSTATUS Function_IRP_MJ_CLOSE(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	DbgPrint("IRP MJ CLOSE received.");
	return STATUS_SUCCESS;
}

PEPROCESS valorantProcess;
DWORD64 processBaseAddress;

#define COMMAND_MAGIC 0xDEADBEEF
struct memory_command {
	INT operation;

	DWORD64 magic;

	DWORD64 retval;

	DWORD64 memaddress;
	DWORD64 length;
	PVOID buffer;
};

NTSTATUS Function_IRP_DEVICE_CONTROL(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION pIoStackLocation;
	struct memory_command* cmd = Irp->AssociatedIrp.SystemBuffer;

	Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;

	pIoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	switch (pIoStackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_MEMORY_COMMAND:
		DbgPrintEx(0, 0, "[ValorHook] IOCTL command received\n");

		if (cmd->magic != COMMAND_MAGIC) {
			Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
			cmd->retval = 2;
			DbgPrintEx(0, 0, "[ValorHook] IOCTL invalid magic\n");
			break;
		}

		switch (cmd->operation) {
		case 0: // read memory
			Irp->IoStatus.Status = STATUS_SUCCESS;

			ProcessReadWriteMemory(valorantProcess, cmd->memaddress, IoGetCurrentProcess(), cmd->buffer, cmd->length);
			break;

		case 1: // write memory
			Irp->IoStatus.Status = STATUS_SUCCESS;

			ProcessReadWriteMemory(IoGetCurrentProcess(), cmd->buffer, valorantProcess, cmd->memaddress, cmd->length);
			break;
		case 2: // find valorant PEPROCESS
			Irp->IoStatus.Status = STATUS_SUCCESS;
			DbgPrintEx(0, 0, "[ValorHook] Setting target PID...\n");

			valorantProcess = NULL;

			PsLookupProcessByProcessId(cmd->retval, &valorantProcess);

			if (!valorantProcess) {
				cmd->retval = NULL;
				break;
			}
			
			cmd->retval = (DWORD64)PsGetProcessSectionBaseAddress(valorantProcess);

			break;
		case 10:
			// just crash windows idk
			Unload(gDeviceObject);
			break;
		}

		break;
	}

	// Finish the I/O operation by simply completing the packet and returning
	// the same status as in the packet itself.
	Irp->IoStatus.Information = sizeof(struct memory_command);
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

// driver entry, default functions

/// <summary>
/// Defines the entry point of this driver.
/// </summary>
/// <param name="DriverObject">The driver object.</param>
/// <param name="RegistryPath">The registry path.</param>
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrintEx(0, 0, "[ValorHook] Executing " __FUNCTION__ ".\n");

	NTSTATUS			Status;
	UNICODE_STRING		DriverName;
	UNICODE_STRING		TurlaName;
	PDRIVER_OBJECT      TurlaObject;

	RtlInitUnicodeString(&DriverName, ConstDriverName);
	RtlInitUnicodeString(&TurlaName, ConstTurlaDriverName);

	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	//wincrash
	//Unload(NULL);

	ObReferenceObjectByName(&TurlaName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID)&TurlaObject);

	if (TurlaObject)
	{
		const PKLDR_DATA_TABLE_ENTRY DriverSection = TurlaObject->DriverSection;

		if (DriverSection)
		{
			DriverSection->FullImageName.Buffer[0] = L'\0';
			DriverSection->FullImageName.Length = 0;
			DriverSection->FullImageName.MaximumLength = 0;

			DriverSection->BaseImageName.Buffer[0] = L'\0';
			DriverSection->BaseImageName.Length = 0;
			DriverSection->BaseImageName.MaximumLength = 0;
		}

		ObDereferenceObject(TurlaObject);
	}

	Status = IoCreateDriver(&DriverName, &DriverInitialize);

	if (!NT_SUCCESS(Status))
	{
		return STATUS_DRIVER_UNABLE_TO_LOAD;
	}

	return STATUS_SUCCESS;
}

/// <summary>
/// Initializes the driver and its device.
/// </summary>
/// <param name="DriverObject">The driver object.</param>
/// <param name="RegistryPath">The registry path.</param>
NTSTATUS DriverInitialize(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrintEx(0, 0, "[ValorHook] Executing " __FUNCTION__ ".\n");

	NTSTATUS			Status;
	UNICODE_STRING		DeviceName;
	UNICODE_STRING		SymbolicName;
	PDEVICE_OBJECT      DeviceObject;

	UNREFERENCED_PARAMETER(RegistryPath);

	RtlInitUnicodeString(&DeviceName, ConstDeviceName);
	RtlInitUnicodeString(&SymbolicName, ConstSymbolic);

	// Create device
	Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (NT_SUCCESS(Status))
	{
		DbgPrintEx(0, 0, "[ValorHook] Created device\n");

		Status = IoCreateSymbolicLink(&SymbolicName, &DeviceName);

		if (NT_SUCCESS(Status))
		{
			DbgPrintEx(0, 0, "[ValorHook] Created symlink\n");

			for (ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
			{
				DriverObject->MajorFunction[i] = &UnsupportedCall;
			}

			DriverObject->MajorFunction[IRP_MJ_CREATE] = &Function_IRP_MJ_CREATE;
			DriverObject->MajorFunction[IRP_MJ_CLOSE] = &Function_IRP_MJ_CLOSE;
			DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &Function_IRP_DEVICE_CONTROL;

			// Flags..

			DeviceObject->Flags |= DO_BUFFERED_IO;
			DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

			// Globals..

			gDriverObject = DriverObject;
			gDeviceObject = DeviceObject;
		}
		else
		{
			IoDeleteDevice(DeviceObject);
		}
	}

	if (DriverObject)
	{
		const PKLDR_DATA_TABLE_ENTRY DriverSection = DriverObject->DriverSection;

		if (DriverSection)
		{
			DriverSection->FullImageName.Buffer[0] = L'\0';
			DriverSection->FullImageName.Length = 0;
			DriverSection->FullImageName.MaximumLength = 0;

			DriverSection->BaseImageName.Buffer[0] = L'\0';
			DriverSection->BaseImageName.Length = 0;
			DriverSection->BaseImageName.MaximumLength = 0;
		}

		DriverObject->DriverSection = NULL;
		DriverObject->DriverStart = NULL;
		DriverObject->DriverSize = 0;
		DriverObject->DriverUnload = NULL;
		DriverObject->DriverInit = NULL;
		DriverObject->DeviceObject = NULL;

		DbgPrintEx(0, 0, "[ValorHook] Driver is ready\n");
	}

	return Status;
}