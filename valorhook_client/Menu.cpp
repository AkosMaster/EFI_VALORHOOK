#include "DriverController.h"
#include "GameMath.h"
#include "GameGlobal.h"

using namespace driverController;

bool SETTINGS_LEGITBOT_ENABLED;
float SETTINGS_LEGITBOT_SPEED;

bool SETTINGS_ESP_ENABLED;
bool SETTINGS_ESP_TEAMCHECK;
bool SETTINGS_ESP_NOALIVECHECK;
bool SETTINGS_ESP_RADAR;

bool SETTINGS_ESP_BOX;
bool SETTINGS_ESP_BOX_LINE;
bool SETTINGS_ESP_BOX_CORNERONLY;
bool SETTINGS_ESP_BOX_DISPLAYHP;

bool SETTINGS_MISC_BULLET_IMPACT;

Color ColorPalette;

void renderMenu() {
	ImGui::Begin("Legitbot"); {

		ImGui::Checkbox("Enabled", &SETTINGS_LEGITBOT_ENABLED);
		ImGui::SliderFloat("Speed", &SETTINGS_LEGITBOT_SPEED, 0.1, 10);

		ImGui::End();
	}

	ImGui::Begin("ESP"); {

		ImGui::Checkbox("Enabled", &SETTINGS_ESP_ENABLED);
		ImGui::Checkbox("Team Check", &SETTINGS_ESP_TEAMCHECK);
		ImGui::Checkbox("Show Dead Players", &SETTINGS_ESP_NOALIVECHECK);

		ImGui::Checkbox("Radar", &SETTINGS_ESP_RADAR);

		ImGui::Checkbox("Box ESP", &SETTINGS_ESP_BOX);

		if (SETTINGS_ESP_BOX) {
			ImGui::Begin("ESP->Box"); {
				ImGui::Checkbox("Draw Line to box", &SETTINGS_ESP_BOX_LINE);
				ImGui::Checkbox("Corner only", &SETTINGS_ESP_BOX_CORNERONLY);
				ImGui::Checkbox("Display Health", &SETTINGS_ESP_BOX_DISPLAYHP);
				ImGui::End();
			}
		}

		ImGui::End();
	}
}

bool validScreenPos(Vector3 pos) {
	if (pos.x != pos.x || pos.y != pos.y)
		return false;
	if (pos.x < 0 || pos.x >= ScreenInfo::Width)
		return false;
	if (pos.y < 0 || pos.y >= ScreenInfo::Height)
		return false;
	return true;
}

#define radarSize 300

//classID 16777486 -> bulletImpact
//classID 16777502 -> bot

