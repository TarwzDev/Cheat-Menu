#include "pch.h"
#include "Menu.h"
#include "Teleport.h"
#include "Weapon.h"

bool Menu::overlay::coord = false;
bool Menu::overlay::fps = false;
bool Menu::overlay::loc_name = false;
bool Menu::overlay::transparent = false;
bool Menu::overlay::veh_health = false;
bool Menu::overlay::veh_speed = false;
std::vector<std::string> Menu::overlay::pos_names{ "Custom", "Top left", "Top right", "Bottom left", "Bottom right" };
int Menu::overlay::selected_pos = 4;
float Menu::overlay::posX = NULL;
float Menu::overlay::posY = NULL;

int Menu::hotkey::command_window[]{ VK_LMENU ,VK_KEY_C };
int Menu::hotkey::menu_open[]{VK_LCONTROL ,VK_KEY_M};
int Menu::hotkey::aim_skin_changer[]{ VK_RETURN ,VK_RETURN };
int Menu::hotkey::quick_ss[]{ VK_LCONTROL ,VK_KEY_S };
int Menu::hotkey::quick_tp[]{ VK_KEY_X ,VK_KEY_Y };

bool Menu::commands::show_menu = false;
char Menu::commands::input_buffer[INPUT_BUFFER_SIZE] = "";

Menu::Menu()
{
	Events::initGameEvent += []
	{
		// improve these later, use struct
		// Load config data
		overlay::coord = config.GetValue("overlay.coord", false);
		overlay::fps = config.GetValue("overlay.fps", false);
		overlay::loc_name = config.GetValue("overlay.loc_name", false);
		overlay::transparent = config.GetValue("overlay.transparent", false);
		overlay::veh_health = config.GetValue("overlay.veh_health", false);
		overlay::veh_speed = config.GetValue("overlay.veh_speed", false);
		overlay::selected_pos = config.GetValue("overlay.selected_pos", 4);
		overlay::posX = config.GetValue("overlay.posX", 0);
		overlay::posY = config.GetValue("overlay.posY", 0);

		// Hotkeys
		hotkey::aim_skin_changer[0] = config.GetValue("hotkey.aim_skin_changer.key1", VK_RETURN);
		hotkey::aim_skin_changer[1] = config.GetValue("hotkey.aim_skin_changer.key2", VK_RETURN);

		hotkey::quick_ss[0] = config.GetValue("hotkey.quick_screenshot.key1", VK_LCONTROL);
		hotkey::quick_ss[1] = config.GetValue("hotkey.quick_screenshot.key2", VK_KEY_S);

		hotkey::quick_tp[0] = config.GetValue("hotkey.quick_tp.key1", VK_KEY_X);
		hotkey::quick_tp[1] = config.GetValue("hotkey.quick_tp.key2", VK_KEY_Y);

		hotkey::menu_open[0] = config.GetValue("hotkey.menu_open.key1", VK_LCONTROL);
		hotkey::menu_open[1] = config.GetValue("hotkey.menu_open.key2", VK_KEY_M);

		hotkey::command_window[0] = config.GetValue("hotkey.command_window.key1", VK_LMENU);
		hotkey::command_window[1] = config.GetValue("hotkey.command_window.key2", VK_KEY_C);
	};

	Events::shutdownRwEvent += []
	{
		// save config data 
		config.SetValue("overlay.coord", overlay::coord);
		config.SetValue("overlay.fps", overlay::fps);
		config.SetValue("overlay.loc_name", overlay::loc_name);
		config.SetValue("overlay.transparent", overlay::transparent);
		config.SetValue("overlay.veh_health", overlay::veh_health);
		config.SetValue("overlay.veh_speed", overlay::veh_speed);
		config.SetValue("overlay.selected_pos", overlay::selected_pos);
		config.SetValue("overlay.posX", overlay::posX);
		config.SetValue("overlay.posY", overlay::posY);

		// Hotkeys
		config.SetValue("hotkey.aim_skin_changer.key1", hotkey::aim_skin_changer[0]);
		config.SetValue("hotkey.aim_skin_changer.key2", hotkey::aim_skin_changer[1]);
							   
		config.SetValue("hotkey.quick_screenshot.key1", hotkey::quick_ss[0]);
		config.SetValue("hotkey.quick_screenshot.key2", hotkey::quick_ss[1]);
							   
		config.SetValue("hotkey.quick_tp.key1", hotkey::quick_tp[0]);
		config.SetValue("hotkey.quick_tp.key2", hotkey::quick_tp[1]);
							   
		config.SetValue("hotkey.menu_open.key1", hotkey::menu_open[0]);
		config.SetValue("hotkey.menu_open.key2", hotkey::menu_open[1]);
							   
		config.SetValue("hotkey.command_window.key1", hotkey::command_window[0]);
		config.SetValue("hotkey.command_window.key2", hotkey::command_window[1]);
	};
}

Menu::~Menu()
{
}

