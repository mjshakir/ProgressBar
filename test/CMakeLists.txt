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
        GIT_SHALLOW    TRUE # Do a shallow clone to speed up the process
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Function to setup a test executable
function(create_test_target TARGET_NAME SOURCE_FILE)
  # Correcting the file path according to the directory structure
  add_executable(${TARGET_NAME} ${SOURCE_FILE})
  target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
  
  # Link the main project library and Google Test
  target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME})
  if(GTest_FOUND)
      target_link_libraries(${TARGET_NAME} PRIVATE GTest::GTest GTest::Main)
  else()
      target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main)
  endif()

  # Enable testing and add the test to CTest
  enable_testing()
  add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endfunction()

# Add the tests without the 'test/' prefix since we are already in the test directory
create_test_target(${PROJECT_NAME}_CircularBufferTest CircularBufferTest.cpp)
create_test_target(${PROJECT_NAME}_ProgressBarTest ProgressBarTest.cpp)

# Link libraries conditionally based on test requirements
# For example, assuming CircularBuffer requires Boost and TBB
if(Boost_FOUND AND TBB_FOUND)
    target_link_libraries(${PROJECT_NAME}_CircularBufferTest PRIVATE ${Boost_LIBRARIES} TBB::tbb)
endif()

# Assuming ProgressBar requires fmt and TBB
if(fmt_FOUND AND TBB_FOUND)
    target_link_libraries(${PROJECT_NAME}_ProgressBarTest PRIVATE fmt::fmt TBB::tbb)
endif()