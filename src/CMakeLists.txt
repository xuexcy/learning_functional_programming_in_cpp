if(DEFINED ENV{build_output_bin_dir})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{build_output_bin_dir})
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)
endif()
message("CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/opt/homebrew/Cellar/gcc/14.2.0_1/include/c++/14")

set(main_name chapter_01_main)
add_executable(${main_name} chapter_01/main.cc)

set(main_name chapter_02_main)
add_executable(${main_name} chapter_02/main.cc)


set(main_name chapter_03_main)
add_executable(${main_name} chapter_03/main.cc)
target_link_libraries(${main_name} boost_phoenix)

set(main_name chapter_04_main)
add_executable(${main_name} chapter_04/main.cc)
