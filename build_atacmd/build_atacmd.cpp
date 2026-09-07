/*++

Copyright (c) 2016 Alexander A. Telyatnikov (Alter)

Module Name:
    build_atacmd.cpp

Abstract:
    This is simple utility that constructs static ATA-command
    flags array for use during uniata.sys build

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
#include "..\driver\atapi.h"
#include "..\driver\uniata_ver.h"

UCHAR AtaCommands48[256];
UCHAR AtaCommandFlags[256];

void
UniataInitAtaCommands()
{
    int i;
    UCHAR command;
    UCHAR flags;

    //KdPrint2((PRINT_PREFIX "UniataInitAtaCommands:\n"));

    for(i=0; i<256; i++) {

        flags = 0;
        command = i;

        //KdPrint2((PRINT_PREFIX "cmd %2.2x: ", command));

        switch(command) {
        case IDE_COMMAND_READ_DMA48:
        case IDE_COMMAND_READ_DMA_Q48:
        case IDE_COMMAND_READ_STREAM_DMA48:
        case IDE_COMMAND_READ_STREAM48:
        case IDE_COMMAND_WRITE_DMA48:
        case IDE_COMMAND_WRITE_DMA_Q48:
        case IDE_COMMAND_READ_DMA_Q:
        case IDE_COMMAND_READ_DMA:
        case IDE_COMMAND_WRITE_DMA:
        case IDE_COMMAND_WRITE_DMA_Q:
        case IDE_COMMAND_WRITE_STREAM_DMA48:
        case IDE_COMMAND_WRITE_STREAM48:
        case IDE_COMMAND_WRITE_FUA_DMA48:
        case IDE_COMMAND_WRITE_FUA_DMA_Q48:
        case IDE_COMMAND_READ_LOG_DMA48:
        case IDE_COMMAND_WRITE_LOG_DMA48:
        case IDE_COMMAND_TRUSTED_RCV_DMA:
        case IDE_COMMAND_TRUSTED_SEND_DMA:
        case IDE_COMMAND_DATA_SET_MGMT:
            //KdPrint2((PRINT_PREFIX "DMA "));
            flags |= ATA_CMD_FLAG_DMA;
        }

        switch(command) {
        case IDE_COMMAND_WRITE_FUA_DMA48:
        case IDE_COMMAND_WRITE_FUA_DMA_Q48:
        case IDE_COMMAND_WRITE_MUL_FUA48:

            flags |= ATA_CMD_FLAG_FUA;
            /* FALL THROUGH */

        case IDE_COMMAND_READ48:
        case IDE_COMMAND_READ_DMA48:
        case IDE_COMMAND_READ_DMA_Q48:
        case IDE_COMMAND_READ_MUL48:
        case IDE_COMMAND_READ_STREAM_DMA48:
        case IDE_COMMAND_READ_STREAM48:
        case IDE_COMMAND_WRITE48:
        case IDE_COMMAND_WRITE_DMA48:
        case IDE_COMMAND_WRITE_DMA_Q48:
        case IDE_COMMAND_WRITE_MUL48:
        case IDE_COMMAND_WRITE_STREAM_DMA48:
        case IDE_COMMAND_WRITE_STREAM48:
        case IDE_COMMAND_FLUSH_CACHE48:
        case IDE_COMMAND_VERIFY48:

            //KdPrint2((PRINT_PREFIX "48 "));
            flags |= ATA_CMD_FLAG_48;
            /* FALL THROUGH */

        case IDE_COMMAND_READ:
        case IDE_COMMAND_READ_MULTIPLE:
        case IDE_COMMAND_READ_DMA:
        case IDE_COMMAND_READ_DMA_Q:
        case IDE_COMMAND_WRITE:
        case IDE_COMMAND_WRITE_MULTIPLE:
        case IDE_COMMAND_WRITE_DMA:
        case IDE_COMMAND_WRITE_DMA_Q:
        case IDE_COMMAND_FLUSH_CACHE:
        case IDE_COMMAND_VERIFY:

            //KdPrint2((PRINT_PREFIX "LBA "));
            flags |= ATA_CMD_FLAG_LBAIOsupp;
        }

        switch(command) {
        case IDE_COMMAND_READ_NATIVE_SIZE48:
        case IDE_COMMAND_SET_NATIVE_SIZE48:
            // we cannot set LBA flag for these commands to avoid BadBlock handling
            //flags |= ATA_CMD_FLAG_LBAIOsupp;
            flags |= ATA_CMD_FLAG_48;

        case IDE_COMMAND_READ_NATIVE_SIZE:
        case IDE_COMMAND_SET_NATIVE_SIZE:

            flags |= ATA_CMD_FLAG_LBAIOsupp | ATA_CMD_FLAG_FUA;
        }

        flags |= ATA_CMD_FLAG_48supp;

        switch (command) {
        case IDE_COMMAND_READ:
            command = IDE_COMMAND_READ48; break;
        case IDE_COMMAND_READ_MULTIPLE:
            command = IDE_COMMAND_READ_MUL48; break;
        case IDE_COMMAND_READ_DMA:
            command = IDE_COMMAND_READ_DMA48; break;
        case IDE_COMMAND_READ_DMA_Q:
            command = IDE_COMMAND_READ_DMA_Q48; break;
        case IDE_COMMAND_WRITE:
            command = IDE_COMMAND_WRITE48; break;
        case IDE_COMMAND_WRITE_MULTIPLE:
            command = IDE_COMMAND_WRITE_MUL48; break;
        case IDE_COMMAND_WRITE_DMA:
            command = IDE_COMMAND_WRITE_DMA48; break;
        case IDE_COMMAND_WRITE_DMA_Q:
            command = IDE_COMMAND_WRITE_DMA_Q48; break;
        case IDE_COMMAND_FLUSH_CACHE:
            command = IDE_COMMAND_FLUSH_CACHE48; break;
    //    case IDE_COMMAND_READ_NATIVE_SIZE:
    //            command = IDE_COMMAND_READ_NATIVE_SIZE48; break;
        case IDE_COMMAND_SET_NATIVE_SIZE:
            command = IDE_COMMAND_SET_NATIVE_SIZE48; break;
        case IDE_COMMAND_VERIFY:
            command = IDE_COMMAND_VERIFY48; break;
        default:
            //KdPrint2((PRINT_PREFIX "!28->48 "));
            flags &= ~ATA_CMD_FLAG_48supp;
        }

        switch (command) {
        case IDE_COMMAND_READ:
        case IDE_COMMAND_READ_MULTIPLE:
        case IDE_COMMAND_READ_DMA48:
        case IDE_COMMAND_READ_DMA_Q48:
        case IDE_COMMAND_READ_STREAM_DMA48:
        case IDE_COMMAND_READ_STREAM48:
        case IDE_COMMAND_READ_DMA_Q:
        case IDE_COMMAND_READ_DMA:
        case IDE_COMMAND_READ_LOG_DMA48:
        case IDE_COMMAND_TRUSTED_RCV_DMA:
        case IDE_COMMAND_IDENTIFY:
        case IDE_COMMAND_ATAPI_IDENTIFY:
            //KdPrint2((PRINT_PREFIX "RD "));
            flags |= ATA_CMD_FLAG_In;
            break;
        case IDE_COMMAND_WRITE:
        case IDE_COMMAND_WRITE_MULTIPLE:
        case IDE_COMMAND_WRITE_DMA48:
        case IDE_COMMAND_WRITE_DMA_Q48:
        case IDE_COMMAND_WRITE_DMA:
        case IDE_COMMAND_WRITE_DMA_Q:
        case IDE_COMMAND_WRITE_STREAM_DMA48:
        case IDE_COMMAND_WRITE_STREAM48:
        case IDE_COMMAND_WRITE_FUA_DMA48:
        case IDE_COMMAND_WRITE_FUA_DMA_Q48:
            //KdPrint2((PRINT_PREFIX "WR "));
            flags |= ATA_CMD_FLAG_Out;
            break;
        }

        //KdPrint2((PRINT_PREFIX "\t -> %2.2x (%2.2x)\n", command, flags));
        AtaCommands48[i]   = command;
        AtaCommandFlags[i] = flags;
    }
} // end UniataInitAtaCommands()

void main (int argc, char* argv[]) {

    int i;
    UniataInitAtaCommands();

    printf("\n// Build Version 0." UNIATA_VER_STR "\n\n");
    printf("\nUCHAR const AtaCommands48[256] = {\n");
    for(i=0; i<256; i++) {
        if(i) {
            printf(", ");
        }
        printf("0x%x", AtaCommands48[i]);
        if(i % 16 == 15) {
            printf("\n");
        }
    }
    printf("};\n\n");

    printf("\nUCHAR const AtaCommandFlags[256] = {\n");
    for(i=0; i<256; i++) {
        if(i) {
            printf(", ");
        }
        printf("0x%x", AtaCommandFlags[i]);
        if(i % 16 == 15) {
            printf("\n");
        }
    }
    printf("};\n\n");

    exit(0);
} // end main()
