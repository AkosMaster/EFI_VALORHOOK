#include "DriverController.h"

bool driverController::sendCommand(memory_command* cmd) {
    HANDLE hDevice;
    DWORD dwBytesRead = 0;
    memory_command* new_cmd = new memory_command();

    hDevice = CreateFile(L"\\\\.\\ValorMemory", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    cmd->magic = COMMAND_MAGIC;
    DeviceIoControl(hDevice, IOCTL_MEMORY_COMMAND, cmd, sizeof(struct memory_command), new_cmd, sizeof(struct memory_command), &dwBytesRead, NULL);

    *cmd = *(struct memory_command*)new_cmd;

    CloseHandle(hDevice);

    return true;
}

void driverController::crashWindows() {
    memory_command* cmd = new memory_command();
    cmd->operation = 10; // find game process

    sendCommand(cmd);
}

bool driverController::isDriverRunning() {
    HANDLE hDevice = CreateFile(L"\\\\.\\ValorMemory", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    return hDevice != INVALID_HANDLE_VALUE;
}

DWORD64 driverController::setTargetPid(int PID) {
    memory_command* cmd = new memory_command();
    cmd->operation = 2; // find game process
    cmd->retval = PID;

    sendCommand(cmd);

    return cmd->retval;
}

void driverController::readTo(DWORD64 address, void* buffer, DWORD64 len) {

    memory_command* cmd = new memory_command();
    cmd->operation = 0; // read byte

    cmd->buffer = buffer;
    cmd->length = len;

    cmd->memaddress = address;

    sendCommand(cmd);
}

void driverController::writeTo(DWORD64 address, void* buffer, DWORD64 len) {
    memory_command* cmd = new memory_command();
    cmd->operation = 1; // write byte

    cmd->buffer = buffer;
    cmd->length = len;

    cmd->memaddress = address;

    sendCommand(cmd);
}