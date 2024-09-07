#include <main.h>
#include <FileIndexer.h>

eState FileIndexer::state = STATE_NONE;
FileIndexer* pFileIdxr = nullptr;

std::vector<fs::path> accepted_dirs = {
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

std::vector<fs::path> getAcceptedDirs() {
	return accepted_dirs;
}

bool contains(const std::string& str, const std::string& substring) 
{
	return str.find(substring) != std::string::npos;
}

bool startsWith(const std::string& str, const std::string& prefix) {
	if (prefix.size() > str.size()) {
		return false;
	}
	return std::equal(prefix.begin(), prefix.end(), str.begin());
}

bool endsWith(const std::string& str, const std::string& pattern) {
	try {
		std::regex regex_pattern(pattern);
		return std::regex_search(str, regex_pattern);
	} catch (const std::regex_error&) {
		return false;
	}
}

void FileIndexer::Initialise() 
{
	LOGI("FileIndexer::Initialise");

	pFileIdxr = new FileIndexer();
	FileIndexer::state = STATE_INIT;
	
	pFileIdxr->files.clear();
}

eState FileIndexer::getState() {
	return FileIndexer::state;
}

void FileIndexer::indexDirectory(const fs::path& path) 
{
	LOGI("FileIndexer::indexDirectory " + path.string());

	FileIndexer::state = STATE_CACHING;

	try {
		std::unordered_map<std::string, stFileInfo> localFiles{};

		for (const auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
			if (entry.is_symlink()) {
				continue;
			}

			if (entry.is_regular_file() || entry.is_directory()) { 
				std::string filename = entry.path().filename().string();

				if (localFiles.find(filename) == localFiles.end()) {
					LOGI("File: " + filename + " | Dir: " + entry.path().string());
					localFiles.emplace(filename, stFileInfo(filename, entry.path().string()));
				}
			}
		}

		{
			std::lock_guard<std::mutex> lock(mutex);
			files.insert(localFiles.begin(), localFiles.end());
		}

	} catch (const fs::filesystem_error& e) {
		std::cerr << "Filesystem error: " << e.what() << std::endl;
	}
}

void FileIndexer::indexDirectories(const std::vector<fs::path>& paths) 
{
	LOGI("FileIndexer::indexDirectories - start");

	FileIndexer::state = STATE_CACHING_START;
	
	std::vector<std::future<void>> futures;
	size_t numThreads = std::thread::hardware_concurrency();
	size_t chunkSize = (paths.size() + numThreads - 1) / numThreads; 

	for (size_t i = 0; i < paths.size(); i += chunkSize) {
		auto begin = paths.begin() + i;
		auto end = (i + chunkSize < paths.size()) ? paths.begin() + i + chunkSize : paths.end();

		futures.push_back(std::async(std::launch::async, [this, begin, end] {
			for (auto it = begin; it != end; ++it) {
				this->indexDirectory(*it);
			}
		}));
	}

	for (auto& future : futures) {
		future.get();
	}

	FileIndexer::state = STATE_CACHING_END;
	
	LOGI("FileIndexer::indexDirectories - end");
}

std::vector<std::string> FileIndexer::getHomeDir() const {
	LOGI("FileIndexer::getHomeDir");
	return searchPath("/home/");
}

std::vector<std::string> FileIndexer::searchFile(const std::string& query) const {
	std::vector<std::string> results;
	if (!endsWith(query, "\\.[a-zA-Z0-9]+$")) {
		return {};
	}

	for (const auto& [key, fileInfo] : files) 
	{
		if (fileInfo.filename.empty() || fileInfo.path.empty()) {
			continue;
		}

		if (strstr(query.c_str(), fileInfo.filename.c_str())) {
			results.push_back(fileInfo.path);
		}
	}
	return results;
}

std::vector<std::string> FileIndexer::searchPath(const std::string& query) const {
	std::vector<std::string> results;
	if (endsWith(query, "\\.[a-zA-Z0-9]+$")) {
		return {};
	}

	for (const auto& [key, fileInfo] : files) 
	{
		if (fileInfo.path.empty()) {
			continue;
		}

		if (contains(fileInfo.path, query)) {
			results.push_back(fileInfo.path);
		}
	}
	return results;
}

std::vector<std::string> FileIndexer::searchRegex(const std::string& query) const {
	std::vector<std::string> results;
    std::regex re(query);

    results.reserve(files.size());

    std::for_each(std::execution::par, files.begin(), files.end(), [&](const auto& pair) {
        const stFileInfo& fileInfo = pair.second;
        if (std::regex_search(fileInfo.filename, re)) {
            #pragma omp critical
            {
                results.push_back(fileInfo.path);
            }
        }
    });

    return results;
}