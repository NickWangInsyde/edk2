/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/UsbIo.h>

//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID  mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);

//
// Copy the USB_KB_DEV definition from EfiKey.h
//
#define MAX_KEY_ALLOWED                 64

typedef struct {
  VOID          *Buffer[MAX_KEY_ALLOWED + 1];
  UINTN         Head;
  UINTN         Tail;
  UINTN         ItemSize;
} USB_SIMPLE_QUEUE;

typedef struct {
  UINTN                                 Signature;
  EFI_HANDLE                            ControllerHandle;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL        SimpleInput;
  EFI_USB_IO_PROTOCOL                   *UsbIo;
  EFI_USB_INTERFACE_DESCRIPTOR          InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR           IntEndpointDescriptor;
  USB_SIMPLE_QUEUE                      UsbKeyQueue;
  USB_SIMPLE_QUEUE                      EfiKeyQueue;
} USB_KB_DEV;

#define USB_KB_DEV_SIGNATURE            SIGNATURE_32 ('u', 'k', 'b', 'd')
#define USB_KB_DEV_FROM_THIS(a)         CR(a, USB_KB_DEV, SimpleInput, USB_KB_DEV_SIGNATURE)

/**
  Try to peek at the last user input character from the USB keyboard via the EFI_SIMPLE_TEXT_INPUT_PROTOCOL pointer.
**/
VOID
PeekLastCharFromUsbKbDev (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Index;
  UINTN                                 HandleCount;
  EFI_HANDLE                            *HandleBuffer;
  EFI_USB_IO_PROTOCOL                   *UsbIo;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL        *SimpleInput;
  USB_KB_DEV                            *UsbKbDev;
  EFI_KEY_DATA                          KeyData;

  HandleCount   = 0;
  HandleBuffer  = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleTextInProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiUsbIoProtocolGuid, (VOID **) &UsbIo);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiSimpleTextInProtocolGuid, (VOID **) &SimpleInput);
    if (EFI_ERROR (Status)) {
      continue;
    }

    UsbKbDev = USB_KB_DEV_FROM_THIS(SimpleInput);
    break;
  }
  gBS->FreePool (HandleBuffer);
  if (UsbKbDev == NULL) {
    return;
  }

  if (UsbKbDev->EfiKeyQueue.Head == 0) {
    CopyMem (&KeyData, UsbKbDev->EfiKeyQueue.Buffer[MAX_KEY_ALLOWED], sizeof(KeyData));
  } else {
    CopyMem (&KeyData, UsbKbDev->EfiKeyQueue.Buffer[UsbKbDev->EfiKeyQueue.Head - 1], sizeof(KeyData));
  }
  Print (L"USB KB Last Char=%c\n", KeyData.Key.UnicodeChar);
}

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32  Index;

  Index = 0;

  //
  // Three PCD type (FeatureFlag, UINT32 and String) are used as the sample.
  //
  if (FeaturePcdGet (PcdHelloWorldPrintEnable)) {
    for (Index = 0; Index < PcdGet32 (PcdHelloWorldPrintTimes); Index++) {
      //
      // Use UefiLib Print API to print string to UEFI console
      //
      Print ((CHAR16 *)PcdGetPtr (PcdHelloWorldPrintString));
    }
  }

  PeekLastCharFromUsbKbDev ();

  return EFI_SUCCESS;
}
