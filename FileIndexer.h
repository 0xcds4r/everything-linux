#pragma once

namespace fs = std::filesystem;

struct stFileInfo {
    std::string filename;
    std::string path;

    stFileInfo(const std::string& name, const std::string& p) 
        : filename(name), path(p) {}

    bool operator==(const stFileInfo& other) const {
        return filename == other.filename && path == other.path;
    }
};

namespace std {
    template <>
    struct hash<stFileInfo> {
        std::size_t operator()(const stFileInfo& fi) const {
            return hash<std::string>()(fi.filename) ^ (hash<std::string>()(fi.path) << 1);
        }
    };
}

enum eState {
	STATE_NONE = -1,
	STATE_INIT = 0,
	STATE_CACHING_START = 1,
	STATE_CACHING = 2,
	STATE_CACHING_END = 3
};

class FileIndexer {
public:
	FileIndexer() = default;
	static void Initialise();

	void indexDirectory(const fs::path& path);
	void indexDirectories(const std::vector<fs::path>& paths);
	std::vector<std::string> getHomeDir() const;
	std::vector<std::string> searchFile(const std::string& query) const;
	std::vector<std::string> searchPath(const std::string& query) const;
	std::vector<std::string> searchRegex(const std::string& query) const;

	static eState getState();
private:
	static eState state;
	std::unordered_map<std::string, stFileInfo> files;
	mutable std::mutex mutex;
};

std::vector<fs::path> getAcceptedDirs();