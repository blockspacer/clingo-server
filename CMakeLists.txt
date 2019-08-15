cmake_minimum_required(VERSION 3.1)
project(CLINGOSERVER VERSION 0.1.0)

message("CLINGOSERVER_SOURCE_DIR: ${CLINGOSERVER_SOURCE_DIR}")
message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CLASP_BUILD_APP OFF)
set(CLASP_INSTALL_LIB ON)
set(LIB_POTASSCO_INSTALL_LIB ON)

set(CLINGO_BUILD_SHARED OFF)
set(CLINGO_BUILD_STATIC ON)

# Include flatbuffers generation and include/link directories
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Flatbuffers.cmake)

#add_subdirectory(clingo)

#add_library(Pyclingo::Pyclingo INTERFACE IMPORTED)
#set_property(TARGET Pyclingo::Pyclingo PROPERTY INTERFACE_LINK_LIBRARIES "${PYTHON_LIBRARIES}")

enable_testing()

add_subdirectory(libcommscpp)

#add_subdirectory(worker)

#if (NOT CLINGO_BUILD_STATIC AND (CLINGO_BUILD_SHARED OR PYTHONLIBS_FOUND OR LUA_FOUND))
#    foreach(target ${worker_library_targets})
#        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
#    endforeach()
#endif()
