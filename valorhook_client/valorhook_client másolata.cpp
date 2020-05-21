#include "DriverController.h"
#include "process.h"

#include "tlhelp32.h"
#include "tchar.h"

struct vec3 {
	float x;
	float y;
	float z;
};

uint64_t world = 0;
uint64_t persistent_level;
uint64_t game_instance;
uint64_t local_player_array;
uint64_t local_player;
uint64_t local_player_controller;
uint64_t local_player_pawn;
uint64_t damage_controller;
float health;
uint64_t camera_manager;
vec3 camera_position;
vec3 camera_rotation;
float camera_fov;
uint64_t actors;
int actor_count;
vec3 LocalPos;
uint64_t LocalRoot;

#define OFFSET_WORLD 0x627ced8

int GetProcessThreadNumByID(DWORD dwPID)
{
	//获取进程信息
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(pe32);
	BOOL bRet = ::Process32First(hProcessSnap, &pe32);;
	while (bRet)
	{
		if (pe32.th32ProcessID == dwPID)
		{
			::CloseHandle(hProcessSnap);
			return pe32.cntThreads;
		}
		bRet = ::Process32Next(hProcessSnap, &pe32);
	}
	return 0;
}

int getValorantProcId() {
	DWORD dwRet = 0;
	DWORD dwThreadCountMax = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &pe32);
	do
	{
		if (_tcsicmp(pe32.szExeFile, _T("VALORANT-Win64-Shipping.exe")) == 0)

		{
			DWORD dwTmpThreadCount = GetProcessThreadNumByID(pe32.th32ProcessID);

			if (dwTmpThreadCount > dwThreadCountMax)
			{
				dwThreadCountMax = dwTmpThreadCount;
				dwRet = pe32.th32ProcessID;
			}
		}
	} while (Process32Next(hSnapshot, &pe32));
	CloseHandle(hSnapshot);
	return dwRet;
}


int main() {

	if (!driverController::isDriverRunning()) {
		std::cout << "[-] Driver is not running" << std::endl;
		return -1;
	}

	int valorantPID = getValorantProcId();

	std::cout << "[?] PID: " << valorantPID << std::endl;

	return -1;
	uint64_t m_base = driverController::setTargetPid(valorantPID);

	if (!m_base) {
		std::cout << "[-] Valorant is not running" << std::endl;
		return -1;
	}

	std::cout << "m_base: " << (void*)m_base << std::endl;

	system("pause");

	driverController::read(m_base + OFFSET_WORLD, &world, sizeof(uint64_t));

	driverController::read(world + 0x38, &persistent_level, sizeof(uint64_t));
	driverController::read(world + 0x180, &game_instance, sizeof(uint64_t));

	driverController::read(game_instance + 0x40, &local_player_array, sizeof(uint64_t));
	driverController::read(local_player_array, &local_player, sizeof(uint64_t));

	driverController::read(local_player + 0x38, &local_player_controller, sizeof(uint64_t));
	driverController::read(local_player_controller + 0x518, &local_player_pawn, sizeof(uint64_t));

	driverController::read(local_player_controller + 0x530, &camera_manager, sizeof(uint64_t));
	
	std::cout << "[?] world: 0x" << (void*)world << std::endl;

	return 0;
}