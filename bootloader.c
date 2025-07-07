#include <efi/lib.h>
#include <efi/efilib.h>
#include <stdint.h>

#define PAGE_SIZE 4096

EFI_STATUS getMemoryMap(EFI_MEMORY_DESCRIPTOR **map_buf, UINTN *map_size, UINTN *map_key, UINTN *desc_size, UINT32 *desc_version) {
    EFI_STATUS status = EFI_SUCCESS;

    getMap:
    *map_size += sizeof(**map_buf);

    status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, *map_size, (void **)map_buf);
    if (status != EFI_SUCCESS) {
        Print(L"BOOTLOADER ERROR: Failed to get memory map (Allocate Error); HALTING");
        while(1) {

        }
        return status;
    }

    status = uefi_call_wrapper(BS->GetMemoryMap, 5, map_size, *map_buf, map_key, desc_size, desc_version);
    if (status != EFI_SUCCESS) {
        if (status == EFI_BUFFER_TOO_SMALL) {
            uefi_call_wrapper(BS->FreePool, 1, (void *)*map_buf);
            goto getMap;
        }
        Print(L"BOOTLOADER ERROR: Failed to get memory map; HALTING");
        while(1) {

        }
    }
    return status;
}

EFI_FILE_HANDLE getVolume(EFI_HANDLE image) {
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *IOVolume;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_HANDLE Volume;

    uefi_call_wrapper(BS->HandleProtocol, 3, image, &lipGuid, (void **) &loaded_image);
    uefi_call_wrapper(BS->HandleProtocol, 3, loaded_image->DeviceHandle, &fsGuid, (VOID*)&IOVolume);
    uefi_call_wrapper(IOVolume->OpenVolume, 2, IOVolume, &Volume);
    return Volume;
}

void *cpymem(void *dest, const void *src, unsigned long n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (unsigned long i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable){
    InitializeLib(imageHandle, systemTable);

    // Get Framebuffer
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN SizeOfInfo, numModes, nativeMode;
    EFI_STATUS status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);
    if (status == EFI_NOT_STARTED) {
        status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
    }

    if(EFI_ERROR(status)) {
        Print(L"BOOTLOADER ERROR: Could not create framebuffer (No Native Video Mode Found); HALTING");
        while(1) {

        }
    } else {
        nativeMode = gop->Mode->Mode;
        numModes = gop->Mode->MaxMode;
    }

    for (int i = 0; i < numModes; i++) {
        status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
    }

    status = uefi_call_wrapper(gop->SetMode, 2, gop, nativeMode);
    if(EFI_ERROR(status)) {
        Print(L"BOOTLOADER ERROR: Could not create framebuffer (Video Mode Could Not Be Set); HALTING");
        while(1) {

        }
    } else {
        Print(L"BOOTLOADER: Framebuffer Created\n");
    }

    // Load kernel from "kernel.bin"
    Print(L"BOOTLOADER: Reading kernel from \"kernel.bin\"\n");
    CHAR16 *fileName = L"kernel.bin";
    EFI_FILE_HANDLE volume = getVolume(imageHandle);
    EFI_FILE_HANDLE file;
    uefi_call_wrapper(volume->Open, 5, volume, &file, fileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

    EFI_FILE_INFO *fileInfo = LibFileInfo(file);
    UINT64 fileSize = fileInfo->FileSize;
    UINT8 *fileData = AllocatePool(fileSize);
    uefi_call_wrapper(file->Read, 3, file, &fileSize, fileData);

    // Get Memory to put kernel
    EFI_MEMORY_DESCRIPTOR *buf;
    UINTN desc_size;
    UINT32 desc_version;
    UINTN size, map_key, mapping_size;
    EFI_MEMORY_DESCRIPTOR *desc;
    EFI_STATUS err = EFI_SUCCESS;
    int i = 0;

    getMemoryMap(&buf, &size, &map_key, &desc_size, &desc_version);

    int foundGoodMemory = 0;
    void* goodRamStart;
    desc = buf;
    while ((void *)desc < (void *)buf + size) {
        mapping_size = desc->NumberOfPages * PAGE_SIZE;

        if(desc->Type == 7 && mapping_size > 256000) {
            Print(L"BOOTLOADER: Found memory to put kernel at %016llx-%016llx\n", desc->PhysicalStart, desc->PhysicalStart + mapping_size);
            foundGoodMemory = 1;
            goodRamStart = (void*)desc->PhysicalStart;
            break;
        }

        desc = (void *)desc + desc_size;
        i++;
    }
    if (foundGoodMemory != 1) {
        Print(L"BOOTLOADER ERROR: Could not find ram to load kernel in; HALTING");
        while(1) {

        }
    }

    // Exit Boot Services and copy kernel to found memory and execute it
    Print(L"BOOTLOADER: Copying kernel from \"kernel.bin\" to memory and executing it\n");
    uefi_call_wrapper(BS->ExitBootServices, 2, imageHandle, map_key);

    void* kernel_entry = cpymem(goodRamStart, fileData, fileSize);
    void* stack_top = (void*)(desc->PhysicalStart + 0x18000);
    uint64_t fbb = gop->Mode->FrameBufferBase;
    uint32_t fbpps = gop->Mode->Info->PixelsPerScanLine;

    __asm__ volatile (
        "mov %[stack], %%rsp\n"
        "mov %[fbb], %%rdi\n"
        "mov %[fbpps], %%esi\n"
        "jmp *%[entry]\n"
        :
        : [stack] "r"(stack_top),
          [fbb]   "r"(fbb),
          [fbpps] "r"(fbpps),
          [entry] "r"(kernel_entry)
        : "rdi", "esi"
    );

    Print(L"BOOTLOADER ERROR: Failed to execute kernel; How did we get here?");

    while(1) {

    }
}
