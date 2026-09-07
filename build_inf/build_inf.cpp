/*++

Copyright (c) 2003 Alexander A. Telyatnikov (Alter)

Module Name:
    build_inf.cpp

Abstract:
    This is simple utility that constructs uniata2k.inf from
    uniata2k.in_ by insertion of device list, taken from bm_devs.h

Author:
    Alexander A. Telyatnikov (Alter)

Environment:
    User mode only

Notes:

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Revision History:

Licence:
    GPLv2

--*/

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "..\driver\atapi.h"
#include "..\driver\bm_devs.h"
#include "..\driver\uniata_ver.h"
#include "PCIDump.h"
#include "SvcManLib.h"

#define DEVLIST_NAME     ";DEVLIST"
#define DEVNAMELIST_NAME ";DEVNAMELIST"
#define DRIVERVER_NAME   "DriverVer"

BOOLEAN g_html=FALSE;

#define MAX_NEW_NUM_BUSMASTER_ADAPTERS  64
ULONG NewBusmasterCount=0;
BUSMASTER_CONTROLLER_INFORMATION_BASE NewBusMasterAdapters[MAX_NEW_NUM_BUSMASTER_ADAPTERS];

#define UNKNOWN_DEVNAME_TEMPLATE  "Compatible ATA controller (Vendor/Device %4.4x/%4.4x)"
CHAR UnknownDevName[] = "Compatible ATA controller (Vendor/Device ffff/ffff)";

LONGLONG
get_image_size(
    HANDLE h
    )
{
    LONG offh = 0;
    LONG offl = 0;

    offl = SetFilePointer((HANDLE)h, 0, &offh, FILE_END);
    if(GetLastError())
        return -1;
    return (((LONGLONG)offh)<<32) | offl;
}


LONGLONG
set_file_pointer(
    HANDLE h,
    LONGLONG offs
    )
{
    LONG offh = (ULONG)(offs >> 32);
    LONG offl = (ULONG)offs;

    offl = SetFilePointer((HANDLE)h, offl, &offh, FILE_BEGIN);
    if(GetLastError())
        return -1;
    return (((LONGLONG)offh)<<32) | offl;
}

VOID
BuildDevList()
{
    ULONG BusId=0;
    ULONG DevId=0;
    ULONG DevFunc=0;

    ULONG returned;

    PciDump_GetHeader_USER_IN  buf_in;
    PciDump_GetHeader_USER_OUT buf_out;

    HANDLE h;

    h = CreateFile(NT_PciDump_USER_NAME, GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);

    while(h != ((HANDLE)-1)) {

        buf_in.BusNumber = BusId;
        buf_in.DeviceNumber = 0;
        buf_in.FunctionNumber = 0;
        
        if(!DeviceIoControl(h,IOCTL_PciDump_GetHeader,
                            &buf_in,sizeof(buf_in),
                            &buf_out,sizeof(buf_out),
                            &returned,NULL)
                  ||
            buf_out.status == 0) {
            CloseHandle(h);
            h = ((HANDLE)-1);
            break;
        }
        
        for(DevId=0; DevId<PCI_MAX_DEVICES; DevId++) {
            
            buf_in.BusNumber = BusId;
            buf_in.DeviceNumber = DevId;
            buf_in.FunctionNumber = 0;

            DeviceIoControl(h,IOCTL_PciDump_GetHeader,
                                &buf_in,sizeof(buf_in),
                                &buf_out,sizeof(buf_out),
                                &returned,NULL);

            if(buf_out.status == 2)
                continue;

            buf_in.BusNumber = BusId;
            buf_in.DeviceNumber = DevId;
            buf_in.FunctionNumber = 1;

            DeviceIoControl(h,IOCTL_PciDump_GetHeader,
                                &buf_in,sizeof(buf_in),
                                &buf_out,sizeof(buf_out),
                                &returned,NULL);

            if(buf_out.status == 2)
                continue;

            for(DevFunc=0; DevFunc<PCI_MAX_FUNCTION; DevFunc++) {

                buf_in.BusNumber = BusId;
                buf_in.DeviceNumber = DevId;
                buf_in.FunctionNumber = DevFunc;

                DeviceIoControl(h,IOCTL_PciDump_GetHeader,
                                    &buf_in,sizeof(buf_in),
                                    &buf_out,sizeof(buf_out),
                                    &returned,NULL);

                if(buf_out.status == 2)
                    break;

                if(Ata_is_dev_listed((PBUSMASTER_CONTROLLER_INFORMATION_BASE)&BusMasterAdapters[0],
                                     buf_out.pciData.VendorID,
                                     buf_out.pciData.DeviceID,
                                     0,
                                     NUM_BUSMASTER_ADAPTERS) != -1)
                    continue;
                if(Ata_is_dev_listed(&NewBusMasterAdapters[0],
                                     buf_out.pciData.VendorID,
                                     buf_out.pciData.DeviceID,
                                     0,
                                     NewBusmasterCount) != -1)
                    continue;
                if(!Ata_is_supported_dev(&buf_out.pciData))
                    continue;
                if(NewBusmasterCount >= MAX_NEW_NUM_BUSMASTER_ADAPTERS)
                    continue;

                NewBusMasterAdapters[NewBusmasterCount].nVendorId = buf_out.pciData.VendorID;
                NewBusMasterAdapters[NewBusmasterCount].nDeviceId = buf_out.pciData.DeviceID;
                NewBusmasterCount++;
            }
        }

        BusId++;
    }
      
    if(h != ((HANDLE)-1)) {
        CloseHandle(h);
        h = ((HANDLE)-1);
    }
}