void Menu::ProcessOverlay()
{
	CPlayerPed *player = FindPlayerPed();
	bool show_menu = overlay::coord || overlay::fps || overlay::loc_name || 
		((overlay::veh_health || overlay::veh_speed) && player->m_pVehicle && player->m_pVehicle->IsDriver(player));

	int corner = overlay::selected_pos - 1;
	const float offset = 10.0f;
	ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (corner != -1)
	{
		window_flags |= ImGuiWindowFlags_NoMove;
		ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - offset : offset, (corner & 2) ? io.DisplaySize.y - offset : offset);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	}
	else
	{
		if (overlay::posX != NULL && overlay::posY != NULL)
			ImGui::SetNextWindowPos(ImVec2(overlay::posX, overlay::posY), ImGuiCond_Once);
	}

	ImGui::SetNextWindowBgAlpha(overlay::transparent ? 0.0f : 0.5f);

	if (show_menu && ImGui::Begin("Overlay", NULL, window_flags))
	{
		CPlayerPed *player = FindPlayerPed();
		
		if (overlay::coord)
		{
			CVector pos = player->GetPosition();

			std::string text = "Coord: " + std::to_string(int(pos.x)) + ", " + std::to_string(int(pos.y)) + ", " 
				+ std::to_string(int(pos.z));

			ImGui::Text(text.c_str());
		}

		if (overlay::fps)
			ImGui::Text((std::string("Frames: ") + std::to_string(int(io.Framerate))).c_str());

		if (overlay::loc_name)
			ImGui::Text((std::string("Location: ") + Util::GetLocationName(&player->GetPosition())).c_str());

		if (player->m_pVehicle && player->m_pVehicle->IsDriver(player))
		{
			if (overlay::veh_health)
				ImGui::Text((std::string("Veh Health: ") + std::to_string(int(player->m_pVehicle->m_fHealth))).c_str());

			if (overlay::veh_speed)
			{
				int speed = player->m_pVehicle->m_vecMoveSpeed.Magnitude()*50; // 02E3 - GET_CAR_SPEED
				ImGui::Text((std::string("Veh Speed: ") + std::to_string(speed)).c_str());
			}
		}

		ImVec2 pos = ImGui::GetWindowPos();
		overlay::posX = pos.x;
		overlay::posY = pos.y;

		ImGui::End();
	}
}

void Menu::ProcessShortcutsWindow()
{
		int resX = int(screen::GetScreenWidth());
		int resY = int(screen::GetScreenHeight());

		ImGui::SetNextWindowPos(ImVec2(0, resY - 40), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(resX, 40));

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration + ImGuiWindowFlags_AlwaysAutoResize + ImGuiWindowFlags_NoSavedSettings
			+ ImGuiWindowFlags_NoMove;

		if (ImGui::Begin("Shortcuts window", NULL, flags))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, resY / 130));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3, 3));
			
			ImGui::SetNextItemWidth(resX);
			ImGui::SetKeyboardFocusHere(-1);

			ImGui::InputTextWithHint("##TEXTFIELD", "Enter command", commands::input_buffer, INPUT_BUFFER_SIZE);
			
			ImGui::PopStyleVar(2);
			ImGui::End();
		}
}

void Menu::ProcessCommands()
{
	std::stringstream ss(commands::input_buffer);

	std::string command;
	ss >> command;

	if (command == "hp")
	{
		try
		{
			std::string temp;
			ss >> temp;
			FindPlayerPed()->m_fHealth = std::stof(temp);
		}
		catch (...) {
			CHud::SetHelpMessage("Invalid value", false, false, false);
		}
	}

	if (command == "time")
	{
		try 
		{
			std::string temp;
			ss >> temp;
			CClock::ms_nGameClockHours = std::stoi(temp);

			ss >> temp;
			CClock::ms_nGameClockMinutes = std::stoi(temp);
		}
		catch (...) {
			CHud::SetHelpMessage("Invalid value", false, false, false);
		}
	}

	if (command == "tp")
	{
		try {
			CVector pos;
			std::string temp;

			ss >> temp;
			pos.x = std::stof(temp);

			ss >> temp;
			pos.y = std::stof(temp);

			ss >> temp;
			pos.z = std::stof(temp);

			Teleport::TeleportPlayer(false, &pos, 0);
		}
		catch (...) {
			CHud::SetHelpMessage("Invalid location", false, false, false);
		}
	}

	if (command == "wep")
	{ 
		std::string wep_name;
		ss >> wep_name;

		if (wep_name == "jetpack")
		{
			Weapon::GiveWeaponToPlayer(std::string("-1"));
			CHud::SetHelpMessage("Weapon given", false, false, false);
		}
		else
		{
			eWeaponType weapon = CWeaponInfo::FindWeaponType((char*)wep_name.c_str());
			CWeaponInfo* pweaponinfo = CWeaponInfo::GetWeaponInfo(weapon, 1);

			if (wep_name != "" && pweaponinfo->m_nModelId1 != -1)
			{
				Weapon::GiveWeaponToPlayer(std::to_string(weapon));
				CHud::SetHelpMessage("Weapon given", false, false, false);
			}
			else
				CHud::SetHelpMessage("Invalid command", false, false, false);
		}
		
		return;
	}
}

