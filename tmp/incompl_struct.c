#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

struct A {
    char name[20];
};

struct B {
    int len;
    struct A **arr;
};

const char *const testArr[] = {
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j"
};

int main(void){
    /*struct B *b = malloc(sizeof(struct B));
    b->len = 10;
    b->arr = malloc(sizeof(struct A *) * b->len);
    int i = 0;
    for (struct A **tmp = b->arr; &(*tmp) != &(*b->arr) + b->len; &(*tmp++)){
        *tmp = malloc(sizeof(struct A));
        strcpy((*tmp)->name, testArr[i]);
        i++;
    }
    for (int j = 0; j< 10; j++){
        printf("%s\n", (b->arr[j])->name);
    }*/
    printf("%d\n", sizeof(float));
    int x = 10000;
    int y = 10000;
    long k = 1;
    clock_t begin = clock();

    long **multArr = malloc(sizeof(long *) * x);
    for (long **tmp = multArr; &(*tmp) != &(*multArr) + x; &(*tmp++)){
        *tmp = malloc(sizeof(long) * y);
        for (long *tmp2 = &(**tmp); tmp2 != &(**tmp) + y; tmp2++)
            *tmp2 = k++;
    }
    clock_t end = clock();
    printf("Time = %f\n", (double)(end - begin)/CLOCKS_PER_SEC);
    
    /*
    for (int **tmp = multArr; &(*tmp) != &(*multArr) + x; &(*tmp++)){
        for (int *tmp2 = &(**tmp); tmp2 != &(**tmp) + y; tmp2++)
            printf("%d ", *tmp2);
        printf("\n");
    }
    printf("Alt\n\n");
    */
   /*
    k = 1;
    long **ma =  malloc(sizeof(long *) * x);
    for (int i = 0; i < x; i++) {
        ma[i] = malloc(sizeof(long) * y);
        for (int j = 0; j < y; j++)
            ma[i][j] = k++;
    }
    clock_t end = clock();
    printf("Time = %f\n", (double)(end - begin)/CLOCKS_PER_SEC);
    */
    /*
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++)
            printf("%d ", ma[i][j]);
        printf("\n");
    }
    */
    return 0;
}