set(ZINDEX_TEST_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/catch.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LineFinderTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SqliteTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RegExpTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RegExpIndexerTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TempDir.h
        ${CMAKE_CURRENT_SOURCE_DIR}/TempDir.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PrettyBytesTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IndexTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RangeFetcherTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FieldIndexerTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ExternalIndexerTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LogTest.cpp)
add_executable(unit-tests ${ZINDEX_TEST_FILES})
target_link_libraries(unit-tests ${PROJECT_NAME}_core ${DEPENDENCY_LIB})
find_program(MEMORYCHECK_COMMAND valgrind)

add_test(NAME unit-tests
        COMMAND unit-tests)
add_subdirectory(python)