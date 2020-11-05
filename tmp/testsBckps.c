#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define TEST_SIZE 10000000
#define ARRAY_INIT_SIZE 10

double tiL, tiA;

int testArray(void) {
    clock_t begin = clock();
    int size2 = ARRAY_INIT_SIZE;
    int *arr = malloc(size2 * sizeof(int));
    if (arr == 0){
        return 1;
    }

    time_t t = 0;
    srand((unsigned) time(&t));

    int *size = arr + ARRAY_INIT_SIZE;
    int *tmp = 0;
    int *it = arr;
    while (1) {
        *it = rand() % 10000;
        //printf("%d\n", *it);
        it++;
        if (it >= size) {
            if (size2 * 2 > TEST_SIZE)
                break;
            tmp = realloc(arr, size2 * 2 * sizeof(int));
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
    //printf("Array time for %d elements %f\n", TEST_SIZE, tiA);
    free(arr);
    return 0;
}

struct A {
    struct A *next, *prev;
    int val;
};

int addToList(struct A *head, int val){
    if (head->prev == 0){
        head->val = val;
        head->next = head;
        head->prev = head;
    } else {
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

int testLinkedList(void) {
    clock_t begin = clock();

    time_t t = 0;
    srand((unsigned) time(&t));
    struct A head = {0};
    int value = 0;
    int res = 0;
    for (int i=0; i<TEST_SIZE; i++){
        value = rand() % 10000;
        res = addToList(&head, value);
        if (res != 0){
            printf("Test of linked list error\n");
            return 1;
        }
    }

    clock_t end = clock();
    tiL = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Linked list time for %d elements %f\n", TEST_SIZE, tiL);
    listFree(&head);
    //listTraverse(&head);
    return 0;
};


int main(){
    //testArray();
    for (int i=0; i<5; i++){
        testLinkedList();
        testArray();
    }
    if (tiA != 0 && tiL != 0){
        printf("Ratio: %f\n", (double)(tiL / tiA));
    }
/*    int a = 1;
    int b = 1;
    int c = 2;
    if (a & b && (c = 3))
        printf("c = %d\n", c);
    printf("c = %d\n", c);
    printf("end\n");*/
    return 0;
}
