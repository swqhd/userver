project(userver-easy-samples-key-value CXX)

add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} userver::easy)

userver_testsuite_add_simple(DUMP_CONFIG True)
