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
option(PROGRESSBAR_BUILD_EXAMPLE "Build the ProgressBar example application" ${PROGRESSBAR_STANDALONE_PROJECT})
option(WITH_FMT "Include fmt library" ON)
option(WITH_TBB "Include TBB library" ON)
option(WITH_BOOST "Include Boost library" ON)

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
if(MSVC)
    # MSVC specific warning flags
    add_compile_options(/W4)  # Equivalent to -Wall in GCC/Clang
else()
    # GCC/Clang specific warning flags
    add_compile_options(-Wall -Wextra -Wpedantic -pedantic-errors -Werror=format-security -fasynchronous-unwind-tables)
endif()

# Create the library
set(SOURCES src/ProgressBar.cpp)

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
if(WITH_FMT)
    find_package(fmt QUIET)
    if(fmt_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FMT)
    else()
        message(STATUS "fmt not found or disabled")
    endif()
endif()

if(WITH_TBB)
    find_package(TBB QUIET)
    if(TBB_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_TBB)
    else()
        message(STATUS "TBB not found or disabled")
    endif()
endif()

if(WITH_BOOST)
    find_package(Boost QUIET)
    if(Boost_FOUND)
        target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
        target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_BOOST)
    else()
        message(STATUS "Boost not found or disabled")
        # Add external submodule projects
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external_libraries/CircularBuffer)
            add_subdirectory(external_libraries/CircularBuffer)
        else()
            message(FATAL_ERROR "external_libraries/CircularBuffer directory not found")
        endif()
    endif()
else()
    # Add external submodule projects
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external_libraries/CircularBuffer)
        add_subdirectory(external_libraries/CircularBuffer)
    else()
        message(FATAL_ERROR "external_libraries/CircularBuffer directory not found")
    endif()
endif()

# Link CircularBuffer if Boost is not used
if(NOT WITH_BOOST OR NOT Boost_FOUND)
    if(TARGET CircularBuffer::CircularBuffer)
        target_link_libraries(${PROJECT_NAME} PRIVATE CircularBuffer::CircularBuffer)
    else()
        message(FATAL_ERROR "CircularBuffer target not found")
    endif()
endif()

# Specify the public headers for the library
set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "include/ProgressBar.hpp")

if(PROGRESSBAR_BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}_example example/main.cpp)
    target_link_libraries(${PROJECT_NAME}_example PRIVATE ${PROJECT_NAME})
    add_dependencies(${PROJECT_NAME}_example ${PROJECT_NAME})
endif()

# Compiler and Linker Options for different build types and platforms
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

    set_target_properties(${PROJECT_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    )
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
# Installation and export setup
if(PROGRESSBAR_STANDALONE_PROJECT)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    )

    if(NOT WITH_BOOST OR NOT Boost_FOUND)
        install(TARGETS CircularBuffer
            EXPORT ${PROJECT_NAME}Targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

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

# Set PROGRESSBAR variable
if(PROGRESSBAR_STANDALONE_PROJECT)
    set(PROGRESSBAR_LIBRARIES ${PROJECT_NAME}::progressbar)
else()
    set(PROGRESSBAR_LIBRARIES ${PROJECT_NAME}::progressbar PARENT_SCOPE)
endif()