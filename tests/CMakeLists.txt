# SPDX-License-Identifier: MPL-2.0

project(dxFeedNativeAPITests LANGUAGES CXX)

set(DXFC_INCLUDE_DIRS ../include)

add_library(dxFeedNativeAPITestsMain OBJECT main.cpp)
target_link_libraries(dxFeedNativeAPITestsMain PRIVATE Catch2::Catch2)
target_include_directories(dxFeedNativeAPITestsMain PUBLIC ${DXFC_INCLUDE_DIRS})

set(DXFC_TEST_SOURCES dxFeedNativeSystemTest.cpp)

string(REPLACE ".cpp" "" DXFC_TEST_BASENAMES "${DXFC_TEST_SOURCES}")
set(DXFC_TEST_TARGETS ${DXFC_TEST_BASENAMES})

foreach (dxfc_test_target ${DXFC_TEST_TARGETS})
    add_executable(${dxfc_test_target} ${dxfc_test_target}.cpp)
    target_include_directories(${dxfc_test_target} PUBLIC ${DXFC_INCLUDE_DIRS})
    target_link_libraries(${dxfc_test_target} PRIVATE dxFeedNativeAPITestsMain dxFeedNativeAPI Catch2::Catch2)
    set_property(TARGET ${dxfc_test_target} PROPERTY CXX_STANDARD 20)
    set_property(TARGET ${dxfc_test_target} PROPERTY CXX_EXTENSIONS OFF)
    add_test(NAME ${dxfc_test_target} COMMAND ${dxfc_test_target})
endforeach ()