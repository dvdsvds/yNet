cmake_minimum_required(VERSION 3.20)
project(yNet VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE YNET_SOURCES src/*.cpp)

add_library(ynet STATIC ${YNET_SOURCES})
target_include_directories(ynet PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(ynet PUBLIC OpenSSL::SSL OpenSSL::Crypto)

target_compile_options(ynet PRIVATE
    -Wall -Wextra -Wpedantic
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
    -fPIE
)
target_link_options(ynet PRIVATE
    -pie
    -Wl,-z,relro,-z,now
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_executable(example examples/main.cpp)
    target_link_libraries(example ynet)

    add_executable(test_client examples/client.cpp)
    target_link_libraries(test_client ynet)
endif()

install(DIRECTORY include/ynet DESTINATION include)

install(TARGETS ynet EXPORT ynetTargets ARCHIVE DESTINATION lib)
install(EXPORT ynetTargets NAMESPACE ynet:: DESTINATION lib/cmake/ynet)
