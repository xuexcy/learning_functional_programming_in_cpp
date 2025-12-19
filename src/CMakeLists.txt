if(DEFINED ENV{build_output_bin_dir})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{build_output_bin_dir})
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)
endif()
message("CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/opt/homebrew/Cellar/gcc/14.2.0_1/include/c++/14")

set(chapter chapter_01)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)

set(chapter chapter_02)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)

set(chapter chapter_03)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)
target_link_libraries(${main_name} boost_phoenix)

set(chapter chapter_04)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)

set(chapter chapter_05)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)

set(chapter chapter_06)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)

set(chapter chapter_07)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)
target_link_libraries(${main_name} range-v3::range-v3)

set(chapter chapter_08)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)

set(chapter chapter_09)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)

set(chapter chapter_10)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)
target_link_libraries(${main_name} range-v3::range-v3)

set(chapter chapter_11)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)

set(chapter chapter_12)
include_directories(${chapter})
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)
#add_subdirectory(${chapter}/actor)
target_link_libraries(${main_name} boost_asio boost_system nlohmann_json::nlohmann_json)

set(chapter chapter_13)
set(main_name ${chapter}_main)
add_executable(${main_name} ${chapter}/main.cc)
