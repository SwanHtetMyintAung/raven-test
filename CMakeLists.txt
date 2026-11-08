# NOTE: This is root cmake file
# NOTE: The cmake file is made to work with msvc for now

cmake_minimum_required(VERSION 3.31)

# This is how you create variable in cmake
set(PROJ_NAME Your_proj_name)

#This is how you use the variable: ${Variable name}
project(${PROJ_NAME} LANGUAGES C CXX RC)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER "cl.exe" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "cl.exe" CACHE FILEPATH "C++ compiler" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") 

add_subdirectory(Dependencies/Raven)

# NOTE: This create output folder for executable. For example, in debug build, this should
# NOTE: output to, build/your_proj_name/Debug_Windows/
set(MY_CUSTOM_BIN_DIR "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/$<CONFIG>-$<PLATFORM_ID>")

# Force all executable targets to compile into this specific directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_CUSTOM_BIN_DIR}")

# NOTE: This is the code that actually calls the actual project cmake file, the above code are setup only
add_subdirectory(src)
