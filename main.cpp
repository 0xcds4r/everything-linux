#include <main.h>

void signalHandler(int signal) {
	std::cerr << "Signal " << signal << " received." << std::endl;

	void* array[64];
	size_t size;

	size = backtrace(array, 64);
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

constexpr size_t MAX_QUERY_SIZE = 1024;  
char searchQuery[MAX_QUERY_SIZE] = "";
char prevSearchQuery[MAX_QUERY_SIZE] = "";

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

namespace fs = std::filesystem;

class FileIndexer {
public:
	void indexDirectory(const fs::path& path) {
		try {
			for (const auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
				if (entry.is_symlink()) {
					continue;
				}

				if (entry.is_regular_file() || entry.is_directory()) { 
					std::lock_guard<std::mutex> lock(mutex);
					files[entry.path().filename().string()].push_back(entry.path().string());
				}
			}
		} catch (const fs::filesystem_error& e) {
			std::cerr << "Filesystem error: " << e.what() << std::endl;
		}
	}

	void indexDirectories(const std::vector<fs::path>& paths) {
		std::vector<std::thread> threads;
		for (const auto& path : paths) {
			threads.emplace_back(&FileIndexer::indexDirectory, this, path);
		}

		for (auto& thread : threads) {
			thread.join();
		}
	}

	std::vector<std::string> search(const std::string& query) const {
		std::vector<std::string> results{};

        for (const auto& [filename, paths] : files) {
        	if (filename.find(query) != std::string::npos) {
         	   results.insert(results.end(), paths.begin(), paths.end());
        	}

       		for (const auto& path : paths) 
       		{
       			if (query.length() >= path.length()) {
                    continue;
                }

            	if (path.find(query) != std::string::npos) {
            	    results.push_back(path);
            	}
        	}
    	}
    	
        return results;
	}

	std::vector<std::string> searchWithRegex(const std::string& pattern) const {
		std::vector<std::string> results;
        std::regex re(pattern);
        for (const auto& [filename, paths] : files) {
            if (std::regex_search(filename, re)) {
                results.insert(results.end(), paths.begin(), paths.end());
            }
        }
        return results;
	}

	std::vector<std::string> searchByPath(const std::string& pathQuery) const 
	{
		std::vector<std::string> results{};

		if(pathQuery.empty()) {
			return results;
		}
		
		for (const auto& [_, paths] : files) {
			for (const auto& path : paths) {
				if(path.find(pathQuery) == 0) {
					results.push_back(path);
				}
			}
		}
		return results;
	}


	std::vector<std::string> getAllFilesAndDirectoriesAtHome() const {
		std::vector<std::string> results;
		for (const auto& [_, paths] : files) {
			for (const auto& path : paths) {
				if (path.find("/home/") == 0) {
					results.push_back(path);
				}
			}
		}
		return results;
	}

private:
	bool isValidPath(const std::string& path) const {
		return fs::exists(path) && !fs::is_directory(path);
	}

	std::unordered_map<std::string, std::vector<std::string>> files{};
	mutable std::mutex mutex;
};

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

void copyFilePathToClipboard(const std::string& filePath) 
{
	if (filePath.empty() || filePath.size() >= MAX_QUERY_SIZE) {
		std::cerr << "Invalid file path length." << std::endl;
		return;
	}

	strncpy(searchQuery, filePath.c_str(), sizeof(searchQuery) - 1);
	searchQuery[sizeof(searchQuery) - 1] = '\0'; // Ensure null termination
	sprintf(prevSearchQuery, "%s", searchQuery); // Also copy to prevSearchQuery
}

void copyPathToClipboard(const std::string& filePath)
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

void renderGui(FileIndexer& indexer) 
{
	static std::vector<std::string> searchResults = indexer.getAllFilesAndDirectoriesAtHome(); 
	static std::vector<std::string> regexResults;
	static std::vector<std::string> pathResults;
	static int selected = -1;

	static int currentPage = 0;

	static auto lastAutoSearchTime = std::chrono::steady_clock::now();
	constexpr std::chrono::milliseconds autoSearchInterval(1000);

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImGui::SetNextWindowSize(displaySize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("File and Directory Search", nullptr, window_flags);

	ImGui::InputText("<<<", searchQuery, MAX_QUERY_SIZE, ImGuiInputTextFlags_EnterReturnsTrue);
	updateItemsPerPageFromQuery();

	ImGui::Checkbox("Use Regex", &useRegex);
	ImGui::SameLine();
	ImGui::Checkbox("Use Path", &usePath);
	ImGui::SameLine();
	ImGui::Checkbox("Use Auto Search", &useAutoSearch);
	ImGui::SameLine();

	if(useRegex) {
		usePath = false;
	}

	static bool searchTriggered = false;
	static std::vector<std::string> results = searchResults;

	if(strlen(searchQuery) == 0) {
		results = searchResults;
	}

	auto performSearch = [&]() {
		std::cout << strlen(searchQuery) << std::endl;
		currentPage = 0;

		// std::string szSearchQuery(searchQuery);

		if (usePath) {
			std::cout << "search by path" << std::endl;
			results = indexer.searchByPath(searchQuery);
		} else if (useRegex) {
			std::cout << "search by regex" << std::endl;
			results = indexer.searchWithRegex(searchQuery);
		} else {
			std::cout << "search by file" << std::endl;
			results = indexer.search(searchQuery);
		}
		searchTriggered = strlen(searchQuery) > 0;
		if(searchTriggered) {
			sprintf(prevSearchQuery, "%s", searchQuery); // Also copy to prevSearchQuery
		}
	};

	if (ImGui::Button("Search")) {
		performSearch();
	}

	if(results.empty()) {
		results = searchResults;
	}

	if (useAutoSearch && strcmp(searchQuery, prevSearchQuery) != 0) 
	{
		auto now = std::chrono::steady_clock::now();
		if (now - lastAutoSearchTime >= autoSearchInterval) {
			performSearch();
			lastAutoSearchTime = now;
		}
	}

	ImGui::Text("Results:");
	// const std::vector<std::string>& results = searchTriggered ? (usePath ? pathResults : (useRegex ? regexResults : searchResults)) : searchResults;

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
	ImGui::EndChild();

	if (ImGui::Button("Previous Page") && currentPage > 0) {
		currentPage--;
	}
	ImGui::SameLine();
	if (ImGui::Button("Next Page") && currentPage < totalPages - 1) {
		currentPage++;
	}

	ImGui::SameLine();
	if (selected != -1) {
		std::string selectedFile = results[selected];
		
		if (ImGui::Button("Open Folder")) {
			openFolderInExplorer(selectedFile); 
		}

		ImGui::SameLine();

		if (ImGui::Button("Show path")) {
			if(!usePath)
				copyFilePathToClipboard(selectedFile);
			else
				copyPathToClipboard(selectedFile);
		}
	}

	ImGui::SameLine();

	ImGui::Text("Page %d of %d (%d)", currentPage + 1, totalPages, itemsPerPage);

	ImGui::End();
}

void renderLoadingScreen() {
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImGui::SetNextWindowSize(displaySize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;

	ImGui::Begin("Loading", nullptr, window_flags);

	ImGui::Text("Indexing files and directories...");
	ImGui::Text("\n");
	ImGui::Text("------------------------------------------------------------------------------------------------------------------------------");
	ImGui::Text("\n");

	ImGui::Text("Everything for Linux it's free and open-source software.");
	ImGui::Text("\n");

	ImGui::Text("- The original Everything project for Windows has nothing to do with this project, apart from the idea.");
	// ImGui::Text("\n");

	ImGui::Text("- The author of this project greatly missed a proper search tool for Linux, so they decided to recreate something similar to Everything.");
	// ImGui::Text("\n");

	ImGui::Text("- The author plans to continue developing this project if they receive support from regular users.");
	ImGui::Text("\n");

	ImGui::Text("The author of 'Everything for Linux' is 0xcds4r.");

	ImGui::Text("\n");
	ImGui::Text("------------------------------------------------------------------------------------------------------------------------------");
	ImGui::Text("\n");

	ImGui::End();
}

int main() 
{
	std::signal(SIGSEGV, signalHandler); // Handle segmentation faults
	std::signal(SIGABRT, signalHandler); // Handle aborts
	std::signal(SIGFPE, signalHandler);  // Handle floating point exceptions
	std::signal(SIGILL, signalHandler);  // Handle illegal instructions

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

	std::promise<void> indexPromise;
	std::future<void> indexFuture = indexPromise.get_future();

	FileIndexer indexer;
	std::thread indexingThread([&]() {
		std::vector<fs::path> directories = {
			"/home/",           // User home directories
			"/media/",          // Mount points
			"/mnt/",            // Mount points
			"/opt/",            // Optional application software packages
			"/usr/",            // User binaries, libraries, etc.
			"/var/",            // Variable data files
			"/etc/",            // Configuration files
			"/tmp/",            // Temporary files
			"/root/"            // Root user directory
		};
		indexer.indexDirectories(directories);
		indexPromise.set_value();  
	});

	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (indexFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			renderGui(indexer);
		} else {
			renderLoadingScreen();
		}

		ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	indexingThread.join();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
