#include <main.h>
#include <UI/UIManager.h>

int renderSplashFrames = 0;

#define LOADING_UPDATE_START_FRAMES_COUNT 50
#define LOADING_OVER_FRAMES_COUNT 300

void SplashInterface::Display() {
	m_bSplashAllowed = true;	
}

void SplashInterface::Render() 
{
	if(!m_bSplashAllowed) {
		return;
	}

	if(renderSplashFrames > LOADING_UPDATE_START_FRAMES_COUNT && UIManager::GetInterface<MainSearchInterface>("main")->IsNeverIndexing()) {
		UIManager::GetInterface<MainSearchInterface>("main")->UpdateDirectories();
	}

	if(renderSplashFrames > LOADING_OVER_FRAMES_COUNT && UIManager::GetInterface<MainSearchInterface>("main")->IsOverIndexing()) 
	{
		Toggle(false);
		UIManager::GetInterface<MainSearchInterface>("main")->Toggle(true);
	}

	renderSplashFrames++;

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

	ImVec2 text_pos_state(displaySize.x * 0.5f - ImGui::CalcTextSize("Initializing app..").x * 0.5, displaySize.y * 0.1f);
	draw_list->AddText(text_pos_state, IM_COL32(255, 255, 255, 255), "Initializing app..");

	ImVec2 separator_pos(displaySize.x * 0.1f, displaySize.y * 0.2f);

	int size = strlen("- The author of this project greatly missed a proper search tool for Linux, so they decided to recreate something similar to Everything.");
	
	std::string bufferTp;
	for(int i = 0; i < size; i++)
		bufferTp += std::string("-");

	draw_list->AddText(separator_pos, IM_COL32(255, 255, 255, 255), bufferTp.c_str());

	ImVec2 text_pos_info(displaySize.x * 0.1f, displaySize.y * 0.25f);
	draw_list->AddText(text_pos_info, IM_COL32(0, 255, 0, 255), "Everything for Linux it's free and open-source software.");

	ImVec2 text_pos_info2(displaySize.x * 0.1f, displaySize.y * 0.30f);
	draw_list->AddText(text_pos_info2, IM_COL32(0, 255, 0, 255), "- The original Everything project for Windows has nothing to do with this project, apart from the idea.");

	ImVec2 text_pos_info3(displaySize.x * 0.1f, displaySize.y * 0.35f);
	draw_list->AddText(text_pos_info3, IM_COL32(0, 255, 0, 255), "- The author of this project greatly missed a proper search tool for Linux, so they decided to recreate something similar to Everything.");

	ImVec2 text_pos_info4(displaySize.x * 0.1f, displaySize.y * 0.40f);
	draw_list->AddText(text_pos_info4, IM_COL32(0, 255, 0, 255), "- The author plans to continue developing this project if they receive support from regular users.");

	ImVec2 text_pos_author(displaySize.x * 0.1f, displaySize.y * 0.45f);
	draw_list->AddText(text_pos_author, IM_COL32(0, 255, 0, 255), "The author of 'Everything for Linux' is 0xcds4r.");

	ImVec2 separator_pos_bottom(displaySize.x * 0.1f, displaySize.y * 0.50f);
	draw_list->AddText(separator_pos_bottom, IM_COL32(255, 255, 255, 255), bufferTp.c_str());

    float progress = (float)renderSplashFrames / LOADING_OVER_FRAMES_COUNT; 

    ImVec2 progressBarPos(displaySize.x * 0.1f, displaySize.y * 0.6f);
    ImVec2 progressBarSize(ImGui::CalcTextSize(bufferTp.c_str()).x, ImGui::GetFontSize() * 1.95f); 

    ImU32 borderColor = IM_COL32(55, 55, 55, 255); 
    ImU32 fillColor = IM_COL32(44, 150, 44, 255);
 
    draw_list->AddRect(progressBarPos, ImVec2(progressBarPos.x + progressBarSize.x, progressBarPos.y + progressBarSize.y), borderColor, 5.0f, 0, 0.5f);
    draw_list->AddRectFilled(progressBarPos, ImVec2(progressBarPos.x + progressBarSize.x * progress, progressBarPos.y + progressBarSize.y), fillColor, 5.0f);

    int percentage = static_cast<int>(progress * 100.0f);
    std::string percentageText = "0%";
   	percentageText = std::to_string(percentage) + "%";
    ImVec2 textSize = ImGui::CalcTextSize(percentageText.c_str());
    ImVec2 textPos = ImVec2(progressBarPos.x + (progressBarSize.x - textSize.x) * 0.5f, progressBarPos.y + (progressBarSize.y - textSize.y) * 0.5f);
    draw_list->AddText(textPos, IM_COL32(255, 255, 255, 255), percentageText.c_str()); // White text
}