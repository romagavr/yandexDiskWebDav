#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define TEST_SIZE 10000000
#define ARRAY_INIT_SIZE 10

double tiL, tiA;

struct A {
    struct A *next, *prev;
    int val;
};

int testArray(int testSize) {
    clock_t begin = clock();
    int size2 = ARRAY_INIT_SIZE;
    int *arr = malloc(size2 * sizeof(struct A));
    if (arr == 0){
        return 1;
    }

    time_t t = 0;
    srand((unsigned) time(&t));

    int *size = arr + ARRAY_INIT_SIZE;
    int *tmp = 0;
    int *it = arr;
    int val = rand() % 10000;
    while (1) {
        *it = 0;
        *(it+16) = val;
        //printf("%d\n", *it);
        it++;
        if (it >= size) {
            if (size2 * 2 > testSize)
                break;
            tmp = realloc(arr, size2 * 2 * sizeof(struct A));
            if (tmp == 0){
                printf("Realloc error\n");
                return 1;
            }
            //printf("reallocated\n");
            arr = tmp;
            it  = arr + size2;
            size2 *= 2;
            size = arr + size2;
        }
    }
    clock_t end = clock();
    tiA = (double)(end - begin) / CLOCKS_PER_SEC;
    //printf("Array time for %d elements %f\n", testSize, tiA);
    free(arr);
    return 0;
}


int addToList(struct A *head, int val){
    if (head->prev == 0){
        head->val = val;
        head->next = head;
        head->prev = head;
    } else {
        struct A *tmp = malloc(sizeof(struct A));
        /*if (tmp == 0){
            perror("malloc failed in addToList");
            return 1;
        }*/
        head->prev->next = tmp;
        tmp->prev = head->prev;
        head->prev = tmp;
        tmp->next = head;
        tmp->val = val;
    }
    return 0;
};

void listTraverse(struct A *head){
    struct A *tmp = head;
    do {
        printf("%d\n", tmp->val);
        tmp = tmp->next;
    } while (tmp != head);
}

void listFree(struct A *head){
    struct A *tmp = head->next;
    struct A *tmp2 = 0;
    while(tmp != head) {
        tmp2 = tmp->next;
        free(tmp);
        tmp = tmp2;
    };
    head->next = head->prev = 0;
}

int testLinkedList(int testSize) {
    clock_t begin = clock();

    time_t t = 0;
    srand((unsigned) time(&t));
    struct A *head = malloc(sizeof *head);
    head->val = 0;
    head->next = head;
    head->prev = head;
    int val = 0;
    val = rand() % 10000;
    for (int i=1; i<testSize; i++){
        struct A *tmp = malloc(sizeof(struct A));
        if (tmp == 0){
            perror("malloc failed in addToList");
            return 1;
        }
        head->prev->next = tmp;
        tmp->prev = head->prev;
        head->prev = tmp;
        tmp->next = head;
        tmp->val = val;
    }

    clock_t end = clock();
    tiL = (double)(end - begin) / CLOCKS_PER_SEC;
    //printf("Linked list time for %d elements %f\n", testSize, tiL);
    listFree(head);
    free(head);
    //listTraverse(&head);
    return 0;
};


int main(void){
    //testArray();
    int count = 10;
    double sumL, sumA;
    int begin = 1200;
    int end = 10000;
    for (int j = begin; j < end; j += 100) {
        sumL = sumA = 0;
        printf("-------Test for %d elements-------\n", j);
        for (int i=0; i<count; i++){
            testLinkedList(j);
            testArray(j);
            if (i != 0) {
                sumL += tiL;
                sumA += tiA;
            }
        }
        tiL = (double)sumL/count;
        tiA = (double)sumA/count;

        if (sumL != 0)
            printf("Avarage value for linked list (%d elements) = %f (%d iters)\n", j, tiL, count); 
        if (sumA != 0)
            printf("Avarage value for array (%d elements) = %f (%d iters)\n", j, tiA, count); 
        if (tiA != 0 && tiL != 0){
            printf("Avarage ratio: %s faster in %f\n", (tiL > tiA) ? "array" : "linked list",  (tiL > tiA) ? (double)(tiL / tiA) : (double)(tiA / tiL));
        }
    }
    return 0;
}