#include <main.h>
#include <SearchEngine.h>

std::list<fs::path> indexResults;
std::mutex indexMutex; 

void SearchEngine::IndexDirectory(const fs::path& directory) 
{
	if(!fs::exists(directory)) {
		LOGI("Can't index dir");
		return;
	}

	std::async(std::launch::async, [directory]() {
		std::lock_guard<std::mutex> lock(indexMutex); 
		indexResults.clear();

		try {
			for (const auto& entry : fs::recursive_directory_iterator(directory, fs::directory_options::skip_permission_denied)) {
				LOGI("Indexing path: " + entry.path().string());
				indexResults.push_back(entry.path());
			}
		} catch (const std::exception& e) {
			LOGI("Error during indexing: " + std::string(e.what()));
		}

		LOGI("Indexing completed");
	});
}

bool SearchEngine::FindOne(const std::string& keys) 
{
	auto it = std::find_if(indexResults.begin(), indexResults.end(),
		[&keys](const fs::path& path) {
			return strstr(path.string().c_str(), keys.c_str());
		});

	if (it != indexResults.end()) {
		LOGI("File found: " + it->string());
		return true;
	} else {
		LOGI("File not found");
		return false;
	}
}

std::list<fs::path> SearchEngine::SearchByKeywords(const std::string& keyword) 
{
	std::list<fs::path> results{};

	// filter invalid chars
	if (!keyword.empty()) {
		for (char c : keyword) {
        	if (c == '\0' || c == '\\') {
            	return SearchByKeywords("");
        	}
    	}
	}
	
    for (const auto& path : indexResults) {
        if (!fs::exists(path)) {
            LOGE("Path does not exist: " + path.string());
            break;
        }

        std::string pathStr;
        try {
            pathStr = path.string();
        } catch (const std::exception& e) {
            LOGE("Error converting path to string: " + std::string(e.what()));
            continue; 
        }

        if (pathStr.find(keyword) != std::string::npos) {
            LOGI("Found match: " + pathStr);
            results.push_back(path);
        }
    }

    if (results.empty()) {
        LOGI("No matches found for keyword: " + keyword);
        return SearchByKeywords("");
    }

	return results;
}

std::mutex resultsMutex;

void SearchInChunk(const std::list<fs::path>::iterator& start, const std::list<fs::path>::iterator& end, 
				   const std::regex& pattern, std::list<fs::path>& results) {
	for (auto it = start; it != end; ++it) {
		if (std::regex_search(it->string(), pattern)) {
			std::lock_guard<std::mutex> lock(resultsMutex); // Protect the results list
			LOGI("Found match: " + it->string());
			results.push_back(*it);
		}
	}
}

std::list<fs::path> SearchEngine::SearchByRegex(const std::string& regexPattern) {
	std::list<fs::path> results;

	try {
		std::regex pattern(regexPattern);
		int numThreads = std::thread::hardware_concurrency();
		if (numThreads == 0) {
			numThreads = 4;
		}

		auto totalPaths = indexResults.size();
		auto chunkSize = totalPaths / numThreads;

		std::vector<std::thread> threads;
		auto it = indexResults.begin();

		for (int i = 0; i < numThreads - 1; ++i) {
			auto chunkEnd = std::next(it, chunkSize);
			threads.emplace_back(SearchInChunk, it, chunkEnd, std::cref(pattern), std::ref(results));
			it = chunkEnd; 
		}
		
		threads.emplace_back(SearchInChunk, it, indexResults.end(), std::cref(pattern), std::ref(results));

		for (auto& t : threads) {
			if (t.joinable()) {
				t.join();
			}
		}

		if (results.empty()) {
			LOGI("No matches found for regex: " + regexPattern);
		}

	} catch (const std::regex_error& e) {
		LOGI("Invalid regex pattern: " + regexPattern + ", Error: " + e.what());
	} catch (const std::exception& e) {
		LOGI("Exception during search: " + std::string(e.what()));
	} catch (const std::logic_error& e) {
		LOGI("Exception logic error: " + std::string(e.what()));
	}

	return results;
}