char*
GetAtaTypeStr(
    int k
    )
{
    switch(BusMasterAdapters[k].nVendorId) {
    case ATA_SIS_ID:
    case ATA_VIA_ID:
        return "";
    }
    switch(BusMasterAdapters[k].MaxTransferMode) {
        case ATA_PIO:
        case ATA_PIO_NRDY:
        case ATA_PIO0:
        case ATA_PIO1:
        case ATA_PIO2:
        case ATA_PIO3:
        case ATA_PIO4:
        case ATA_PIO5:
            return "IDE";
        case ATA_DMA:
        case ATA_SDMA1:
        case ATA_SDMA2:
        case ATA_WDMA:
        case ATA_WDMA1:
        case ATA_WDMA2:
            return "IDE DMA";
        case ATA_UDMA0:
            return "ATA-16";
        case ATA_UDMA1:
            return "ATA-25";
        case ATA_UDMA2:
            return "ATA-33";
        case ATA_UDMA3:
            return "ATA-44";
        case ATA_UDMA4:
            return "ATA-66";
        case ATA_UDMA5:
            return "ATA-100";
        case ATA_UDMA6:
            return "ATA-133";
/*        case ATA_UDMA7:
            return "ATA-166";*/
        case ATA_SA150:
            return "SATA";
        case ATA_SA300:
            return "SATA2";
        case ATA_SA600:
            return "SATA3";
    }
    return "IDE";
}

char*
GetControllerTypeStr(
    int k
    )
{
    if(BusMasterAdapters[k].RaidFlags & UNIATA_AHCI) {
        return "AHCI";
    }
    return "";
}

