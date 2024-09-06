Project Structure

The project uses a C++17 standard and builds a target executable named everything located in the builds/ directory. All object files are stored in the objs/ directory to keep the root directory clean.
Makefile Overview
```
    Compiler: g++
    Standard: C++17
    Flags: -Wall (enables all compiler warnings), -g (includes debugging information), and -std=c++17 (sets the C++ standard).
    Libraries:
        -lGL (OpenGL),
        -lGLEW (OpenGL Extension Wrangler),
        -lglfw (GLFW for window management),
        -ldl (dynamic linking),
        -lpthread (POSIX threads).
        -lstdc++fs (for filesystem support).
```
Makefile Targets
1. Build the Project

To compile and link the project, run the following command:
```
bash
```
```
make
```
This will:
```
    Recursively find all .cpp files in the project directory.
    Compile each .cpp file into an object file (.o), which will be stored in the objs/ directory.
    Link the object files into an executable called everything located in builds/.
```
2. Clean the Project

To remove all compiled object files and the target executable, run:

```
bash
```
```
make clean
```

This will delete all files in the objs/ directory and the builds/everything executable.
3. Run the Executable

Once the project is built, you can run the executable by typing:

```
bash
```

```
make run
```

# This will compile the project (if necessary) and then execute the program.
## Directory Structure
```
    src/: Contains all the source code (.cpp files).
    objs/: Object files (.o) generated during compilation.
    builds/: The final executable everything.
```
## Customization
```
    CXXFLAGS: You can modify this variable in the Makefile to add more compiler options or adjust optimization/debug settings.
    LIBS: Modify this variable to link additional libraries if needed.
```
## Requirements

# To build and run this project, ensure you have the following installed:
```
    g++ compiler (supporting C++17)
    OpenGL (libGL)
    GLEW (libGLEW)
    GLFW (libglfw)
    POSIX threads (libpthread)
```
# Example Commands

    Build: ```make```
    Run: ```make run```
    Clean: ```make clean```