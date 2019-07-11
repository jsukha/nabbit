#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cilk/cilk.h>

#include <example_util_gettime.h>
#include <check_sort.h>
#include <concurrent_hash_table.h>


bool check_hash_insert(ConcurrentHashTable* H, int R) {
    long long final_size;  
    long long* a = H->get_keys(&final_size);
    std::sort(a, a+final_size);
    delete [] a;
    return true;
}


// Tries to insert elements [0, 1, .. R-1] into the list. 
void all_hash_insert(ConcurrentHashTable* H, int R) {
  
    int statusCounts[OP_LAST];
  
    for (int i = OP_NULL; i < OP_LAST; i++) {
        statusCounts[i] = 0;
    }

    for (int i = 0; i < R; i++) {
        LOpStatus code = OP_FAILED;
        void* return_val;
        void* i_as_ptr = reinterpret_cast<void*>(std::size_t(i));
        // Retry insert until return code is not OP_FAILED.
        while (code == OP_FAILED) {
            return_val = H->insert_if_absent(i,
                                             i_as_ptr, 
                                             &code);
            statusCounts[code]++;
        }
        assert(return_val == i_as_ptr);
    }

    std::cout << "Final code results: ";
    for (int i = OP_NULL; i < OP_LAST; i++) {
        std::cout << "(" << i << ", " << statusCounts[i] << ")  ";
    }
    std::cout << "\n";
}

// Tries to insert n random elements into L, with each element being
// chosen uniformly from the range [0 .. R-1].
void test_hash_insert(ConcurrentHashTable* H, int R, int n) {

    bool print_output = false;
    int statusCounts[OP_LAST];
    for (int i = OP_NULL; i < OP_LAST; i++) {
        statusCounts[i] = 0;
    }

    if (print_output) {
        std::cout << "Test hash insert: R = " << R << ", n = " << n << "\n";
    }

    for (int i = 0; i < n; i++) {
        int rand_num = rand() % R;
        void* rand_as_ptr = reinterpret_cast<void*>(std::size_t(rand_num));
        LOpStatus code = OP_FAILED;
        void* return_val;

        while (code == OP_FAILED) {
            return_val = H->insert_if_absent(rand_num,
                                             rand_as_ptr, 
                                             &code);
            statusCounts[code]++;
        }
        assert(return_val == rand_as_ptr);
    }

    if (print_output) {
        std::cout << "Code results: ";
        for (int i = OP_NULL; i < OP_LAST; i++) {
            std::cout << "(" << i << ", " << statusCounts[i] << ")  ";
        }
        std::cout << "\n";
    }
}




int main(int argc, char *argv[])
{
    int R = 10000;
    if (argc >= 2) {
        R = atoi(argv[1]);
    }

    std::cout << "Value of R: " << R << "\n";
    ConcurrentHashTable* H = new ConcurrentHashTable(5*R); // + ((R % 10) > 0));
    std::cout << "An empty hash table\n";
    H->print_table();


    long start_time = example_get_time();
    for (int i = 0; i < 20; i++) {
        cilk_spawn test_hash_insert(H, R, R/20);
    }
    cilk_sync;
    long end_time = example_get_time();

    int num_inserts = 20 * (int)(R/20);
    double running_time = (end_time-start_time) / 1000.f;

    std::cout << "** Running time of "
              << num_inserts
              << "hash insert attempts: "
              << running_time << " seconds **\n ";
     
    all_hash_insert(H, R);
    check_hash_insert(H, R);

    if (R <= 100) {
        std::cout << "Final hash table\n";
        H->print_table();
    }

    std::cout << "Deleting hash table: \n";
    delete H;
    std::cout << "Done with delete\n";
    std::cout << "PASSED\n";

    return 0;
}

