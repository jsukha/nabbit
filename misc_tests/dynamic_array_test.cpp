#include <iostream>
#include <cstdlib>

#include <cilk/cilk.h>

#include <example_util_gettime.h>
#include <dynamic_array.h>


void serial_try_insert_n(DynamicArray<int>* A, int n) {
    for (int i = 0; i < n; i++) {
        bool success;
        do {
            success = A->try_atomic_add(i);
        } while (!success);

        assert(A->get(i) == i);
    }
}

void serial_try_get_n(DynamicArray<int>* A, int n) {
    for (int i = 0; i < n; i++) {
        int current_size = A->size_estimate();
        volatile int val = A->get(current_size-1);

        if (val != (current_size-1)) {
            printf("i = %d", i);
            printf("ERROR: search val returns %d, expected %d\n",
                   val,
                   current_size-1);

      
            A->print();
            printf("ELEMENT from get_with_print: %d\n",
                   A->get_with_print(current_size-1));

      
            printf("The element: %d\n",
                   A->get(current_size-1));

        }
        assert(val == (current_size-1));           
    }
}

void parallel_get(DynamicArray<int>* A, int n, int p) {
    cilk_for(int i = 0; i < p; i++) {
        serial_try_get_n(A, n);
    }  
}

void parallel_search_get_test(int n, int p) {

    int k = 0;
    bool done = false;
    DynamicArray<int>* A = new DynamicArray<int>(2);
    cilk_spawn serial_try_insert_n(A, n);
    do {
        parallel_get(A, n, p-1);
        done = (A->size_estimate() == n);
        printf("Completed gets, iteration %d\n", k);
    } while (!done);
    cilk_sync;
    delete A;
}			      

void serial_dynamic_array_test(int n,
			       bool test_resize) {
    int init_size = n;  
    if (n <= 0) {
        return;
    }

    if (test_resize) {
        init_size = 1 + n/4;
    }
    
    DynamicArray<int>* A = new DynamicArray<int>(init_size);
    assert(A != NULL);

    printf("Testing resize? %d\n", test_resize);
    printf("An empty dynamic array:\n");
    A->print();
    printf("\n");

    for (int i = 0; i < n; i++) {
        A->add(2*i+1);
        assert(A->get(i) == (2*i+1));
    }

    for (int i = 0; i < n; i++) {
        assert(A->get(i) == (2*i+1));
    }

    if (n <= 100) {
        printf("The final dynamic array: \n");
        A->print();
    }
    delete A;
}

void serial_dynamic_array_benchmark(int n,
				    int init_size) {
    assert(n > 0);
    assert(init_size > 0);

    
    long start_time = example_get_time();
    DynamicArray<int>* A = new DynamicArray<int>(init_size);
    assert(A != NULL);

    for (int i = 0; i < n; i++) {
        A->add(2*i + 1);
    }
    long end_time = example_get_time();  
    double total_time = (end_time - start_time) * 1.0f;

    printf("Running time for %d inserts, init_size = %d: %f s\n",	 
           n,
           init_size,
           total_time / 1000.f);
    printf("Average time per insert: %f us\n",
           total_time * 1000.f / n);

    long long computed_sum = 0;
    long long expected_sum = 0;

    for (int i = 0; i < n; i++) {
        expected_sum += (2*i+1);
        computed_sum += A->get(i);
    }

    int final_size = A->size_estimate();
    printf("Final size is: %d\n",
           final_size);
    assert(final_size == n);

    printf("Final sum = %lld. Expected = %lld\n",
           computed_sum, expected_sum);
    assert(computed_sum == expected_sum);

    delete A;
}


void parallel_dynamic_array_benchmark(int n,
				      int init_size) {
    assert(n > 0);
    assert(init_size > 0);

    
    long start_time = example_get_time();
    DynamicArray<int>* A = new DynamicArray<int>(init_size);
    assert(A != NULL);

    cilk_for (int i = 0; i < n; i++) {
        int val = 2*i + 1;
        bool success;
        do {
            success = A->try_atomic_add(val);
        } while (!success);
    }
    //  cilk_sync;
    long end_time = example_get_time();  
    double total_time = (end_time - start_time) * 1.0f;

    printf("Running time for %d inserts, init_size = %d: %f s\n",	 
           n,
           init_size,
           total_time / 1000.f);
    printf("Average time per insert: %f us\n",
           total_time * 1000.f / n);

    long long computed_sum = 0;
    long long expected_sum = 0;

    for (int i = 0; i < n; i++) {
        expected_sum += (2*i+1);
        computed_sum += A->get(i);
    }


    int final_size = A->size_estimate();
    printf("Final size is: %d\n",
           final_size);
    assert(final_size == n);

    printf("Final sum = %lld. Expected = %lld\n",
           computed_sum, expected_sum);


    printf("Final sum = %lld. Expected = %lld\n",
           computed_sum, expected_sum);
    assert(computed_sum == expected_sum);

    delete A;
}


int main(int argc, char *argv[])
{
    int R = 10000;
    if (argc >= 2) {
        R = atoi(argv[1]);
    }

    printf("Value of R: %d\n", R);
    serial_dynamic_array_test(R, false);
    serial_dynamic_array_test(R, true);
    serial_dynamic_array_benchmark(R, 10);
    parallel_dynamic_array_benchmark(R, 10);

    printf("Try with no resize needed\n");
    parallel_dynamic_array_benchmark(R, R + 100);

    printf("Trying dyn_array_search_get_test\n");

    parallel_search_get_test(R, 10);

    printf("PASSED\n");
     
    return 0;
}

