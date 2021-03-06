#include "pch.h"
#include "Ui.h"
#include "CustomWidgets.h"

std::string Ui::current_hotkey = "";

static Ui::JsonPopUpData json_popup;
static Ui::ImgPopUpData img_popup;

bool Ui::ListBox(const char* label, std::vector<std::string>& all_items, int& selected)
{
	bool rtn = false;
	if (ImGui::BeginCombo(label, all_items[selected].c_str()))
	{
		for (size_t index = 0; index < all_items.size(); index++)
		{
			if (selected != index)
			{
				if (ImGui::MenuItem(all_items[index].c_str()))
				{
					selected = index;
					rtn = true;
				}
			}
		}		
		ImGui::EndCombo();
	}
	return rtn;
}

bool Ui::ListBoxStr(const char* label, std::vector<std::string>& all_items, std::string& selected)
{
	bool rtn = false;
	if (ImGui::BeginCombo(label, selected.c_str()))
	{
		for (std::string current_item : all_items)
		{
			if (current_item != label)
			{
				if (ImGui::MenuItem(current_item.c_str()))
				{
					selected = current_item;
					rtn = true;
				}
			}
		}
		ImGui::EndCombo();
	}

	return rtn;
}

ImVec2 Ui::GetSize(short count,bool spacing)
{
	if (count == 1)
		spacing = false;

	float factor = ImGui::GetStyle().ItemSpacing.x / 2.0f;
	float x;
	
	if (count == 3)
		factor = ImGui::GetStyle().ItemSpacing.x / 1.403f;

	if (spacing)
		x = ImGui::GetWindowContentRegionWidth() / count - factor;
	else
		x = ImGui::GetWindowContentRegionWidth() / count;

	return ImVec2(x, ImGui::GetFrameHeight()*1.3f);
}

void Ui::DrawHeaders(unsortedMap& data)
{

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	short i = 1;
	auto colors = ImGui::GetStyle().Colors;
	ImVec4 btn_col = colors[ImGuiCol_Button];
	static void* func;
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		const char* btn_text = it->first.c_str();

		if (btn_text == Globals::header_id)
			colors[ImGuiCol_Button] = colors[ImGuiCol_ButtonActive];


		if (ImGui::Button(btn_text, GetSize(3, false)))
		{
			Globals::header_id = btn_text;
			func = it->second;
		}

		colors[ImGuiCol_Button] = btn_col;

		if (i % 3 != 0)
			ImGui::SameLine();
		i++;
	}
	ImGui::PopStyleVar();
	ImGui::Dummy(ImVec2(0, 10));

	if (Globals::header_id != "" && func != nullptr)
	{
		if (ImGui::BeginChild("TABSBAR"))
		{
			((void(*)(void))func)();
			ImGui::EndChild();
		}
	}
}

void Ui::ShowTooltip(const char* text)
{
	ImGui::SameLine();
	ImGui::TextDisabled("?");

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(text);
		ImGui::EndTooltip();
	}
}

bool Ui::CheckboxWithHint(const char* label, bool* state, const char* hint, const bool is_disabled)
{
	bool rtn = false;

	if (CustomWidgets::Checkbox(label,state,hint,is_disabled))
		rtn = true;

	return rtn; 
}

bool Ui::CheckboxAddress(const char* label, const int addr, const char* hint)
{
	bool rtn = false;
	bool state = patch::Get<bool>(addr,false);

	if (CheckboxWithHint(label, &state, hint) && addr != NULL)
	{
		patch::Set<bool>(addr, state, false);
		rtn = true;
	}

	return rtn;
}

bool Ui::CheckboxAddressEx(const char* label, const int addr, int enabled_val, int disabled_val, const char* hint)
{
	bool rtn = false;

	bool state = false;
	int val = 0;
	patch::GetRaw(addr, &val, 1, false);

	if (val == enabled_val)
		state = true;

	if (CheckboxWithHint(label, &state, hint) && addr != NULL)
	{
		if (state)
			patch::SetRaw(addr, &enabled_val, 1, false);
		else
			patch::SetRaw(addr, &disabled_val, 1, false);
		rtn = true;
	}

	return rtn;
}

bool Ui::CheckboxAddressVar(const char* label, bool val, int addr, const char* hint)
{
	bool rtn = false;
	bool state = val;
	if (CheckboxWithHint(label, &state, hint))
	{
		patch::Set<bool>(addr, state, false);
		rtn = true;
	}

	return rtn;
}

