#include <main.h>
#include <UI/UIManager.h>
#include <SearchEngine.h>

constexpr size_t MAX_QUERY_SIZE = 2048;  
char searchQuery[MAX_QUERY_SIZE + 1] = "";
char prevSearchQuery[MAX_QUERY_SIZE + 1] = "";

// static int itemsPerPage = 30;
static int currentPage = 0;
static int itemsPerPage = 60;
static bool useAutoSearch = false;
static bool usePath = false;
static bool useScrollPage = false;
static bool useRegex = false;
static bool bCopy = false;
static int selected = -1;

std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
bool bInputUpdated = false;

static std::list<std::filesystem::path> searchResults;

SearchEngine searcher;

const char* directories[] = { "/media", "/home", "/etc", "/var", "/usr", "/bin", "/opt" };
static int directoryIndex = 1;

void MainSearchInterface::Display() {
	m_bMSAllowed = false;
    m_cIndexingState = 3;
}

void MainSearchInterface::UpdateDirectories() {
    // LOGI("Indexing started");
    m_cIndexingState = 1;
    searcher.IndexDirectory(directories[directoryIndex]);
    m_cIndexingState = 0;
    searchResults = searcher.SearchByKeywords("");
    // LOGI("Indexing ended");
}

void openInExplorer(const std::string& path, bool openFolder = false) 
{
    std::string command = "xdg-open \"" + (openFolder ? std::filesystem::path(path).parent_path().string() : path) + "\"";
    if (std::system(command.c_str()) != 0) {
        std::cerr << "Failed to open " << (openFolder ? "folder: " : "file: ") << path << std::endl;
        openInExplorer(path, !openFolder);
    }
}

void copyPathToSearch(const std::string& filePath, bool copyFolder = false)
{
    std::string path = copyFolder ? std::filesystem::path(filePath).parent_path().string() : filePath;
    if (path.size() >= MAX_QUERY_SIZE) {
        std::cerr << "Invalid file path length." << std::endl;
        return;
    }
    strncpy(searchQuery, path.c_str(), MAX_QUERY_SIZE);
    strncpy(prevSearchQuery, searchQuery, MAX_QUERY_SIZE);
}
#include <regex>

bool doCheckAppCommands() 
{
    std::string szQuery(searchQuery);
    if (strncmp(searchQuery, ">>", 2) == 0) {
        std::regex pattern(R"(>> max_results_count: (\d+))"); 
        std::smatch match;

        if (std::regex_search(szQuery, match, pattern) && !match.str(1).empty()) {
            itemsPerPage = std::stoi(match.str(1));
            std::string query = std::regex_replace(searchQuery, pattern, "");
            strncpy(searchQuery, query.c_str(), MAX_QUERY_SIZE);
        }

        return true;
    }

    return false;
}

int InputText(ImGuiInputTextCallbackData* data) {
	// LOGI("OnInputCallback: " + std::string(data->Buf));
    bInputUpdated = true;
    lastUpdateTime = std::chrono::steady_clock::now();
	return 1;
}

void MainSearchInterface::Render()
{
    if (!m_bMSAllowed) return;

    RenderWindow();
    RenderSearchInput();
    RenderSearchOptions();
    RenderSearchResults();
    HandleSelected();
    RenderPagination();

    ImGui::End();
}

void MainSearchInterface::RenderWindow()
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGui::Begin("SearchMain", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
}

void MainSearchInterface::RenderSearchInput()
{
    ImGui::SetNextItemWidth(90.0f);
    if(ImGui::Combo("  > > ", &directoryIndex, directories, IM_ARRAYSIZE(directories))) {
        UpdateDirectories();
    }

    ImGui::SameLine();
    ImGui::InputTextEx("\t\t", "Enter a path or filename", searchQuery, MAX_QUERY_SIZE, ImVec2(-1, ImGui::GetFontSize() * 1.5f), ImGuiInputTextFlags_CallbackEdit, InputText, NULL);

    if (ImGui::IsItemDeactivatedAfterEdit() || useAutoSearch)
    {
        if(!doCheckAppCommands()) {
            HandleSearchQuery();
        }
    }
}

void MainSearchInterface::HandleSearchQuery()
{
    if (bInputUpdated)
    {
        if(useAutoSearch) {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime);
            if (duration.count() < 1000) {
                return;
            }
        }

        currentPage = 0;
        searchResults = useRegex ? searcher.SearchByRegex(searchQuery) : searcher.SearchByKeywords(searchQuery);
        LOGI("Searching -> " + std::string(searchQuery));
        bInputUpdated = false;
    }
}

void MainSearchInterface::RenderSearchOptions()
{
    ImGui::Checkbox("Use Regex", &useRegex);
    ImGui::SameLine();
    ImGui::Checkbox("Auto Search", &useAutoSearch);
    ImGui::SameLine();
    ImGui::Checkbox("Scroll-page", &useScrollPage);
}

void MainSearchInterface::RenderSearchResults()
{
    int totalPages = (searchResults.size() + itemsPerPage - 1) / itemsPerPage;

    ImGui::BeginChild("Results", ImVec2(-1, 300), true);

    int startItem = currentPage * itemsPerPage;
    int endItem = std::min(startItem + itemsPerPage, static_cast<int>(searchResults.size()));

    auto it = searchResults.begin();
    std::advance(it, startItem);

    for (int i = startItem; i < endItem && it != searchResults.end(); ++i, ++it)
    {
        const std::string filePath = it->string();
        if (ImGui::Selectable(filePath.c_str(), selected == i))
        {
            if(i != selected)
                selected = i;
            else {
                openInExplorer(filePath.c_str());
            }
        }
    }

    HandleScrollPaging(totalPages);

    ImGui::EndChild();
}

void MainSearchInterface::HandleSelected() {
    RenderFileActions();
}

void MainSearchInterface::HandleScrollPaging(int totalPages)
{
    if (useScrollPage)
    {
        float scrollY = ImGui::GetScrollY();
        float maxScrollY = ImGui::GetScrollMaxY();

        if (scrollY >= maxScrollY - 0.5f && currentPage < totalPages - 1)
        {
            currentPage++;
            ImGui::SetScrollY(0.5f);
        }
        else if (scrollY == 0.0f && currentPage > 0)
        {
            currentPage--;
            ImGui::SetScrollY(0.5f);
        }
    }
}

void MainSearchInterface::RenderPagination()
{
    int totalPages = (searchResults.size() + itemsPerPage - 1) / itemsPerPage;

    if (ImGui::Button("Previous Page") && currentPage > 0) currentPage--;
    ImGui::SameLine();
    if (ImGui::Button("Next Page") && currentPage < totalPages - 1) currentPage++;

    ImGui::Text("Page %d of %d (%d items per page)", currentPage + 1, totalPages, itemsPerPage);
}

void DoCopyToClipBoard(const char* path);
void MainSearchInterface::RenderFileActions()
{
    // LOGI("RenderFileActions");
    if (selected != -1)
    {
         // LOGI("RenderFileActions 1");
        auto it = searchResults.begin();
        std::advance(it, selected);
        const std::string selectedFile = it->string();
         // LOGI("RenderFileActions 2");
        // ImGui::SameLine();
        if (ImGui::Button("Open Folder")) openInExplorer(selectedFile, true);
        ImGui::SameLine();
        if (ImGui::Button("Open File")) openInExplorer(selectedFile, false);
        ImGui::SameLine();
        if (ImGui::Button("Show Path")) copyPathToSearch(selectedFile, false);
        ImGui::SameLine();
        if (ImGui::Button("Copy Path")) DoCopyToClipBoard(selectedFile.c_str());
    }
}