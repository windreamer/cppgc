project(AllTest)
include_directories (../include)
include_directories (./tut)
SET(EXECUTABLE_OUTPUT_PATH ${AllTest_BINARY_DIR} CACHE INTERNAL  "Where to put the executables for CMake"  )
SET(LIBRARY_OUTPUT_PATH ${AllTest_BINARY_DIR} CACHE INTERNAL   "Where to put the libraries for CMake"  )
IF (WIN32)
    IF (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
        SET( CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Flags used by the C++ compiler during release builds." FORCE )
        SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO "-DNDEBUG -g" CACHE STRING "Flags used by the C++ compiler during release with debug info builds." FORCE )
    ENDIF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
ENDIF(WIN32)
FILE(GLOB TEST_SRCS globbing ./test*.cpp)
FOREACH(VAR ${TEST_SRCS})
    GET_FILENAME_COMPONENT(SRCNAME_WE ${VAR} NAME_WE)
    SET (TEST_LIST ${SRCNAME_WE} ${TEST_LIST})
ENDFOREACH(VAR)
FILE(GLOB_RECURSE SRC_INCLUDE globbing ../include/*.h)
add_executable (AllTest main.cpp ${TEST_SRCS} ${SRC_INCLUDE})
FOREACH(VAR ${TEST_LIST})
    ADD_TEST(
    ${VAR} ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CPPGC_SOURCE_DIR}/tests"
    "${CPPGC_BINARY_DIR}/tests/${VAR}"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-project AllTest
    --test-command AllTest ${VAR}
    )    
ENDFOREACH(VAR)