bool Ui::CheckboxAddressVarEx(const char* label, bool val, int addr, int enabled_val, int disabled_val, const char* hint)
{
	bool rtn = false;
	bool state = val;
	if (CheckboxWithHint(label, &state, hint))
	{
		if (state)
			patch::SetRaw(addr, &enabled_val, 1, false);
		else
			patch::SetRaw(addr, &disabled_val, 1, false);

		rtn = true;
	}

	return rtn;
}

bool Ui::CheckboxBitFlag(const char* label,uint flag, const char* hint)
{
	bool rtn = false;
	bool state = (flag == 1) ? true : false;
	if (CheckboxWithHint(label, &state, hint))
	{
		flag = state ? 1 : 0;
		rtn = true;
	}

	return rtn;
}

void Ui::DrawJSON(CJson& json, std::vector<std::string>& combo_items, std::string& selected_item, ImGuiTextFilter& filter , std::function<void(std::string&, std::string&, std::string&)> func_left_click, std::function<void(std::string&, std::string&, std::string&)> func_right_click)
{
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()/2 - 5);
	ListBoxStr("##Categories", combo_items, selected_item);
	ImGui::SameLine();

	filter.Draw("##Filter");
	if (strlen(filter.InputBuf) == 0)
	{
		ImDrawList *drawlist = ImGui::GetWindowDrawList();

		ImVec2 min = ImGui::GetItemRectMin();
		min.x += ImGui::GetStyle().FramePadding.x;
		min.y += ImGui::GetStyle().FramePadding.y;

		drawlist->AddText(min, ImGui::GetColorU32(ImGuiCol_TextDisabled), "Search");
	}
	
	ImGui::PopItemWidth();

	ImGui::Spacing();

	if (ImGui::IsMouseClicked(1))
		json_popup.function = nullptr;


	ImGui::BeginChild(1);
	for (auto root : json.data.items())
	{
		if (root.key() == selected_item || selected_item == "All")
		{
			for (auto _data : root.value().items())
			{
				
				std::string name = _data.key();
				if (filter.PassFilter(name.c_str()))
				{
					if (ImGui::MenuItem(name.c_str()) && func_left_click != nullptr)
					{
						func_left_click(std::string(root.key()),std::string(_data.key()), std::string(_data.value()));
					}

					if (ImGui::IsItemClicked(1) && func_right_click != nullptr)
					{
						json_popup.function = func_right_click;
						json_popup.root_key = root.key();
						json_popup.key = name;
						json_popup.value = _data.value();
					}
				}
			}
		}
	}

	if (json_popup.function != nullptr)
	{
		if (ImGui::BeginPopupContextWindow())
		{
			ImGui::Text(json_popup.key.c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("Remove"))
				json_popup.function(json_popup.root_key,json_popup.key,json_popup.value);


			if (ImGui::MenuItem("Close"))
				json_popup.function = nullptr;

			ImGui::EndPopup();
		}
	}
	ImGui::EndChild();
}


void Ui::EditStat(const char *label, const int stat_id, const int min, const int def, const int max)
{
	if (ImGui::CollapsingHeader(label))
	{
		int val = static_cast<int>(CStats::GetStatValue(stat_id));

		ImGui::Columns(3, 0, false);
		ImGui::Text(("Min: " + std::to_string(min)).c_str());
		ImGui::NextColumn();
		ImGui::Text(("Def: " + std::to_string(def)).c_str());
		ImGui::NextColumn();
		ImGui::Text(("Max: " + std::to_string(max)).c_str());
		ImGui::Columns(1);

		ImGui::Spacing();

		if (ImGui::InputInt(("Set value##" + std::string(label)).c_str(), &val))
			CStats::SetStatValue(stat_id, static_cast<float>(val));

		ImGui::Spacing();

		if (ImGui::Button(("Minimum##" + std::string(label)).c_str(), Ui::GetSize(3)))
			CStats::SetStatValue(stat_id, static_cast<float>(min));

		ImGui::SameLine();

		if (ImGui::Button(("Default##" + std::string(label)).c_str(), Ui::GetSize(3)))
			CStats::SetStatValue(stat_id, static_cast<float>(def));

		ImGui::SameLine();

		if (ImGui::Button(("Maximum##" + std::string(label)).c_str(), Ui::GetSize(3)))
			CStats::SetStatValue(stat_id, static_cast<float>(max));

		ImGui::Spacing();
		ImGui::Separator();
	}
}

