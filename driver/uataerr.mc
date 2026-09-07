;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1992, 1993  Microsoft Corporation
;
;Module Name:
;
;    ntiologc.h
;
;Abstract:
;
;    Constant definitions for the I/O error code log values.
;
;Author:
;
;    Tony Ercolano (Tonye) 12-23-1992
;
;Revision History:
;
;--*/
;
;#ifndef _UATALOG_
;#define _UATALOG_
;
;//
;//  Status values are 32 bit values layed out as follows:
;//
;//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
;//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
;//  +---+-+-------------------------+-------------------------------+
;//  |Sev|C|       Facility          |               Code            |
;//  +---+-+-------------------------+-------------------------------+
;//
;//  where
;//
;//      Sev - is the severity code
;//
;//          00 - Success
;//          01 - Informational
;//          10 - Warning
;//          11 - Error
;//
;//      C - is the Customer code flag
;//
;//      Facility - is the facility code
;//
;//      Code - is the facility's status code
;//
;
MessageIdTypedef=NTSTATUS


MessageId=0x0001 SymbolicName=SP_BUS_PARITY_ERROR
Language=English
Bus Parity Error.
.

MessageId=0x0002 SymbolicName=SP_UNEXPECTED_DISCONNECT
Language=English
Unexpected disconnect.
.

MessageId=0x0003 SymbolicName=SP_INVALID_RESELECTION
Language=English
Invalid reselection.
.

MessageId=0x0004 SymbolicName=SP_BUS_TIME_OUT
Language=English
Bus timeout.
.

MessageId=0x0005 SymbolicName=SP_PROTOCOL_ERROR
Language=English
Protocol Error.
.

MessageId=0x0006 SymbolicName=SP_INTERNAL_ADAPTER_ERROR
Language=English
Internal adapter Error.
.

MessageId=0x0007 SymbolicName=SP_REQUEST_TIMEOUT
Language=English
Request timeout.
.

MessageId=0x0008 SymbolicName=SP_IRQ_NOT_RESPONDING
Language=English
IRQ not responding.
.

MessageId=0x0009 SymbolicName=SP_BAD_FW_WARNING
Language=English
Firmware warning.
.

MessageId=0x000a SymbolicName=SP_BAD_FW_ERROR
Language=English
Firmware error.
.

MessageId=0x000b SymbolicName=SP_LOST_WMI_MINIPORT_REQUEST
Language=English
Lost WMI request.
.



;#endif /* _UATALOG_ */
