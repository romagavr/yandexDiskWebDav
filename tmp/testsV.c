#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define TEST_SIZE 200000000
#define ARRAY_INIT_SIZE 100

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
    double ti = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Array time for %d elements %f\n", TEST_SIZE, ti);
    free(arr);
    return 0;
}
/*
int testLinkedList(){

};
*/

int main(){
    testArray();
    int a = 1;
    int b = 1;
    int c = 2;
    if (a & b && (c = 3))
        printf("c = %d\n", c);
    printf("c = %d\n", c);
    printf("end\n");
    return 0;
}