int tickCount = 0;
void actorLoop(ImVec2 radarWindowMid) {

	Vector2 radarVecPos = Vector2(radarWindowMid.x, radarWindowMid.y);

	Vector3 camera_position = read<Vector3>(globals::camera_manager + 0x11D0);
	Vector3 camera_rotation = read<Vector3>(globals::camera_manager + 0x11DC);

	float camera_fov = read<float>(globals::camera_manager + 0x11E8);

	for (uint64_t i = 0; i < maxPlayerCount; i++)
	{
		std::uint64_t actor = read<uint64_t>(globals::actors + globals::cachedPlayerIndexes[i] * 0x8);

		if (!actor || actor == globals::local_player_pawn)
			continue;

		int classID = read<int>(actor + 0x3C);

		if (classID != 16777502)
			continue;

		bool is_dormant = read<bool>(actor + 0x100);
		if (is_dormant)
			continue;

		uint64_t actor_state = read<uint64_t>(actor + PlayerState);

		uint64_t team_component = read<uint64_t>(actor_state + TeamComponent);
		int teamID = read<int>(team_component + 0x118);

		uint64_t root_component = read<uint64_t>(actor + RootComponent);
		Vector3 root_position = read<Vector3>(root_component + RelativeLocation);

		Vector3 relativePosition = root_position - camera_position;
		float relDistance = relativePosition.Length() / 10000 * 2;

		if (SETTINGS_ESP_ENABLED) {

			uint64_t damage_handler = read<uint64_t>(actor + DamageHandler);
			float health = read<float>(damage_handler + 0x190);

			if ((health <= 0 || health > 100) && !SETTINGS_ESP_NOALIVECHECK)
				continue;

			if (SETTINGS_ESP_TEAMCHECK && teamID == globals::local_teamID)
				continue;

			if (SETTINGS_ESP_BOX) {
				Vector3 screenPos = WorldToScreen(root_position, camera_position, camera_rotation, camera_fov);
				if (!validScreenPos(screenPos))
					continue;

				int boxWidth = 15 / relDistance;
				int boxHeight = 35 / relDistance;

				RGBA* boxColor = teamID == globals::local_teamID ? &ColorPalette.grayblue : &ColorPalette.peachred;

				/*
				BYTE visible = read<BYTE>(root_component + 0x200);
				if (!visible)
					boxColor = &ColorPalette.green;
				*/

				if (SETTINGS_ESP_BOX_CORNERONLY) {
					DrawCornerBox(screenPos.x - boxWidth / 2, screenPos.y - boxHeight / 2, boxWidth, boxHeight, 1, boxColor);
					DrawCornerBox(screenPos.x - boxWidth / 2 - 1, screenPos.y - boxHeight / 2 - 1, boxWidth, boxHeight, 1, &ColorPalette.black);
					DrawCornerBox(screenPos.x - boxWidth / 2 + 1, screenPos.y - boxHeight / 2 + 1, boxWidth, boxHeight, 1, &ColorPalette.black);
				}
				else {
					DrawRect(screenPos.x - boxWidth / 2, screenPos.y - boxHeight / 2, boxWidth, boxHeight, boxColor, 1);
					DrawRect(screenPos.x - boxWidth / 2 - 1, screenPos.y - boxHeight / 2 - 1, boxWidth, boxHeight, &ColorPalette.black, 1);
					DrawRect(screenPos.x - boxWidth / 2 + 1, screenPos.y - boxHeight / 2 + 1, boxWidth, boxHeight, &ColorPalette.black, 1);
				}
				
				if (SETTINGS_ESP_BOX_LINE) {
					DrawLine(ScreenInfo::Width/2, ScreenInfo::Height, screenPos.x, screenPos.y + boxHeight/2, boxColor, 1);
					DrawLine(ScreenInfo::Width / 2-1, ScreenInfo::Height, screenPos.x-1, screenPos.y + boxHeight / 2, &ColorPalette.black, 1);
					DrawLine(ScreenInfo::Width / 2+1, ScreenInfo::Height, screenPos.x+1, screenPos.y + boxHeight / 2, &ColorPalette.black, 1);
				}

				if (SETTINGS_ESP_BOX_DISPLAYHP) {
					// long ass math
					int hpBox_width = 1 / relDistance;

					int hpBox_x = screenPos.x - boxWidth / 2 - 5 - hpBox_width;
					int hpBox_y = screenPos.y - boxHeight / 2 + (boxHeight - boxHeight * (health / 100));
					
					int hpBox_height = boxHeight * (health / 100);

					DrawFilledRect(hpBox_x, hpBox_y, hpBox_width, hpBox_height, &ColorPalette.green);
					DrawRect(hpBox_x - 1, hpBox_y - 1, hpBox_width + 2, hpBox_height + 2, &ColorPalette.black, 1);
				}

				BYTE isVisible = read<BYTE>(actor + 0x200);

				//DrawStrokeText(screenPos.x, screenPos.y, &ColorPalette.graygreen, std::to_string(isVisible).c_str());
			}

			if (SETTINGS_ESP_RADAR) {
#define zoom 24
				Vector2 dotPos = Vector2(radarWindowMid.x + (relativePosition.x / zoom), radarWindowMid.y + (relativePosition.y / zoom));

				float radarDistance = radarVecPos.Distance(dotPos);

				if (radarDistance > radarSize / 2 - 18 - 4)
					continue;

				float angle = camera_rotation.y + 90; //face forwards

				dotPos = rotatePointAroundPivot(radarWindowMid.x, radarWindowMid.y, deg2rad(-angle), dotPos);

				RGBA* dotColor = teamID == globals::local_teamID ? &ColorPalette.grayblue : &ColorPalette.peachred;
				DrawCircleFilled(dotPos.x, dotPos.y, 4, dotColor, 8);
			}
		}
	}

	//std::cout << foundPlayerCount << std::endl;
}

void renderExtraGui() {

	ImVec2 radarWindowMid;

	if (SETTINGS_ESP_RADAR && SETTINGS_ESP_ENABLED) {
		ImGui::SetNextWindowSize(ImVec2(radarSize,radarSize));
		ImGui::SetNextWindowCollapsed(false);
		ImGui::Begin("ESP->Radar"); {
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowMid = windowPos;
			windowMid.x += radarSize / 2;
			windowMid.y += radarSize / 2 + 10;
			
			radarWindowMid = windowMid;

			DrawCircleFilled(windowMid.x, windowMid.y, radarSize / 2 - 20, &ColorPalette.SilverWhite, 100);
			DrawCircleFilled(windowMid.x, windowMid.y, 3, &ColorPalette.black, 8);

			ImGui::End();
		}
	}

	actorLoop(radarWindowMid);
}