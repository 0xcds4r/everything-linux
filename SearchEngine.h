#pragma once

namespace fs = std::filesystem;

class SearchEngine {	
public:
	void IndexDirectory(const fs::path& directory);
	bool FindOne(const std::string& keys);
	std::list<fs::path> SearchByKeywords(const std::string& keyword);
	std::list<fs::path> SearchByRegex(const std::string& regexPattern);

	/*std::vector<fs::path> doSearch(const fs::path& directory, const std::string& name) 
	{
		std::vector<fs::path> result;

		if(strstr(name.c_str(), "/")) 
		{
			size_t lastSlash = name.find_last_of("/");
			std::string dir = name.substr(0, lastSlash);
			std::string fileName = name.substr(lastSlash + 1);

			fs::path newFilePath(dir);
			if(!fs::exists(newFilePath)) {
				return result;
			}
			
			for (const auto& entry : fs::recursive_directory_iterator(newFilePath, fs::directory_options::skip_permission_denied)) {
				if (entry.is_regular_file()) {
					if(entry.path().filename() == fileName) {
						result.push_back(entry.path());
						break;
					}
				}
			}

			if(result.size() == 0) {
				for (const auto& entry : fs::recursive_directory_iterator(newFilePath, fs::directory_options::skip_permission_denied)) {
					result.push_back(entry.path());
				}
			}

			return result;
		}
		
		for (const auto& entry : fs::recursive_directory_iterator(directory, fs::directory_options::skip_permission_denied)) {
			if (entry.is_regular_file() && entry.path().filename() == name) {
				result.push_back(entry.path());
			}
		}

		return result;
	}

	std::vector<fs::path> searchByRegex(const fs::path& directory, const std::regex& pattern) 
	{
		std::vector<fs::path> result;
		for (const auto& entry : fs::recursive_directory_iterator(directory)) {
			if (entry.is_regular_file()) {
				std::string filename = entry.path().filename().string();
				if (std::regex_match(filename, pattern)) {
					result.push_back(entry.path());
				}
			}
		}
		return result;
	}

	std::vector<fs::path> searchByExtension(const fs::path& directory, const std::string& extension) 
	{
		std::vector<fs::path> result;
		for (const auto& entry : fs::recursive_directory_iterator(directory)) {
			if (entry.is_regular_file() && entry.path().extension() == extension) {
				result.push_back(entry.path());
			}
		}
		return result;
	}*/
};