#include "SvcManLib.h"

UINT
NtServiceIsRunning(
    LPCTSTR ServiceName
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    SERVICE_STATUS  ssStatus;
    UINT            return_value;

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    QueryServiceStatus(schService, &ssStatus);
    if(ssStatus.dwCurrentState == SERVICE_RUNNING) return_value = 1;
                                              else return_value = 0;
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

UINT
NtServiceStart(
    LPCTSTR ServiceName
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    return_value = StartService(schService, 0, NULL) ? 1 : -1;
    RC = GetLastError();
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

UINT
NtServiceInstall(
    LPCTSTR ServiceName,
    PCHAR   PathToExecutable
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;
    CHAR path_name1[MAX_PATH];
    CHAR path_name[MAX_PATH];
    CHAR sys_path_name[MAX_PATH];
    CHAR drv_path_name[MAX_PATH];

    GetSystemDirectory(sys_path_name, MAX_PATH-40);
    _snprintf(path_name,     MAX_PATH-1, "%s\\drivers\\%s.sys", sys_path_name, ServiceName);
    _snprintf(path_name1,    MAX_PATH-1, "%s\\%s.sys", PathToExecutable, ServiceName);
    _snprintf(drv_path_name, MAX_PATH-1, "System32\\drivers\\%s.sys", ServiceName);

    return_value = CopyFile(path_name1, path_name, 0);
    return_value = GetLastError();

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = CreateService(schSCManager, ServiceName, ServiceName, SERVICE_ALL_ACCESS,
                                SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
                                drv_path_name, NULL, NULL, NULL, NULL, NULL);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    return_value = StartService(schService, 0, NULL) ? 1 : -1;
    return_value = GetLastError();
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

