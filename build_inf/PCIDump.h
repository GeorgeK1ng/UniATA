///////////////////////////////////////////////////////////////////////////////
// General.h

#ifndef __PCI_DUMP_H__
#define __PCI_DUMP_H__

extern "C" {

#pragma pack(push, 1)

#ifndef USER_MODE

#include <ntddk.h>                  // various NT definitions
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "tools.h"

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'DicP')
#endif

#else //USER_MODE

typedef struct _PCI_SLOT_NUMBER {
    union {
        struct {
            ULONG   DeviceNumber:5;
            ULONG   FunctionNumber:3;
            ULONG   Reserved:24;
        } bits;
        ULONG   AsULONG;
    } u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;

#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2

typedef struct _PCI_COMMON_CONFIG {
    USHORT  VendorID;                   // (ro)
    USHORT  DeviceID;                   // (ro)
    USHORT  Command;                    // Device control
    USHORT  Status;
    UCHAR   RevisionID;                 // (ro)
    UCHAR   ProgIf;                     // (ro)
    UCHAR   SubClass;                   // (ro)
    UCHAR   BaseClass;                  // (ro)
    UCHAR   CacheLineSize;              // (ro+)
    UCHAR   LatencyTimer;               // (ro+)
    UCHAR   HeaderType;                 // (ro)
    UCHAR   BIST;                       // Built in self test

    union {
        struct _PCI_HEADER_TYPE_0 {
            ULONG   BaseAddresses[PCI_TYPE0_ADDRESSES];
            ULONG   CIS;
            USHORT  SubVendorID;
            USHORT  SubSystemID;
            ULONG   ROMBaseAddress;
            ULONG   Reserved2[2];

            UCHAR   InterruptLine;      //
            UCHAR   InterruptPin;       // (ro)
            UCHAR   MinimumGrant;       // (ro)
            UCHAR   MaximumLatency;     // (ro)
        } type0;


    } u;

    UCHAR   DeviceSpecific[192];

} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;


#define PCI_COMMON_HDR_LENGTH (FIELD_OFFSET (PCI_COMMON_CONFIG, DeviceSpecific))

#define PCI_MAX_DEVICES                     32
#define PCI_MAX_FUNCTION                    8

#define PCI_INVALID_VENDORID                0xFFFF

//
// Bit encodings for  PCI_COMMON_CONFIG.HeaderType
//

#define PCI_MULTIFUNCTION                   0x80
#define PCI_DEVICE_TYPE                     0x00
#define PCI_BRIDGE_TYPE                     0x01

//
// Bit encodings for PCI_COMMON_CONFIG.Command
//

#define PCI_ENABLE_IO_SPACE                 0x0001
#define PCI_ENABLE_MEMORY_SPACE             0x0002
#define PCI_ENABLE_BUS_MASTER               0x0004
#define PCI_ENABLE_SPECIAL_CYCLES           0x0008
#define PCI_ENABLE_WRITE_AND_INVALIDATE     0x0010
#define PCI_ENABLE_VGA_COMPATIBLE_PALETTE   0x0020
#define PCI_ENABLE_PARITY                   0x0040  // (ro+)
#define PCI_ENABLE_WAIT_CYCLE               0x0080  // (ro+)
#define PCI_ENABLE_SERR                     0x0100  // (ro+)
#define PCI_ENABLE_FAST_BACK_TO_BACK        0x0200  // (ro)

//
// Bit encodings for PCI_COMMON_CONFIG.Status
//

#define PCI_STATUS_FAST_BACK_TO_BACK        0x0080  // (ro)
#define PCI_STATUS_DATA_PARITY_DETECTED     0x0100
#define PCI_STATUS_DEVSEL                   0x0600  // 2 bits wide
#define PCI_STATUS_SIGNALED_TARGET_ABORT    0x0800
#define PCI_STATUS_RECEIVED_TARGET_ABORT    0x1000
#define PCI_STATUS_RECEIVED_MASTER_ABORT    0x2000
#define PCI_STATUS_SIGNALED_SYSTEM_ERROR    0x4000
#define PCI_STATUS_DETECTED_PARITY_ERROR    0x8000


//
// Bit encodes for PCI_COMMON_CONFIG.u.type0.BaseAddresses
//

#define PCI_ADDRESS_IO_SPACE                0x00000001  // (ro)
#define PCI_ADDRESS_MEMORY_TYPE_MASK        0x00000006  // (ro)
#define PCI_ADDRESS_MEMORY_PREFETCHABLE     0x00000008  // (ro)

#define PCI_TYPE_32BIT      0
#define PCI_TYPE_20BIT      2
#define PCI_TYPE_64BIT      4

//
// Bit encodes for PCI_COMMON_CONFIG.u.type0.ROMBaseAddresses
//

#define PCI_ROMADDRESS_ENABLED              0x00000001

#endif //USER_MODE

#ifndef PCI_DEV_CLASS_STORAGE

#define PCI_DEV_CLASS_OLD_DEVICE        0x00
#define PCI_DEV_CLASS_UNKNOWN           0x00

#define PCI_DEV_CLASS_STORAGE           0x01
  #define PCI_DEV_SUBCLASS_SCSI           0x00
  #define PCI_DEV_SUBCLASS_IDE            0x01
  #define PCI_DEV_SUBCLASS_RAID           0x04

#define PCI_DEV_CLASS_NETWORK_CARD      0x02
#define PCI_DEV_CLASS_VIDEO             0x03
#define PCI_DEV_CLASS_MULTIMEDIA        0x04
#define PCI_DEV_CLASS_MEMORY_CONTROLLER 0x05
#define PCI_DEV_CLASS_BRIDGE            0x06


#endif

#ifndef NO_PCIBIOS_API
#include "pci_bios32.h"
#endif //NO_PCIBIOS_API

// Device names
#define NT_PciDump_DOS_NAME     ("\\DosDevices\\PciDump")
#define NT_PciDump_NAME         ("\\Device\\PciDump")
#define NT_PciDump_USER_NAME    ("\\\\.\\PciDump")
#define NT_PciDump_SVC_NAME     ("PCIDumpEx")

#ifndef CTL_CODE
#include "winioctl.h"
#endif

#define PCID_CTL_CODE_X(a,b)    CTL_CODE(FILE_DEVICE_UNKNOWN, a,b, FILE_ANY_ACCESS )

#define IOCTL_PciDump_GetHeader             PCID_CTL_CODE_X(0x801, METHOD_BUFFERED)
#define IOCTL_PciDump_GetDataByOffset       PCID_CTL_CODE_X(0x802, METHOD_BUFFERED)
#define IOCTL_PciDump_GetBios32             PCID_CTL_CODE_X(0x803, METHOD_BUFFERED)
#define IOCTL_PciDump_GetIrqRoutingTable    PCID_CTL_CODE_X(0x804, METHOD_BUFFERED)

typedef struct _PciDump_GetHeader_USER_IN {
    ULONG BusNumber;
    ULONG DeviceNumber;
    ULONG FunctionNumber;
} PciDump_GetHeader_USER_IN, *PPciDump_GetHeader_USER_IN;

typedef struct _PciDump_GetHeader_USER_OUT {
    ULONG                 status;
    PCI_COMMON_CONFIG     pciData;
} PciDump_GetHeader_USER_OUT, *PPciDump_GetHeader_USER_OUT;

typedef struct _PciDump_GetDataByOffset_USER_IN {
    ULONG BusNumber;
    ULONG DeviceNumber;
    ULONG FunctionNumber;
    ULONG Offset;
    ULONG Length;
} PciDump_GetDataByOffset_USER_IN, *PPciDump_GetDataByOffset_USER_IN;

typedef struct _PciDump_GetDataByOffset_USER_OUT {
    ULONG                 status;
//    CHAR     pciData[0];
} PciDump_GetDataByOffset_USER_OUT, *PPciDump_GetDataByOffset_USER_OUT;

#ifndef NO_PCIBIOS_API
typedef struct _PciDump_GetBios32_USER_OUT {
    UCHAR VerMj;
    UCHAR VerMn;
    UCHAR Reserved[2];
    ULONG EntryPoint;
    ULONG Reserved1;
} PciDump_GetBios32_USER_OUT, *PPciDump_GetBios32_USER_OUT;

typedef struct _PciDump_GetIrqRoutingTable_USER_OUT {
    ULONG ItemCount;
    ULONG TotalItemCount;
    ULONG IrqBitMap;
    PCI_BIOS32_IRQ_ROUTING_TABLE_ENTRY IrqRoutingTable[1];
} PciDump_GetIrqRoutingTable_USER_OUT, *PPciDump_GetIrqRoutingTable_USER_OUT;
#endif //NO_PCIBIOS_API

#ifndef USER_MODE

// Function declarations
extern NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
extern NTSTATUS PciDumpCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
extern NTSTATUS PciDumpDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
extern VOID PciDumpUnload(IN PDRIVER_OBJECT DriverObject);

#endif //USER_MODE

#pragma pack(pop)

};

#endif // __PCI_DUMP_H__

