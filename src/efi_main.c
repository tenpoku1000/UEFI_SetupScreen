
#include <efi.h>
#include <efilib.h>

#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI 0x0000000000000001

#define true 1
#define false 0

typedef UINT8 bool;

static void reset_system(EFI_STATUS status)
{
	EFI_STATUS local_status = EFI_SUCCESS;

	do{
		EFI_INPUT_KEY key;
		local_status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
	} while (EFI_SUCCESS != local_status);

	RT->ResetSystem(EfiResetCold, status, 0, NULL);
}

static void error_print(CHAR16* msg)
{
	Print(msg);
	reset_system(EFI_SUCCESS);
}

static EFI_STATUS get_variable_u64(CHAR16* variable_name, UINT32* attr, UINT64* data)
{
	UINTN data_size = sizeof(UINT64);

	EFI_STATUS status = RT->GetVariable(variable_name, &EfiGlobalVariable, attr, &data_size, data);

	switch (status){
	case EFI_NOT_FOUND:
//		break;
	case EFI_SUCCESS:
		return EFI_SUCCESS;
	default:
		break;
	}

	return status;
}

static EFI_STATUS set_variable_u64(CHAR16* variable_name, UINT32 attr, UINT64* data)
{
	UINTN data_size = sizeof(UINT64);

	EFI_STATUS status = RT->SetVariable(variable_name, &EfiGlobalVariable, attr, data_size, data);

	switch (status) {
	case EFI_SUCCESS:
		return EFI_SUCCESS;
	default:
		break;
	}

	return status;
}

static bool is_boot_to_fw_ui()
{
	UINT32 attr = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
	UINT64 os_indications_supported = 0;

	if (EFI_SUCCESS != get_variable_u64(L"OsIndicationsSupported", &attr, &os_indications_supported)){

		error_print(L"get_variable_u64(OsIndicationsSupported) failed.\n");
	}

	if (EFI_OS_INDICATIONS_BOOT_TO_FW_UI & os_indications_supported){

		return true;
	}

	return false;
}

static void set_boot_to_fw_ui()
{
	UINT32 attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;

	UINT64 os_indications = 0;

	os_indications |= EFI_OS_INDICATIONS_BOOT_TO_FW_UI;

	if (EFI_SUCCESS != set_variable_u64(L"OsIndications", attr, &os_indications)) {

		error_print(L"set_variable_u64(OsIndications) failed.\n");
	}
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	InitializeLib(ImageHandle, SystemTable);

	if (is_boot_to_fw_ui()) {

		Print(L"When you press any key, The system will transition to the UEFI setup screen.\n");

		set_boot_to_fw_ui();

	}else{

		Print(L"When you press any key, the system will reboot.\n");
	}

	reset_system(EFI_SUCCESS);

	return EFI_SUCCESS;
}
