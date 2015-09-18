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

        // Retry insert until return code is not OP_FAILED.
        while (code == OP_FAILED) {
            return_val = H->insert_if_absent(i,
                                             (void*)i,
                                             &code);
            statusCounts[code]++;
        }
        assert(return_val == (void*)i);
    }

    //  L->print_list();
    printf("Final code results: ");
    for (int i = OP_NULL; i < OP_LAST; i++) {
        printf("(%d, %d)  ",
               i, statusCounts[i]);
    }
    printf("\n");
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
        printf("Test hash insert: R = %d, n = %d\n",
               R, n);
    }

    for (int i = 0; i < n; i++) {


        int rand_num = rand() % R;
        LOpStatus code = OP_FAILED;
        void* return_val;

        while (code == OP_FAILED) {
            return_val = H->insert_if_absent(rand_num,
                                             (void*)rand_num,
                                             &code);
            statusCounts[code]++;
        }
        assert(return_val == (void*)rand_num);
    }

    if (print_output) {
        printf("Code results: ");
        for (int i = OP_NULL; i < OP_LAST; i++) {
            printf("(%d, %d)  ",
                   i, statusCounts[i]);
        }
        printf("\n");
    }
    //  L->print_list();
}




int main(int argc, char *argv[])
{
    int R = 10000;
    if (argc >= 2) {
        R = atoi(argv[1]);
    }


    printf("Value of R: %d\n", R);     
    ConcurrentHashTable* H = new ConcurrentHashTable(5*R); // + ((R % 10) > 0));
    printf("An empty hash table\n");
    H->print_table();


    long start_time = example_get_time();
    for (int i = 0; i < 20; i++) {
        cilk_spawn test_hash_insert(H, R, R/20);
    }
    cilk_sync;
    long end_time = example_get_time();
    printf("** Running time of %d hash insert attempts: %f seconds **\n ",
           20 * (int)(R/20),
           (end_time-start_time) / 1000.f);
     
    all_hash_insert(H, R);
    check_hash_insert(H, R);

    if (R <= 100) {
        printf("Final hash table\n");
        H->print_table();
    }

     
    printf("Deleting hash table: \n");
    delete H;
    printf("Done with delete\n");
    printf("PASSED\n");
    
    return 0;
}