void Menu::Main()
{
	if (ImGui::BeginTabBar("Menu", ImGuiTabBarFlags_NoTooltip + ImGuiTabBarFlags_FittingPolicyScroll))
	{
		if (ImGui::BeginTabItem("Overlay"))
		{
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SameLine();
			Ui::ListBox("Overlay", overlay::pos_names, overlay::selected_pos);
			ImGui::Spacing();

			ImGui::Columns(2, NULL, false);
			ImGui::Checkbox("No background", &overlay::transparent);
			ImGui::Checkbox("Show coordinates", &overlay::coord);
			ImGui::Checkbox("Show FPS", &overlay::fps);
			ImGui::NextColumn();

			ImGui::Checkbox("Show location", &overlay::loc_name);
			ImGui::Checkbox("Show veh health", &overlay::veh_health);
			ImGui::Checkbox("Show veh speed", &overlay::veh_speed);
			ImGui::Columns(1);
			
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Hotkeys"))
		{
			ImGui::Spacing();
			ImGui::BeginChild("Hotkeys");
			Ui::HotKey("Open/ close cheat menu", hotkey::menu_open);
			Ui::HotKey("Open/ close command window", hotkey::command_window);

			ImGui::Dummy(ImVec2(0,10));

			Ui::HotKey("Activate aim skin changer", hotkey::aim_skin_changer);
			Ui::HotKey("Take quick screenshot", hotkey::quick_ss);
			Ui::HotKey("Toggle quick teleport", hotkey::quick_tp);

			ImGui::Dummy(ImVec2(0, 10));

			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Commands"))
		{
			if (ImGui::BeginChild("CommandsChild"))
			{
				ImGui::TextWrapped(std::string("Open or close command window using " + Ui::GetHotKeyNameString(hotkey::command_window)).c_str());
				ImGui::Spacing();
				if (ImGui::CollapsingHeader("Set health"))
				{
					ImGui::Spacing();
					ImGui::TextWrapped("Set player health.\nExample: hp (health).");
					ImGui::Spacing();
					ImGui::Separator();
				}
				if (ImGui::CollapsingHeader("Set time"))
				{
					ImGui::Spacing();
					ImGui::TextWrapped("Set current game time.\nExample: time (hour) (minute).\n");
					ImGui::TextWrapped("Writing something like 'time 12' would be interpreted as 'time 12 12'");
					ImGui::Spacing();
					ImGui::Separator();
				}
				if (ImGui::CollapsingHeader("Quick vehicle spawner"))
				{
					ImGui::Spacing();
					ImGui::TextWrapped("Spawn vehicles by typing their model names.\nExample: veh (veh_name)");
					ImGui::Spacing();
					ImGui::Separator();
				}
				if (ImGui::CollapsingHeader("Quick weapon spawner"))
				{
					ImGui::Spacing();
					ImGui::TextWrapped("Spawn weapons by typing their model names.\nExample: wep (wep_name)");
					ImGui::Spacing();
					ImGui::Separator();
				}

				ImGui::EndChild();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("About"))
		{
			ImGui::Spacing();

			if (ImGui::Button("Discord server", ImVec2(Ui::GetSize(2))))
				ShellExecute(NULL, "open", "https://discord.gg/ZzW7kmf", NULL, NULL, SW_SHOWNORMAL);

			ImGui::SameLine();

			if (ImGui::Button("GitHub repo", ImVec2(Ui::GetSize(2))))
				ShellExecute(NULL, "open", "https://github.com/user-grinch/Cheat-Menu", NULL, NULL, SW_SHOWNORMAL);
					
			ImGui::Spacing();

			if (ImGui::BeginChild("AboutChild"))
			{
				ImGui::Columns(2, NULL, false);
				ImGui::Text("Author: Grinch_");

				ImGui::Text((std::string("Version: ") + MENU_VERSION).c_str());

				ImGui::NextColumn();
				ImGui::Text(std::string("Imgui: " + std::string(ImGui::GetVersion())).c_str());
				ImGui::Text((std::string("Build: ") + BUILD_NUMBER).c_str());

				ImGui::Columns(1);
						
				ImGui::Dummy(ImVec2(0, 10));
				ImGui::TextWrapped("Special thanks to,");
				ImGui::Columns(2, NULL, false);
						
				ImGui::TextWrapped("Dowglas_");
				ImGui::TextWrapped("Israel");
				
				ImGui::NextColumn();
				ImGui::TextWrapped("Junior-Djjr");
				ImGui::TextWrapped("Um-geek");

				ImGui::Columns(1);

				ImGui::Dummy(ImVec2(0, 10));
				ImGui::TextWrapped("Copyright GPLv3 2019-2020 Grinch_");
				
				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}