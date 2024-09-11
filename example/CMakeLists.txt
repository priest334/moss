# example

set(CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/${PROJECT_NAME})
source_group("src" "moss.cpp")
add_executable(example "moss.cpp")
target_link_libraries(example moss)

