#include <cassert>
#include <cstdlib>
#include <vector>

#include <nabbit.h>
#include "hash_tbl.h"

#define LAZY (1<<15)

struct State;
struct State {
    struct State *stack, *next;
    unsigned int type;
    int deps;
    int path;
    const char name[8];
};

template <class DynNodeType>
class DynNode: public DynNodeType {
    DynHashTable* H;

    protected:
        void Init();
        void Compute();
        void Generate();

        struct State *s;

    public:
        //: DynNodeType((node_key_t)s, H), children(NULL), s(s) {};
        DynNode(struct State *s, DynHashTable* H)
                : DynNodeType((node_key_t)s, H), H(H), s(s) {};

};

template <class DynNodeType>
void DynNode<DynNodeType>::Init() {
    printf("Node %s: Counting eggs.\n", s->name);
    if(s->type & LAZY) {
        printf("%s: Nah, I think I'll do it later.\n", s->name);
        return;
    }

    for(struct State *c = s->stack; c != NULL; c = c->next) {
        if(H->insert_task_if_absent((node_key_t)c)) {
            printf("Error inserting child task!");
            exit(1);
        }
        this->add_dep((node_key_t)c);
    }
}

template <class DynNodeType>
void DynNode<DynNodeType>::Compute() {
    s->deps = 1;
    s->path = 1;
    if(s->type & LAZY) {
        printf("%s: I wasn't ready.\n", s->name);
        return;
    }
    for(struct State *c = s->stack; c != NULL; c = c->next) {
        s->deps += c->deps;
        s->path = s->path > c->path+1 ? s->path : c->path+1;
    }
}

template <class DynNodeType>
void DynNode<DynNodeType>::Generate() {
    if(s->type & LAZY) {
        printf("%s: Reporting for duty!\n", s->name);
        s->type = s->type & (~LAZY); // Get a job!

        /* TODO: add fake successor node to do actual computation?
        if(H->insert_task_if_absent((node_key_t)s)) {
            printf("Error inserting child task!");
            exit(1);
        }
        this->generate_task((node_key_t)s);*/

        for(struct State *c = s->stack; c != NULL; c = c->next) {
            if(H->insert_task_if_absent((node_key_t)c)) {
                printf("Error inserting child task!");
                exit(1);
            }
            this->generate_task((node_key_t)c);
        }
    }
}

template <class DynNodeType>
DynamicNode *mk_node(node_key_t key, void *H) {
    DynNode<DynNodeType>* ret =
        new DynNode<DynNodeType>((struct State *)key, (DynHashTable *)H);
    return (DynamicNode *)ret;
}

typedef DynNode<DynamicSerialNode> NodeT;

int main(int argc, char *argv[]) {
    struct State c3 = {
        NULL, NULL, 0, 0, 0, "c3"
    };
    struct State c2 = {
        &c3, NULL, 0, 0, 0, "c2"
    };
    struct State c1 = {
        &c3, &c2, 0, 0, 0, "c1"
    };
    struct State root = {
        &c1, NULL, 0, 0, 0, "root"
    };

    DynHashTable *H = new DynHashTable(4, mk_node<DynamicSerialNode>);
    NodeT *node = new NodeT(&root, H);
    // H->insert_if_absent(node, 

    /*cilk_spawn node->compute_and_notify();
    typedef DynamicArray<node_key_t> DTGSKeyArray;*/

    // start computation:
    node->init_root_and_compute((node_key_t)&root); // for ea. root, k
}

