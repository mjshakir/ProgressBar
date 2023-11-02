# Try Finding Installed Google Test
find_package(GTest QUIET)

# Use Installed Version or Fetch
if(NOT GTest_FOUND)
    message(STATUS "Google Test not found. Fetching from GitHub repository...")

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(googletest)
endif()


# Add the Unit Test
file(GLOB TEST_SOURCES "*.cpp")
add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
target_include_directories(${PROJECT_NAME}_tests PUBLIC ${CMAKE_SOURCE_DIR}/include)
if(TBB_FOUND AND GTest_FOUND)
    target_link_libraries(${PROJECT_NAME}_tests GTest::GTest GTest::Main ${PROJECT_NAME} TBB::tbb)
elseif(TBB_FOUND)
    target_link_libraries(${PROJECT_NAME}_tests gtest gtest_main ${PROJECT_NAME} TBB::tbb)
elseif(GTest_FOUND)
    target_link_libraries(${PROJECT_NAME}_tests GTest::GTest GTest::Main ${PROJECT_NAME})
else()
    target_link_libraries(${PROJECT_NAME}_tests gtest gtest_main ${PROJECT_NAME})
endif()

# Enable Testing and Add the Test to CTest
enable_testing()
add_test(NAME ${PROJECT_NAME}_unit_tests COMMAND ${PROJECT_NAME}_tests)
