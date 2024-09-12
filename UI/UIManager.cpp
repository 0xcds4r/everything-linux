#include <main.h>
#include <UI/UIManager.h>

std::list<std::unique_ptr<Interface>> UIManager::interfaces;
std::list<std::string> UIManager::interfaceNames;

void UIManager::Initialise() 
{
    LOGI("Initializing UIManager..");
    
    UIManager::SetupImGuiStyle();

    UIManager::RegisterInterfaces();

    UIManager::LoadAllInterfaces();
}

void UIManager::RegisterInterfaces() 
{
	LOGI("Registering interfaces..");

	UIManager::RegisterInterface<SplashInterface>("splash");
    UIManager::RegisterInterface<MainSearchInterface>("main");
}

void UIManager::Draw() {
	UIManager::RenderAllInterfaces();
}

void UIManager::SetupImGuiStyle() {
	LOGI("Updating imgui styles..");
	ImGuiStyle& style = ImGui::GetStyle();

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text]                 = ImVec4(0.93f, 0.96f, 1.00f, 1.00f); 
	colors[ImGuiCol_WindowBg]             = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); 
	colors[ImGuiCol_ChildBg]              = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_Border]               = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]              = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_FrameBgActive]        = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_TitleBg]              = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgActive]        = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_MenuBarBg]            = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_CheckMark]            = ImVec4(0.33f, 0.55f, 0.65f, 1.00f); 
	colors[ImGuiCol_SliderGrab]           = ImVec4(0.33f, 0.55f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.23f, 0.45f, 0.55f, 1.00f);
	colors[ImGuiCol_Button]               = ImVec4(0.33f, 0.55f, 0.65f, 1.00f);
	colors[ImGuiCol_ButtonHovered]        = ImVec4(0.43f, 0.65f, 0.75f, 1.00f);
	colors[ImGuiCol_ButtonActive]         = ImVec4(0.23f, 0.45f, 0.55f, 1.00f);
	colors[ImGuiCol_Header]               = ImVec4(0.33f, 0.55f, 0.65f, 1.00f);
	colors[ImGuiCol_HeaderHovered]        = ImVec4(0.43f, 0.65f, 0.75f, 1.00f);
	colors[ImGuiCol_HeaderActive]         = ImVec4(0.23f, 0.45f, 0.55f, 1.00f);
	colors[ImGuiCol_Separator]            = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_SeparatorActive]      = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	
	style.FrameRounding = 4.0f;
	style.PopupRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.ScrollbarRounding = 4.0f;
	style.WindowRounding = 4.0f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.33f, 0.55f, 0.65f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.43f, 0.65f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.23f, 0.45f, 0.55f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.55f, 0.65f, 1.00f);
}