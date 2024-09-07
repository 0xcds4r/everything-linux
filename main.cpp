#include <main.h>
#include <FileIndexer.h>

extern FileIndexer* pFileIdxr;

void signalHandler(int signal) {
	std::cerr << "Signal " << signal << " received." << std::endl;

	void* array[128];
	size_t size;

	size = backtrace(array, 128);
	std::cerr << "Backtrace (most recent call last):" << std::endl;

	for (size_t i = 0; i < size; ++i) {
		Dl_info dl_info;
		if (dladdr(array[i], &dl_info) && dl_info.dli_sname) {
			std::cerr << "  #" << i << " " << dl_info.dli_fname << " (" 
					  << dl_info.dli_fbase << ") : " 
					  << dl_info.dli_sname << " + " 
					  << (static_cast<char*>(array[i]) - static_cast<char*>(dl_info.dli_saddr))
					  << std::endl;
		} else {
			std::cerr << "  #" << i << " " << array[i] << std::endl;
		}
	}

	Dl_info exe_info;
	if (dladdr(reinterpret_cast<void*>(&signalHandler), &exe_info) && exe_info.dli_sname) {
		std::cerr << "Crash address: " << reinterpret_cast<void*>(reinterpret_cast<char*>(array[0]) - reinterpret_cast<char*>(exe_info.dli_fbase)) 
				  << " (offset from " << exe_info.dli_fname << " base address " << exe_info.dli_fbase << ")" << std::endl;
	}

	std::cerr << "Aborting due to signal " << signal << std::endl;
	exit(EXIT_FAILURE);
}

GLFWwindow* window = nullptr;

constexpr size_t MAX_QUERY_SIZE = 2048;  
char searchQuery[MAX_QUERY_SIZE+1] = "";
char prevSearchQuery[MAX_QUERY_SIZE+1] = "";

static int itemsPerPage = 30;

