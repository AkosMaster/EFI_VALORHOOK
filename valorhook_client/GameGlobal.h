#pragma once
#include "GameMath.h"
#include <windows.h>
#include <iostream>

#define maxPlayerCount 40
namespace globals {
	extern uint64_t m_base;

	extern uint64_t world;
	extern uint64_t persistent_level;
	extern uint64_t game_instance;
	extern uint64_t local_player_array;
	extern uint64_t local_player;
	extern uint64_t local_player_controller;
	extern uint64_t local_player_pawn;
	extern uint64_t damage_controller;
	extern uint64_t camera_manager;
	extern uint64_t actors;
	extern uint64_t local_root;

	extern uint64_t local_player_state;

	extern uint64_t local_team_component;
	extern int local_teamID;

	extern int cachedPlayerIndexes[maxPlayerCount];
}
#define globalUpdateDelay 1000