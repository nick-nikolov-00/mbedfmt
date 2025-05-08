cmake_minimum_required(VERSION 3.18)
project(mbedfmt VERSION 0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
        ctre
        GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions.git
        GIT_TAG        v3.9.0
)
FetchContent_MakeAvailable(ctre)

add_library(mbedfmt INTERFACE)
target_compile_features(mbedfmt INTERFACE cxx_std_17)

target_include_directories(mbedfmt INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(mbedfmt INTERFACE ctre)

install(TARGETS mbedfmt
        EXPORT mbedfmtTargets
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT mbedfmtTargets
        FILE mbedfmtTargets.cmake
        DESTINATION lib/cmake/mbedfmt
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/mbedfmtConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mbedfmtConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/mbedfmtConfig.cmake"
        INSTALL_DESTINATION lib/cmake/mbedfmt
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mbedfmtConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mbedfmtConfigVersion.cmake"
        DESTINATION lib/cmake/mbedfmt
)

option(MBEDFMT_BUILD_TESTS "Build mbedfmt test executable" ON)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS test/*.cpp)

if(MBEDFMT_BUILD_TESTS)
    enable_testing()
    add_executable(mbedfmt_tests ${TEST_SOURCES})
    target_link_libraries(mbedfmt_tests PRIVATE mbedfmt)
    add_test(NAME mbedfmt_test COMMAND mbedfmt_tests)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    add_executable(mbedfmt_dev main.cpp)
    target_link_libraries(mbedfmt_dev PRIVATE mbedfmt)
endif()
