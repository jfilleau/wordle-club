add_executable(human_readable human_readable_test.cpp)
target_link_libraries(human_readable PRIVATE wordlelib)
add_test(NAME "Human Redable Test" COMMAND human_readable)