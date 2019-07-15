function (gen_sw_block block_size test_N)
    set(test_name "swblock_${block_size}")
    add_executable(${test_name} sw_compute.cpp)
    target_include_directories(${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/util)
    target_link_libraries(${test_name} PRIVATE Nabbit cilkrts pthread)
    target_compile_options(${test_name} PRIVATE ${CMAKE_CILK_FLAGS} -DBLOCK_VALUE=${block_size})
    add_test(run_${test_name} swblock_${block_size} ${test_N} ${test_N})
endfunction()

function (gen_sw_blockCons block_size test_N)
    set(test_name "swblockCons_${block_size}")
    add_executable(${test_name} sw_compute.cpp)
    target_include_directories(${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/util)
    target_link_libraries(${test_name} PRIVATE Nabbit cilkrts pthread)
    target_compile_options(${test_name} PRIVATE ${CMAKE_CILK_FLAGS} -DBLOCK_VALUE=${block_size} -DCOMPUTE_CONSTANT_EF)
    add_test(run_${test_name} swblock_${block_size} ${test_N} ${test_N})
endfunction()

function (gen_all_block_tests block_size test_N)
    gen_sw_block(${block_size} ${test_N})
    gen_sw_blockCons(${block_size} ${test_N})
endfunction()


gen_all_block_tests(1 128)
gen_all_block_tests(2 128)
gen_all_block_tests(4 128)
gen_all_block_tests(8 128)
gen_all_block_tests(16 512)
gen_all_block_tests(32 512)
gen_all_block_tests(64 512)
gen_all_block_tests(128 512)


function (run_test_script script_name)
    configure_file(${script_name}.sh ${script_name}.sh COPYONLY)
    add_custom_command(OUTPUT dummy_run_${script_name} COMMAND VERBATIM ./${script_name}.sh)
    # ALL not specified, so this target is not automatically run as part of a default build
    add_custom_target(${script_name}_run DEPENDS swblock_1 swblock_2 swblock_4 swblock_8 swblock_16 swblock_32 dummy_run_${script_name})
endfunction()

run_test_script(sw_btest)
run_test_script(sw_test)
