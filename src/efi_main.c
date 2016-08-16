
// Copyright 2015 Shin'ichi Ichikawa. Released under the MIT license.

#include <efi.h>
#include <efilib.h>
#include "efi_status.h"

#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI 0x0000000000000001

#define true 1
#define false 0

typedef UINT8 bool;

static void init(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table);
static void read_key(void);
static void reset_system(EFI_STATUS status);
static void error_print(CHAR16* msg, EFI_STATUS* status);
static EFI_STATUS get_variable_u64(CHAR16* variable_name, UINT32* attr, UINT64* data);
static EFI_STATUS set_variable_u64(CHAR16* variable_name, UINT32 attr, UINT64* data);
static bool is_boot_to_fw_ui(void);
static void set_boot_to_fw_ui(void);

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table)
{
    init(image_handle, system_table);

    if (is_boot_to_fw_ui()){

        Print(L"When you press any key, The system will transition to the UEFI setup screen.\n");

        set_boot_to_fw_ui();

    }else{

        Print(L"When you press any key, the system will reboot.\n");
    }

    reset_system(EFI_SUCCESS);

    return EFI_SUCCESS;
}

static void init(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table)
{
    InitializeLib(image_handle, system_table);

    EFI_STATUS status = EFI_SUCCESS;

    if ((NULL == ST->ConIn) || (EFI_SUCCESS != (status = ST->ConIn->Reset(ST->ConIn, 0)))){

        error_print(L"Input device unavailable.\n", ST->ConIn ? &status : NULL);
    }
}

static void read_key(void)
{
    if (ST->ConIn){

        EFI_STATUS local_status = EFI_SUCCESS;

        do{
            EFI_INPUT_KEY key;

            local_status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key);

        } while (EFI_SUCCESS != local_status);
    }
}

static void reset_system(EFI_STATUS status)
{
    read_key();

    RT->ResetSystem(EfiResetCold, status, 0, NULL);
}

static void error_print(CHAR16* msg, EFI_STATUS* status)
{
    Print(msg);

    if (status){

        Print(L"EFI_STATUS = %d, %s\n", *status, print_status_msg(*status));
    }

    reset_system(EFI_SUCCESS);
}

static EFI_STATUS get_variable_u64(CHAR16* variable_name, UINT32* attr, UINT64* data)
{
    UINTN data_size = sizeof(UINT64);

    EFI_STATUS status = RT->GetVariable(variable_name, &EfiGlobalVariable, attr, &data_size, data);

    switch (status){
    case EFI_NOT_FOUND:
//        break;
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

    switch (status){
    case EFI_SUCCESS:
        return EFI_SUCCESS;
    default:
        break;
    }

    return status;
}

static bool is_boot_to_fw_ui(void)
{
    UINT32 attr = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
    UINT64 os_indications_supported = 0;

    EFI_STATUS status = get_variable_u64(L"OsIndicationsSupported", &attr, &os_indications_supported);

    if (EFI_ERROR(status)){

        error_print(L"get_variable_u64(OsIndicationsSupported) failed.\n", &status);
    }

    if (EFI_OS_INDICATIONS_BOOT_TO_FW_UI & os_indications_supported){

        return true;
    }

    return false;
}

static void set_boot_to_fw_ui(void)
{
    UINT32 attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;

    UINT64 os_indications = 0;

    os_indications |= EFI_OS_INDICATIONS_BOOT_TO_FW_UI;

    EFI_STATUS status = set_variable_u64(L"OsIndications", attr, &os_indications);

    if (EFI_ERROR(status)){

        error_print(L"set_variable_u64(OsIndications) failed.\n", &status);
    }
}