void Ui::FilterWithHint(const char* label, ImGuiTextFilter& filter, const char* hint)
{
	filter.Draw(label);

	if (strlen(filter.InputBuf) == 0)
	{
		ImDrawList *drawlist = ImGui::GetWindowDrawList();

		ImVec2 min = ImGui::GetItemRectMin();
		min.x += ImGui::GetStyle().FramePadding.x;
		min.y += ImGui::GetStyle().FramePadding.y;

		drawlist->AddText(min, ImGui::GetColorU32(ImGuiCol_TextDisabled), hint);
	}
}

// clean up the code someday
void Ui::DrawImages(std::vector<std::unique_ptr<TextureStructure>> &img_vec, ImVec2 image_size,
	std::vector<std::string>& category_vec,std::string& selected_item, ImGuiTextFilter& filter,
	std::function<void(std::string&)> on_left_click, std::function<void(std::string&)> on_right_click,
	std::function<std::string(std::string&)> get_name_func, std::function<bool(std::string&)> verify_func)
{

	int images_in_row = static_cast<int>(ImGui::GetWindowContentRegionWidth() / image_size.x);
	image_size.x = ImGui::GetWindowContentRegionWidth() / images_in_row - ImGuiStyleVar_ItemSpacing*0.65f;

	int images_count = 1;

	ImGui::Spacing();

	if (ImGui::IsMouseClicked(1))
		img_popup.function = nullptr;

	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 2 - 5);
	ListBoxStr("##Categories", category_vec, selected_item);
	ImGui::SameLine();
	FilterWithHint("##Filter", filter, "Search");

	ImGui::Spacing();

	ImGui::BeginChild("DrawImages");
	for (uint i = 0; i < img_vec.size(); i++)
	{
		std::string text = img_vec[i]->file_name;
		std::string model_name = get_name_func(text);

		if (filter.PassFilter(model_name.c_str())
			&& (img_vec[i]->category_name == selected_item || selected_item == "All")
			&& (verify_func == nullptr || verify_func(text))
			)
		{
			void *texture = nullptr;

			if (Globals::renderer == Render_DirectX9)
				texture = img_vec[i]->texture9;
			else // consider 11
				texture = img_vec[i]->texture11;

			if (ImGui::ImageButton(texture, image_size, ImVec2(0, 0), ImVec2(1, 1), 1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1)))
				on_left_click(text);

			if (ImGui::IsItemClicked(1) && on_right_click != nullptr)
			{
				img_popup.function = on_right_click;
				img_popup.value = model_name;
			}

			if (ImGui::IsItemHovered())
			{
				ImDrawList *drawlist = ImGui::GetWindowDrawList();

				ImVec2 btn_min = ImGui::GetItemRectMin();
				ImVec2 btn_max = ImGui::GetItemRectMax();

				drawlist->AddRectFilled(btn_min, btn_max, ImGui::GetColorU32(ImGuiCol_ModalWindowDimBg));

				ImVec2 text_size = ImGui::CalcTextSize(model_name.c_str());
				if (text_size.x < image_size.x)
				{
					float offsetX = (ImGui::GetItemRectSize().x - text_size.x) / 2;
					drawlist->AddText(ImVec2(btn_min.x + offsetX, btn_min.y + 10), ImGui::GetColorU32(ImGuiCol_Text), model_name.c_str());
				}
				else
				{
					std::string buff = "";

					std::stringstream ss(model_name); 
					short count = 1;

					while (ss >> buff)
					{
						text_size = ImGui::CalcTextSize(buff.c_str());
						float offsetX = (ImGui::GetItemRectSize().x - text_size.x) / 2;
						drawlist->AddText(ImVec2(btn_min.x + offsetX, btn_min.y + 10 * count), ImGui::GetColorU32(ImGuiCol_Text), buff.c_str());
						++count;
					}
		
				}
			}

			if (images_count % images_in_row != 0)
				ImGui::SameLine(0.0, 4.0);

			images_count++;
		}
	}

	if (img_popup.function != nullptr)
	{
		if (ImGui::BeginPopupContextWindow())
		{
			ImGui::Text(img_popup.value.c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("Remove"))
				img_popup.function(img_popup.value);


			if (ImGui::MenuItem("Close"))
				img_popup.function = nullptr;

			ImGui::EndPopup();
		}
	}
	ImGui::EndChild();
}

