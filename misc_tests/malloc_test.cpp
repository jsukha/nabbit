#include <iostream>
#include <cstdlib>


#include <cilk/cilk.h>


#include <example_util_gettime.h>
#include <concurrent_linked_list.h>


// Spawn n linked list nodes.
ListNode* create_linked_list(int n) {
    int k = 0;
    ListNode* temp;
    ListNode* head = new ListNode();

    while (k < n) {
        temp = new ListNode();
        temp->next = head;
        head = temp;
        k++;
    }

    return head;
}

void delete_list_nodes(ListNode* head) {
    ListNode* current = head;
    while (head!= NULL) {
        current = head;
        head = head->next;
        delete current;        
    }
}


void list_creation_test(int list_length, int reps) {
    for (int i = 0; i < reps; i++) {
        ListNode* head = create_linked_list(list_length);
        delete_list_nodes(head);
    }
}


int main(int argc, char *argv[])
{
    int list_length = 10;
    int R = 10000;
    if (argc >= 2) {
        R = atoi(argv[1]);
    }

  
    printf("Value of R: %d\n", R);
    printf("List length = %d\n", list_length);

    long start_time = example_get_time();
    for (int i = 0; i < 20; i++) {
        cilk_spawn list_creation_test(list_length, R);
    }
    cilk_sync;
    long end_time = example_get_time();
    printf("** Running time of %d reps: attempts: %f seconds total, avg = %f **\n ",
           R,
           (end_time-start_time) / 1000.f,
           (end_time-start_time) / (1000.f * R));
	 
  
    return 0;
}