void SetupImGuiStyle() {
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

void openFileInExplorer(const std::string& path) 
{
	std::string command = "xdg-open \"" + path + "\"";
	int result = std::system(command.c_str());

	if (result != 0) {
		std::cerr << "Failed to open file: " << path << std::endl;

		fs::path filePath(path);
		if (filePath.has_parent_path()) {
			std::string directory = filePath.parent_path().string();
			std::string openDirCommand = "xdg-open \"" + directory + "\"";

			result = std::system(openDirCommand.c_str());
			if (result != 0) {
				std::cerr << "Failed to open directory: " << directory << std::endl;
			}
		} else {
			std::cerr << "No valid parent directory found for: " << path << std::endl;
		}
	}
}

void openFolderInExplorer(const std::string& filePath) 
{
	std::filesystem::path path(filePath);
	std::string folderPath = path.parent_path().string();

	std::string command = "xdg-open \"" + folderPath + "\"";
	system(command.c_str());
}

void copyFilePathToInput(const std::string& filePath) 
{
	if (filePath.empty() || filePath.size() >= MAX_QUERY_SIZE) {
		std::cerr << "Invalid file path length." << std::endl;
		return;
	}

	strncpy(searchQuery, filePath.c_str(), sizeof(searchQuery) - 1);
	searchQuery[sizeof(searchQuery) - 1] = '\0'; // Ensure null termination
	sprintf(prevSearchQuery, "%s", searchQuery); // Also copy to prevSearchQuery
}

void copyPathToInput(const std::string& filePath)
{
	std::filesystem::path path(filePath);
	std::string folderPath = path.parent_path().string();

	strncpy(searchQuery, folderPath.c_str(), sizeof(searchQuery) - 1);
	searchQuery[sizeof(searchQuery) - 1] = '\0'; // Ensure null termination
	sprintf(prevSearchQuery, "%s", searchQuery); // Also copy to prevSearchQuery
}

void updateItemsPerPageFromQuery() 
{
	if (searchQuery[0] != '>' && searchQuery[1] != '>') {
		// std::cerr << "Query does not start with '>>', ignoring." << std::endl;
		return;
	}

	std::string query(searchQuery);

	std::regex pattern(R"(>> max_results_count: (\d+))"); 
	std::smatch match;

	if (std::regex_search(query, match, pattern)) {
		if (!match.str(1).empty()) {
			itemsPerPage = std::stoi(match.str(1));
		}

		query = std::regex_replace(query, pattern, "");

		if (query.length() >= sizeof(searchQuery) - 1) {
			std::cerr << "Search query too long, truncating." << std::endl;
			query = query.substr(0, sizeof(searchQuery) - 1);
		}

		strncpy(searchQuery, query.c_str(), sizeof(searchQuery) - 1);
		searchQuery[sizeof(searchQuery) - 1] = '\0';
	}
}

bool useRegex = false;
bool usePath = true;
bool useAutoSearch = false;
bool useScrollPage = false;

void renderGui()
{
	if (!pFileIdxr) {
		LOGE("pFileIdxr is NULL");
		return;
	}

	static bool bCopy = false;
	static int selected = -1;
	static int currentPage = 0;
	static auto lastAutoSearchTime = std::chrono::steady_clock::now();
	constexpr std::chrono::milliseconds autoSearchInterval(1000);

	static auto lastCopyTime = std::chrono::steady_clock::now();
	constexpr std::chrono::milliseconds CopyInterval(2000);

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImGui::SetNextWindowSize(displaySize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
									ImGuiWindowFlags_NoResize |
									ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_NoScrollbar;

	ImGui::Begin("File and Directory Search", nullptr, window_flags);

	ImGui::InputText("<<<", searchQuery, MAX_QUERY_SIZE, ImGuiInputTextFlags_EnterReturnsTrue);
	updateItemsPerPageFromQuery();

	ImGui::Checkbox("Use Regex", &useRegex);
	ImGui::SameLine();
	ImGui::Checkbox("Use Path", &usePath);
	ImGui::SameLine();
	ImGui::Checkbox("Use Scroll Page", &useScrollPage);
	ImGui::SameLine();
	ImGui::Checkbox("Use Auto Search", &useAutoSearch);

	if (useRegex) {
		usePath = false;
	}

	static std::vector<std::string> searchResults = pFileIdxr->getHomeDir();
	static std::vector<std::string> results = searchResults;

	auto performSearch = [&]() {
		currentPage = 0;

		if (usePath) {
			results = pFileIdxr->searchPath(searchQuery);
		} else if (useRegex) {
			results = pFileIdxr->searchRegex(searchQuery);
		} else {
			results = pFileIdxr->searchFile(searchQuery);
		}

		if (strlen(searchQuery) > 0) {
			std::strcpy(prevSearchQuery, searchQuery);
		} else {
			results = searchResults;
		}
	};

	ImGui::SameLine();
	if (ImGui::Button("Search")) {
		performSearch();
	}

	if (useAutoSearch && strlen(searchQuery) > 0 && strcmp(searchQuery, prevSearchQuery) != 0) 
	{
		auto now = std::chrono::steady_clock::now();
		if (now - lastAutoSearchTime >= autoSearchInterval) {
			performSearch();
			lastAutoSearchTime = now;
		}
	}

	ImGui::Text("Results:");

	int totalPages = (results.size() + itemsPerPage - 1) / itemsPerPage;
	ImGui::BeginChild("Results", ImVec2(0, 300), true);

	int startIndex = currentPage * itemsPerPage;
	int endIndex = std::min(startIndex + itemsPerPage, static_cast<int>(results.size()));

	for (int i = startIndex; i < endIndex; ++i) {
		if (ImGui::Selectable(results[i].c_str(), selected == i, ImGuiSelectableFlags_AllowDoubleClick)) {
			if (ImGui::IsMouseDoubleClicked(0)) {
				openFileInExplorer(results[i]);
			}
			selected = i;
		}
	}

	if(useScrollPage) {
		float scrollY = ImGui::GetScrollY();
		float maxScrollY = ImGui::GetScrollMaxY();
		static bool prevScroll = true;

		if (!prevScroll && (scrollY >= maxScrollY - 1.0f || scrollY == 0.0f)) {
			if (scrollY >= maxScrollY - 1.0f && currentPage < totalPages - 1) {
				currentPage++;
				ImGui::SetScrollY(0.1f);
			} else if (scrollY == 0.0f && currentPage > 0) {
				currentPage--;
				ImGui::SetScrollY(0.5f);
			}
			prevScroll = true;
		} 
		else if (scrollY > 0.1f) {
			prevScroll = false;
		}
	}

	ImGui::EndChild();

	if (ImGui::Button("Previous Page") && currentPage > 0) {
		currentPage--;
		ImGui::SetScrollY(ImGui::GetScrollMaxY());
	}
	ImGui::SameLine();
	if (ImGui::Button("Next Page") && currentPage < totalPages - 1) {
		currentPage++;
		ImGui::SetScrollY(0.0f);
	}

	ImGui::SameLine();
	if (selected != -1) {
		std::string selectedFile = results[selected];

		if (ImGui::Button("Open Folder")) {
			openFolderInExplorer(selectedFile);
		}

		ImGui::SameLine();

		if (ImGui::Button("Show path")) {
			if (!usePath) {
				copyFilePathToInput(selectedFile);
			} else {
				copyPathToInput(selectedFile);
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Copy path")) {
			bCopy = true;
			lastCopyTime = std::chrono::steady_clock::now();
		}

		if(bCopy) 
		{
			auto now = std::chrono::steady_clock::now();
			if (now - lastCopyTime >= CopyInterval) {
				bCopy = false;
				lastCopyTime = now;
			}

			std::filesystem::path path(selectedFile);
			std::string folderPath = path.parent_path().string();

			if (!usePath) {
				ImGui::SameLine();
				ImGui::Text("<< '%s' copied to clipboard >>", selectedFile.c_str());
				glfwSetClipboardString(window, selectedFile.c_str());
			} else {
				ImGui::SameLine();
				ImGui::Text("<< '%s' copied to clipboard >>", folderPath.c_str());
				glfwSetClipboardString(window, folderPath.c_str());
			}
		}
	}

	if(!bCopy) ImGui::SameLine();
	ImGui::Text("Page %d of %d (%d)", currentPage + 1, totalPages, itemsPerPage);

	ImGui::End();
}


const char* loadingText = "";
void renderLoadingScreen() 
{
	if(!loadingText) {
		return;
	}

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

	ImVec2 text_pos_state(displaySize.x * 0.5f - ImGui::CalcTextSize(loadingText).x * 0.5, displaySize.y * 0.1f);
	draw_list->AddText(text_pos_state, IM_COL32(255, 255, 255, 255), loadingText);

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
}

void setup_signal_handler() {
	std::signal(SIGSEGV, signalHandler); // Handle segmentation faults
	std::signal(SIGABRT, signalHandler); // Handle aborts
	std::signal(SIGFPE, signalHandler);  // Handle floating point exceptions
	std::signal(SIGILL, signalHandler);  // Handle illegal instructions
}

int main() 
{
	setup_signal_handler();

	if (!glfwInit()) { 
		return -1;
	}

	std::string appName(std::string("Everything for Linux v") + std::string(APP_VERSION));
	window = glfwCreateWindow(MAX_WIDTH, MAX_HEIGHT, appName.c_str(), NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0); 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard; 

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	ImGui::StyleColorsDark();
	SetupImGuiStyle();

	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if(FileIndexer::getState() != STATE_CACHING_END) {
			loadingText = "Initializing..";
			renderLoadingScreen();
		} else {
			renderGui();
		}

		switch(FileIndexer::getState()) {
			case STATE_NONE: {
				FileIndexer::Initialise();
				break;
			}

			case STATE_INIT: {
				pFileIdxr->indexDirectories(getAcceptedDirs());
				break;
			}
		}

		ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