void Ui::RadioButtonAddress(const char* label, std::vector<NamedMemory> &named_mem)
{
	size_t btn_in_column = named_mem.size() / 2 -1;

	ImGui::Text(label);
	ImGui::Columns(2, 0, false);

	bool state = true;

	for (size_t i = 0; i < named_mem.size(); i++)
	{
		if (patch::Get<bool>(named_mem[i].addr, false))
			state = false;
	}

	if (ImGui::RadioButton("None", state))
	{
		for (size_t i = 0; i < named_mem.size(); i++)
			patch::Set<bool>(named_mem[i].addr, false);
	}

	for (size_t i = 0; i < named_mem.size(); i++)
	{
		state = patch::Get<bool>(named_mem[i].addr, false);

		if (ImGui::RadioButton(named_mem[i].name.c_str(), state))
		{
			for (size_t i = 0; i < named_mem.size(); i++)
				patch::Set<bool>(named_mem[i].addr, false);

			patch::Set<bool>(named_mem[i].addr, true);
		}

		if (i == btn_in_column)
			ImGui::NextColumn();
	}
	ImGui::Columns(1);
}

void Ui::RadioButtonAddressEx(const char* label, int addr, std::vector<NamedValue> &named_val)
{
	size_t btn_in_column = named_val.size() / 2;

	ImGui::Text(label);
	ImGui::Columns(2, 0, false);

	int mem_val = 0;
	patch::GetRaw(addr, &mem_val, 1, false);

	for (size_t i = 0; i < named_val.size(); i++)
	{
		if (ImGui::RadioButton(named_val[i].name.c_str(), &mem_val, named_val[i].value))
			patch::SetRaw(addr, &named_val[i].value, 1, false);

		if (i == btn_in_column)
			ImGui::NextColumn();
	}
	ImGui::Columns(1);
}

void Ui::EditRadioButtonAddress(const char* label, std::vector<NamedMemory> &named_mem)
{
	if (ImGui::CollapsingHeader(label))
	{
		RadioButtonAddress(label, named_mem);
		ImGui::Spacing();
		ImGui::Separator();
	}
}

void Ui::EditRadioButtonAddressEx(const char* label, int addr, std::vector<NamedValue> &named_val)
{
	if (ImGui::CollapsingHeader(label))
	{
		RadioButtonAddressEx(label,addr,named_val);
		ImGui::Spacing();
		ImGui::Separator();
	}
}

void Ui::ColorPickerAddress(const char* label, int base_addr, ImVec4&& default_color)
{
	if (ImGui::CollapsingHeader(label)) 
	{
		float cur_color[4];
		cur_color[0] = patch::Get<BYTE>(base_addr, false);
		cur_color[1] = patch::Get<BYTE>(base_addr + 1, false);
		cur_color[2] = patch::Get<BYTE>(base_addr + 2, false);
		cur_color[3] = patch::Get<BYTE>(base_addr + 3,false);

		// 0-255 -> 0-1
		cur_color[0] /= 255;
		cur_color[1] /= 255;
		cur_color[2] /= 255;
		cur_color[3] /= 255;

		if (ImGui::ColorPicker4(std::string("Pick color##" + std::string(label)).c_str(), cur_color))
		{
			// 0-1 -> 0-255
			cur_color[0] *= 255;
			cur_color[1] *= 255;
			cur_color[2] *= 255;
			cur_color[3] *= 255;

			patch::Set<BYTE>(base_addr, cur_color[0], false);
			patch::Set<BYTE>(base_addr+1, cur_color[1], false);
			patch::Set<BYTE>(base_addr+2, cur_color[2], false);
			patch::Set<BYTE>(base_addr+3, cur_color[3], false);
		}
		ImGui::Spacing();

		if (ImGui::Button("Reset to default", Ui::GetSize()))
		{
			patch::SetRaw(base_addr, &default_color.w, 1, false);
			patch::SetRaw(base_addr + 1, &default_color.x, 1, false);
			patch::SetRaw(base_addr + 2, &default_color.y, 1, false);
			patch::SetRaw(base_addr + 3, &default_color.z, 1, false);
		}

		ImGui::Spacing();
		ImGui::Separator();
	}
}

void Ui::EditBits(const char *label, const int address, const std::vector<std::string>& names)
{
	int *mem_val = (int*)address;

	if (ImGui::CollapsingHeader(label))
	{
		ImGui::Columns(2, NULL, false);

		for (int i = 0; i < 32; ++i)
		{
			int mask = 1 << i;
			bool state = static_cast<bool>(*mem_val & mask);

			if (ImGui::Checkbox(names[i].c_str(), &state))
				*mem_val ^= mask;

			if (i + 1 == 32 / 2)
				ImGui::NextColumn();
		}
		ImGui::Columns(1);

		ImGui::Spacing();
		ImGui::Separator();
	}
}

