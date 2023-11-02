cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Get the name of the folder and use it as the project name
get_filename_component(PROJECT_NAME ${CMAKE_SOURCE_DIR} NAME)
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard, require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Build options
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Force colored output for better diagnostics
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF)
if(${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options(-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options(-fcolor-diagnostics)
    endif()
endif()

# Compiler options
add_compile_options(-Wall -Wextra -Wpedantic)

# Find required packages
find_package(fmt QUIET)
find_package(TBB QUIET)
find_package(Boost QUIET)

# Create the library
file(GLOB SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} ${SOURCES})

# Specify properties and requirements for the library
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


if(fmt_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FMT)
endif()

if(TBB_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_TBB)
endif()

if(Boost_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_BOOST)
endif()

# Specify the public headers for the library
set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "include/ProgressBar.hpp;include/CircularBuffer.hpp")

# Add an executable for the example
add_executable(${PROJECT_NAME}_example example/main.cpp)
target_link_libraries(${PROJECT_NAME}_example PRIVATE ${PROJECT_NAME})

# Compiler and Linker Options for different build types
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
        target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    endif()
endif()

# Enable testing before adding subdirectories
enable_testing()

# Add subdirectory for tests
add_subdirectory(test)

# Installation setup
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

# Exporting and versioning
install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)