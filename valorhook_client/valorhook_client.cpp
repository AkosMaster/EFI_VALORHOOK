#include <iostream>

#include "DriverController.h"
#include "ProcessManager.h"

#include "GameGlobal.h"
using namespace driverController;

#include "GameMath.h"



#define IMGUI_DEFINE_MATH_OPERATORS

void printGlobals() {
	std::cout << "m_base: " << (void*)globals::m_base << std::endl;
	std::cout << "[?] world: 0x" << (void*)globals::world << std::endl;
	std::cout << "[?] persistent_level: 0x" << (void*)globals::persistent_level << std::endl;
	std::cout << "[?] game_instance: 0x" << (void*)globals::game_instance << std::endl;
	std::cout << "[?] local_player_array: 0x" << (void*)globals::local_player_array << std::endl;
	std::cout << "[?] local_player: 0x" << (void*)globals::local_player << std::endl;
	std::cout << "[?] local_player_controller: 0x" << (void*)globals::local_player_controller << std::endl;
	std::cout << "[?] local_player_pawn: 0x" << (void*)globals::local_player_pawn << std::endl;
	std::cout << "[?] camera_manager: 0x" << (void*)globals::camera_manager << std::endl;
	std::cout << "[?] local_root: 0x" << (void*)globals::local_root << std::endl;
	std::cout << "[?] actors: 0x" << (void*)globals::actors << std::endl;
}

/* define globals, accessible from GameGlobal.h */
namespace globals {
	uint64_t m_base;

	uint64_t world;
	uint64_t persistent_level;
	uint64_t game_instance;
	uint64_t local_player_array;
	uint64_t local_player;
	uint64_t local_player_controller;
	uint64_t local_player_pawn;
	uint64_t damage_controller;
	uint64_t camera_manager;
	uint64_t actors;
	uint64_t local_root;

	uint64_t local_player_state;

	uint64_t local_team_component;
	int local_teamID;

	int cachedPlayerIndexes[maxPlayerCount] = { 0 };
}

void updatePlayers() {
	int actor_count = read<int>(globals::persistent_level + 0xB8);
	int foundPlayerCount = 0;

	for (uint64_t i = 0; i < actor_count; i++)
	{
		if (foundPlayerCount >= maxPlayerCount)
			break;

		std::uint64_t actor = read<uint64_t>(globals::actors + i * 0x8);
		if (!actor || actor == globals::local_player_pawn)
			continue;

		int classID = read<int>(actor + 0x3C);

		if (classID != 16777502)
			continue;
		foundPlayerCount++;

		globals::cachedPlayerIndexes[foundPlayerCount] = i;
	}
}

int main() {

	HANDLE hdl = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(startOverlayProcess), nullptr, NULL, nullptr);
	CloseHandle(hdl);

	if (!isDriverRunning()) {
		std::cout << "[-] Driver is not running" << std::endl;
		return -1;
	}
	int valorantPID = getValorantProcId();

	std::cout << "[?] PID: " << valorantPID << std::endl;

	globals::m_base = setTargetPid(valorantPID);

	if (!globals::m_base) {
		std::cout << "[-] Valorant is not running" << std::endl;
		return -1;
	}

	while (true) {
		globals::world = read<uint64_t>(globals::m_base + OFFSET_WORLD);
		globals::persistent_level = read<uint64_t>(globals::world + PersistentLevel);
		globals::game_instance = read<uint64_t>(globals::world + OwningGameInstance);
		globals::local_player_array = read<uint64_t>(globals::game_instance + LocalPlayers);
		globals::local_player = read<uint64_t>(globals::local_player_array);
		globals::local_player_controller = read<uint64_t>(globals::local_player + PlayerController);
		globals::local_player_pawn = read<uint64_t>(globals::local_player_controller + AcknowledgedPawn);
		globals::camera_manager = read<uint64_t>(globals::local_player_controller + PlayerCameraManager);
		globals::local_root = read<uint64_t>(globals::local_player_pawn + RootComponent);
		globals::actors = read<uint64_t>(globals::persistent_level + 0xB0);
		globals::damage_controller = read<uint64_t>(globals::local_player_pawn + DamageHandler);

		globals::local_player_state = read<uint64_t>(globals::local_player_pawn + PlayerState);

		globals::local_team_component = read<uint64_t>(globals::local_player_state + TeamComponent);
		globals::local_teamID = read<int>(globals::local_team_component + 0x118);

		updatePlayers();
	}

	return 0;
}