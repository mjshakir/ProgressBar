cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Determine if ProgressBar is built as a standalone project or included by other projects
set(PROGRESSBAR_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(PROGRESSBAR_STANDALONE_PROJECT ON)
endif()

# Get the name of the folder and use it as the project name
get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROGRESSBAR_PROJECT_NAME "${FOLDER_NAME}_progressbar" CACHE STRING "ProgressBar project name")

project(${PROGRESSBAR_PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard, require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify the output directories for standalone project
if(PROGRESSBAR_STANDALONE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# Build options
option(BUILD_PROGRESSBAR_SHARED_LIBS "Build using shared libraries" ON)
option(PROGRESSBAR_BUILD_TESTS "Build the ProgressBar tests" ${PROGRESSBAR_STANDALONE_PROJECT})
option(PROGRESSBAR_BUILD_EXAMPLE "Build the ProgressBar example application" ${PROGRESSBAR_STANDALONE_PROJECT})

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
# file(GLOB SOURCES "src/*.cpp")
set(SOURCES src/ProgressBar.cpp)
if(NOT Boost_FOUND)
    list(APPEND SOURCES src/CircularBuffer.cpp)
endif()


if(BUILD_PROGRESSBAR_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()

add_library(ProgressBar::progressbar ALIAS ${PROJECT_NAME})


# Specify properties and requirements for the library
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies
if(fmt_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FMT)
    message(STATUS "Found fmt")
else()
    message(STATUS "fmt not found")
endif()

if(TBB_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_TBB)
    message(STATUS "Found TBB")
else()
    message(STATUS "TBB not found")
endif()

if(Boost_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_BOOST)
    message(STATUS "Found Boost")
else()
    message(STATUS "Boost not found")
endif()

# Specify the public headers for the library
set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "include/ProgressBar.hpp;include/CircularBuffer.hpp")

# Only build the example and tests if this is the main project
if(PROGRESSBAR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(PROGRESSBAR_BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}_example example/main.cpp)
    target_link_libraries(${PROJECT_NAME}_example PRIVATE ${PROJECT_NAME})
endif()

# Compiler and Linker Options for different build types
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /MDd")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        "$<$<CONFIG:Release>:/O2>" 
        "$<$<CONFIG:Debug>:/Od>" 
        "$<$<CONFIG:Debug>:/Zi>"
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(${PROJECT_NAME} PRIVATE /DEBUG)
    endif()
else()
    # For non-MSVC compilers
    target_compile_options(${PROJECT_NAME} PRIVATE 
        "$<$<CONFIG:Release>:-O3>"
    )
    
    # Conditionally include AddressSanitizer flags only if standalone and in Debug mode
    if(PROGRESSBAR_STANDALONE_PROJECT AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")
        set(SANITIZE_FLAGS "$<$<CONFIG:Debug>:-fsanitize=address>")
        target_compile_options(${PROJECT_NAME} PRIVATE 
            "$<$<CONFIG:Debug>:-g>"
            "$<$<CONFIG:Debug>:-O0>"
            ${SANITIZE_FLAGS}
        )
        target_link_options(${PROJECT_NAME} PRIVATE ${SANITIZE_FLAGS})
    endif()
endif()

# Installation and export setup
if(PROGRESSBAR_STANDALONE_PROJECT)
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
        NAMESPACE ProgressBar::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/ProgressBarConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/ProgressBarConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ProgressBar
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ProgressBarConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ProgressBarConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ProgressBar
    )
endif()

# Set THREADPOOL_LIBRARIES variable
if(PROGRESSBAR_STANDALONE_PROJECT)
    set(PROGRESSBAR_LIBRARIES ProgressBar::progressbar)
else()
    set(PROGRESSBAR_LIBRARIES ProgressBar::progressbar PARENT_SCOPE)
endif()