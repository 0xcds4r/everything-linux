#pragma once

#define APP_VERSION "0.0.1"
#define MAX_WIDTH 	1280
#define MAX_HEIGHT 	720

#include <external/imgui/imgui.h>
#include <external/imgui/imgui_impl_glfw.h>
#include <external/imgui/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <regex>
#include <cstdlib>  
#include <mutex>
#include <thread>
#include <future>
#include <chrono> 
// #include <new> 
#include <csignal>
#include <execinfo.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string>
#include <iterator>