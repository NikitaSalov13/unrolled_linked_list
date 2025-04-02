add_executable(${PROJECT_NAME} main.cpp
        ../lib/unrolled_list.h)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