void main (int argc, char* argv[]) {
    HANDLE h;
    LONG l=0;
    PUCHAR buff=NULL;
    PUCHAR buff_j=NULL;
    PUCHAR buff_i=NULL;
    LONG tmp;
    LONG i=0, j=0;
    ULONG k;
    struct tm *newtime;
    time_t aclock;
    ULONG ver_1=0, ver_2=0, ver_3=0;
    char ver_str[] = UNIATA_VER_STR;
    
    if(argc < 2)
        exit(-1);

    i=1;
    if(!strcmp(argv[i], "--ver_h")) {
        time( &aclock );                 /* Get time in seconds */
        newtime = localtime( &aclock );  /* Convert time to struct */

        if(sizeof(UNIATA_VER_STR) > 1) {
            if(sscanf(ver_str, "%d", &ver_1) != 1) {
                ver_1 = 0;
            }
        }
        if(sizeof(UNIATA_VER_STR) > 3) {
            ver_2 = ver_str[2]-'a'+1;
        }
        if(sizeof(UNIATA_VER_STR) > 4) {
            if(sscanf(ver_str+3, "%d", &ver_3) != 1) {
                ver_3 = 0;
            }
        }
        printf("#define UNIATA_VER_STR         \"%s\"\n",
            UNIATA_VER_STR
            );
        printf("#define UNIATA_VER_DOT         0.%d.%d.%d\n",
            ver_1,ver_2,ver_3
            );
        printf("#define UNIATA_VER_MJ          0\n");
        printf("#define UNIATA_VER_MN          %d\n",
            ver_1
            );
        printf("#define UNIATA_VER_SUB_MJ      %d\n",
            ver_2
            );
        printf("#define UNIATA_VER_SUB_MN      %d\n",
            ver_3
            );
        printf("#define UNIATA_VER_DOT_COMMA   0,%d,%d,%d\n",
            ver_1,ver_2,ver_3
            );
        printf("#define UNIATA_VER_DOT_STR     \"0.%d.%d.%d\"\n",
            ver_1,ver_2,ver_3
            );
        printf("#define UNIATA_VER_YEAR        %d\n",
            newtime->tm_year+1900
            );
        printf("#define UNIATA_VER_YEAR_STR    \"%d\"\n",
            newtime->tm_year+1900
            );
        exit(0);
    } else
    if(!strcmp(argv[i], "--ver")) {
        if(sizeof(UNIATA_VER_STR) > 1) {
            if(sscanf(ver_str, "%d", &ver_1) != 1) {
                ver_1 = 0;
            }
        }
        if(sizeof(UNIATA_VER_STR) > 3) {
            ver_2 = ver_str[2]-'a'+1;
        }
        if(sizeof(UNIATA_VER_STR) > 4) {
            if(sscanf(ver_str+3, "%d", &ver_3) != 1) {
                ver_3 = 0;
            }
        }
        printf("0.%d.%d.%d",
            ver_1,ver_2,ver_3
            );
        exit(0);
    } else
    if(!strcmp(argv[i], "--timestamp")) {
        time( &aclock );                 /* Get time in seconds */
        newtime = localtime( &aclock );  /* Convert time to struct */
                                         /* tm form */
        printf("%2.2d/%2.2d/%4.4d",
            newtime->tm_mon+1,
            newtime->tm_mday,
            newtime->tm_year+1900
            );
        exit(0);
    } else
    if(!strcmp(argv[i], "--html")) {
        ULONG prev_ven=-1;
        PCHAR xdev_str="";
        PVEN_STR prev_str = &xdev_str[0];
        g_html = TRUE;
        for(k=0; k<NUM_BUSMASTER_ADAPTERS; k++) {

            if(!BusMasterAdapters[k].FullDevName) {
                continue;
            }
            if(!strcmp(prev_str, BusMasterAdapters[k].FullDevName)) {
                continue;
            }
            prev_str = BusMasterAdapters[k].FullDevName;
            if(prev_ven != BusMasterAdapters[k].nVendorId) {
                printf("\n<li> ");
            } else {
                printf("\n , ");
            }
            prev_ven = BusMasterAdapters[k].nVendorId;
            printf("%s %s %s", prev_str, GetAtaTypeStr(k), GetControllerTypeStr(k));
        }
        printf("\n");
        exit(0);
    }
    h = CreateFile(argv[i], GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
    if(h == ((HANDLE)-1)) {
        exit(-2);
    }
    l = (LONG)get_image_size(h);
    if(l < 1) {
        exit(-3);
    }
    if(set_file_pointer(h, 0) < 0) {
        exit(-3);
    }
    buff = (PUCHAR)GlobalAlloc(GMEM_FIXED, l+1);
    if(!ReadFile(h, buff, l, (PULONG)&tmp, NULL) ||
        tmp != l) {
        tmp = GetLastError();

        exit(-4);
    }
    CloseHandle(h);

    memset(NewBusMasterAdapters, 0, sizeof(NewBusMasterAdapters));

    for(i=0, j=0; i<l; i++) {
        if(buff[i] == '\r')
            continue;
        buff[j] = buff[i];
        j++;
    }
    buff[j] = 0;

    buff_i = (PUCHAR)strstr((PCHAR)buff, DEVLIST_NAME);
    if(!buff_i) {
        exit(-5);
    }
/*    if(!WriteFile(h, buff, i, &tmp, NULL) ||
        tmp != l) {
        exit(-6);
    }*/
    k = NtServiceIsRunning(NT_PciDump_SVC_NAME);
    if(k == 1) {
        // Ok
    } else
    if(k == 0) {
        k = NtServiceStart(NT_PciDump_SVC_NAME);
    }
    if(k >= 0) {
        BuildDevList();
    }

    buff_i[0]=0;
    printf("%s", buff);
    printf(";Well-known adapters\n");
    for(k=0; k<NUM_BUSMASTER_ADAPTERS; k++) {
        if(BusMasterAdapters[k].RaidFlags & UNIATA_RAID_CONTROLLER)
            printf(";");
        if(!BusMasterAdapters[k].nRevId) {
            printf("%%PCI\\VEN_%4.4x&DEV_%4.4x.DeviceDesc%%=uniata_Inst, PCI\\VEN_%4.4x&DEV_%4.4x\n", 
                   BusMasterAdapters[k].nVendorId,
                   BusMasterAdapters[k].nDeviceId,
                   BusMasterAdapters[k].nVendorId,
                   BusMasterAdapters[k].nDeviceId
                   );
        } else {
            printf("%%PCI\\VEN_%4.4x&DEV_%4.4x&REV_%2.2x.DeviceDesc%%=uniata_Inst, PCI\\VEN_%4.4x&DEV_%4.4x&REV_%2.2x\n", 
                   BusMasterAdapters[k].nVendorId,
                   BusMasterAdapters[k].nDeviceId,
                   BusMasterAdapters[k].nRevId,
                   BusMasterAdapters[k].nVendorId,
                   BusMasterAdapters[k].nDeviceId,
                   BusMasterAdapters[k].nRevId
                   );
        }
    }
    printf(";Compatible adapters\n");
    for(k=0; k<NewBusmasterCount; k++) {
        printf("%%PCI\\VEN_%4.4x&DEV_%4.4x.DeviceDesc%%=uniata_Inst, PCI\\VEN_%4.4x&DEV_%4.4x\n", 
               NewBusMasterAdapters[k].nVendorId,
               NewBusMasterAdapters[k].nDeviceId,
               NewBusMasterAdapters[k].nVendorId,
               NewBusMasterAdapters[k].nDeviceId
               );
    }
    printf("%s\n", DEVLIST_NAME);
    buff_i+=sizeof(DEVLIST_NAME);
    buff_j=buff_i;
    buff_i = (PUCHAR)strstr((PCHAR)buff_i, DEVNAMELIST_NAME);
    if(!buff_i) {
        exit(-6);
    }
    buff_i[0]=0;
    printf("%s", buff_j);
    printf(";Well-known adapters\n");
    for(k=0; k<NUM_BUSMASTER_ADAPTERS; k++) {
        if(BusMasterAdapters[k].RaidFlags & UNIATA_RAID_CONTROLLER)
            printf(";");
        if(!BusMasterAdapters[k].nRevId) {
            printf("PCI\\VEN_%4.4x&DEV_%4.4x.DeviceDesc=\"%s %s controller%%COMMENT%%\"\n", 
                   BusMasterAdapters[k].nVendorId,
                   BusMasterAdapters[k].nDeviceId,
                   BusMasterAdapters[k].FullDevName,
                   GetAtaTypeStr(k)
                   );
        } else {
            printf("PCI\\VEN_%4.4x&DEV_%4.4x&REV_%2.2x.DeviceDesc=\"%s %s controller%%COMMENT%%\"\n", 
                   BusMasterAdapters[k].nVendorId,
                   BusMasterAdapters[k].nDeviceId,
                   BusMasterAdapters[k].nRevId,
                   BusMasterAdapters[k].FullDevName,
                   GetAtaTypeStr(k)
                   );
        }
    }
    printf(";Compatible adapters\n");
    for(k=0; k<NewBusmasterCount; k++) {
        sprintf(&UnknownDevName[0], UNKNOWN_DEVNAME_TEMPLATE, 
               BusMasterAdapters[k].nVendorId,
               BusMasterAdapters[k].nDeviceId
               );
        printf("PCI\\VEN_%4.4x&DEV_%4.4x.DeviceDesc=\"%s%%COMMENT%%\"\n", 
               BusMasterAdapters[k].nVendorId,
               BusMasterAdapters[k].nDeviceId,
               &UnknownDevName
               );
    }
    printf("%s\n", DEVNAMELIST_NAME);
    buff_i+=sizeof(DEVNAMELIST_NAME);
    printf("%s", buff_i);

    exit(0);
}
