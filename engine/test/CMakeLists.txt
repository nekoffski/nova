
find_package(GTest REQUIRED)

set(GTEST_LINK gtest::gtest)
set(MOCK_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})
set(SL_LIBS starlight-core starlight-renderer starlight-event starlight-window starlight-app)

function(add_test_file TEST_FILE MODULE)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WLE)
    set(TEST_EXE_NAME starlight_${MODULE}_${TEST_NAME})
    message("\t-> ${TEST_EXE_NAME}")
    add_executable(${TEST_EXE_NAME} ${TEST_FILE})

    target_link_libraries(${TEST_EXE_NAME} PUBLIC ${GTEST_LINK} starlight-${MODULE})
    target_include_directories(${TEST_EXE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${SL_INCLUDE} ${MOCK_INCLUDE})
    add_test(NAME ${MODULE}.${TEST_NAME} COMMAND ${TEST_EXE_NAME})
endfunction()

file(GLOB TEST_MODULES LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(TEST_MODULE ${TEST_MODULES})
    if(IS_DIRECTORY ${TEST_MODULE})
        get_filename_component(MODULE_NAME ${TEST_MODULE} NAME)
        if(NOT ${MODULE_NAME} STREQUAL "mock")
            file(GLOB TEST_FILES ${TEST_MODULE}/Tests*.cpp)
            message("-- Processing test module '${MODULE_NAME}'")
            foreach(TEST_FILE ${TEST_FILES})
                message(${TEST_FILE})
                add_test_file(${TEST_FILE} ${MODULE_NAME})
            endforeach()
        endif()
    endif()
endforeach()
