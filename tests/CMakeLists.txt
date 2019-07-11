# Generic function for setting up a unit test run 
# using Cilk, and adding it to ctest
function (setup_unit_test suite_name test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_include_directories(${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/util)
    target_link_libraries(${test_name} PRIVATE Nabbit cilkrts)
    target_compile_options(${test_name} PRIVATE ${CMAKE_CILK_FLAGS})
    add_test(${suite_name}_${test_name} ${test_name})
endfunction()


# Generic function for setting up a unit test that is 
# Cilk-serialized, add to ctest
function (setup_serialized_unit_test suite_name test_name)
    add_executable(serialized_${test_name} ${test_name}.cpp)
    target_include_directories(serialized_${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/util)
    target_link_libraries(serialized_${test_name} PRIVATE Nabbit cilkrts)
    target_compile_options(serialized_${test_name} PRIVATE ${CMAKE_CILK_FLAGS} ${CMAKE_CILK_SERIALIZE_FLAG})
    add_test(${suite_name}_serialized_${test_name} serialized_${test_name})
endfunction()


add_subdirectory(arrays)
add_subdirectory(concurrent)

