function (setup_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE Nabbit cilkrts)
    target_compile_options(${test_name} PRIVATE ${CMAKE_CILK_FLAGS})
    add_test(arrays_${test_name} ${test_name})
endfunction()

enable_testing()
setup_test(test_arrays)
setup_test(test_convert)