void Ui::EditFloat(const char *label, const int address, const float min, const float def, const float max, const float mul)
{
	if (ImGui::CollapsingHeader(label))
	{
		float val = patch::Get<float>(address, false)*mul;
		
		int items = 3;

		if (min == def)
			items = 2;

		ImGui::Columns(items, 0, false);

		ImGui::Text(("Min: " + std::to_string(min)).c_str());

		if (items == 3)
		{
			ImGui::NextColumn();
			ImGui::Text(("Def: " + std::to_string(def)).c_str());
		}
	
		ImGui::NextColumn();
		ImGui::Text(("Max: " + std::to_string(max)).c_str());
		ImGui::Columns(1);

		ImGui::Spacing();

		int size = ImGui::GetFrameHeight();

		if (ImGui::InputFloat(("##" + std::string(label)).c_str(), &val))
			patch::Set<float>(address, val/mul, false);
		
		ImGui::SameLine(0.0, 4.0);
		if (ImGui::Button("-",ImVec2(size, size)))
		{
			val -= 1;
			patch::Set<float>(address, val / mul, false);
		}
		ImGui::SameLine(0.0, 4.0);
		if (ImGui::Button("+",ImVec2(size, size)))
		{
			val += 1;
			patch::Set<float>(address, val / mul, false);
		}
		ImGui::SameLine(0.0, 4.0);
		ImGui::Text("Set");


		ImGui::Spacing();

		if (ImGui::Button(("Minimum##" + std::string(label)).c_str(), Ui::GetSize(items)))
			patch::Set<float>(address, min/mul, false);

		if (items == 3)
		{
			ImGui::SameLine();

			if (ImGui::Button(("Default##" + std::string(label)).c_str(), Ui::GetSize(items)))
				patch::Set<float>(address, def/mul, false);
		}

		ImGui::SameLine();

		if (ImGui::Button(("Maximum##" + std::string(label)).c_str(), Ui::GetSize(items)))
			patch::Set<float>(address, max/mul, false);

		ImGui::Spacing();
		ImGui::Separator();
	}
}

void Ui::HotKey(const char* label, int* key_array)
{
	bool active = current_hotkey == label;

	if (active)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

		for (int key = 2; key != 90; ++key)
		{
			if (KeyPressed(key))
			{
				key_array[0] = key;
				break;
			}
		}

		for (int key = 90; key != 2; --key)
		{
			if (KeyPressed(key))
			{
				key_array[1] = key;
				break;
			}
		}
	}
	
	std::string text = key_names[key_array[0]-1];

	if (key_array[0] != key_array[1])
		text += (" + " + key_names[key_array[1]-1]);

	if (ImGui::Button((text + std::string("##") + std::string(label)).c_str(), ImVec2(ImGui::GetWindowContentRegionWidth() / 3, ImGui::GetFrameHeight())))
	{
		if (active)
			current_hotkey = "";
		else
			current_hotkey = label;;
	}
	ImGui::SameLine();
	ImGui::Text(label);

	if (active)
		ImGui::PopStyleColor();

	if (!(ImGui::IsWindowFocused() || ImGui::IsItemVisible()))
		current_hotkey = "";
}

bool Ui::HotKeyPressed(int *hotkey)
{
	return current_hotkey == "" && KeyPressed(hotkey[0]) && KeyPressed(hotkey[1]);
}

std::string Ui::GetHotKeyNameString(int *hotkey)
{
	std::string text = key_names[hotkey[0] - 1];

	if (hotkey[0] != hotkey[1])
		text += (" + " + key_names[hotkey[1] - 1]);

	return text;
}


bool Ui::ColorButton(int color_id, std::vector<float> &color, ImVec2 size)
{
	bool rtn = false;
	std::string label = "Color " + std::to_string(color_id);

	if (ImGui::ColorButton(label.c_str(), ImVec4(color[0], color[1], color[2], 1), 0, size))
		rtn = true;

	if (ImGui::IsItemHovered())
	{
		ImDrawList *drawlist = ImGui::GetWindowDrawList();
		drawlist->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_ModalWindowDimBg));
	}

	return rtn;
}
