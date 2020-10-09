#include"src/client.h"
    
static void printQueue(Queue *q) {
    //printf("%d\n", q->size);
    printf("Queue size: %d\n", q->size);
    for (int i=q->front; i != ((q->front + q->size) % q->capasity); i = ((i+1) % q->capasity)) {
        //printf("%d\n", i);
        //printf("%d\n", ((q->front + q->size) % q->capasity) - 1);
        //printf("%d\n", ((i+1) % q->capasity));
        //break;
        printf("Front: %d; Rear: %d; Cur: %d; Name: %s;\n", q->front, q->rear, i, q->data[i].name);
    }
}

int main(int argc, char *argv[]){

    Queue *q = initQueue();
    QNode *n = malloc(sizeof *n);
    QNode *n2;
    int countAdd = 10;
    int countRem = 5;

    for (int i=0; i<countAdd; i++){
        sprintf(n->href, "%s", "324234234");
        sprintf(n->name, "%d", i);
        addToQueue(q, n);
    }
    printQueue(q);
    printf("---------------------------- \n");
    for (int i=0; i<countRem; i++){
        getFromQueue(q);
    }
    printQueue(q);
    printf("---------------------------- \n");

    for (int i=0; i<countAdd; i++){
        sprintf(n->href, "%s", "324234234");
        sprintf(n->name, "%d", i);
        addToQueue(q, n);
    }
    printQueue(q);
    printf("---------------------------- \n");

    return 0;
}