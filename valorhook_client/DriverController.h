#include <iostream>
#include <windows.h>

#define COMMAND_MAGIC 0xDEADBEEF

#define IOCTL_CODE 0x800

// Device type
#define SIOCTL_TYPE 40000

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
#define IOCTL_MEMORY_COMMAND\
 CTL_CODE( SIOCTL_TYPE, IOCTL_CODE, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)

namespace driverController {
    struct memory_command {
        INT operation = 0;

        DWORD64 magic = 0;

        DWORD64 retval = 0;

        DWORD64 memaddress = 0;
        DWORD64 length = 0;
        PVOID buffer = 0;
    };

    bool sendCommand(memory_command* cmd);
    void crashWindows();
    bool isDriverRunning();
    DWORD64 setTargetPid(int PID);

    void readTo(DWORD64 address, void* buffer, DWORD64 len);
    void writeTo(DWORD64 address, void* buffer, DWORD64 len);

    template <typename T>
    T read(DWORD64 address) {
        T buffer{};
        readTo(address, &buffer, sizeof(T));
        return buffer;
    }

    template <typename T>
    void write(DWORD64 address, T buffer) {
        writeTo(address, &buffer, sizeof(T));
    }
}