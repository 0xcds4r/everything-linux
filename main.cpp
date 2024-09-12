#include <main.h>
#include <UI/UIManager.h>

GLFWwindow* window = nullptr;

void signalHandler(int signal) {
	std::cerr << "Signal " << signal << " received." << std::endl;

	void* array[512];
	size_t size;

	size = backtrace(array, 512);
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

void DoCopyToClipBoard(const char* path) {
	if(!path) {
		return;
	}
	LOGI(std::string(path) + " -- copied to clipboard");
	glfwSetClipboardString(window, path);
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
	glfwSwapInterval(1); 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard; 

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	ImGui::StyleColorsDark();

	UIManager::Initialise();
	// SetupImGuiStyle();

	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		UIManager::Draw();

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
