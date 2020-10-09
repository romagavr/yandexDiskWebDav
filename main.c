#include"src/client.h"

int main(int argc, char *argv[]){

    synchronize("/test");
    // без слэша в начале  - 400
    // если нет такой директории - 404
    // в остальных случаях - 207
    //if (getFolderStruct("/Others/", net) < 0) {
    //    exit(EXIT_FAILURE);
    //}; 
/*
    //getToken(ssl);
    int res = uploadFile("../res/2.png", "/", ssl);
    if (res == -1){
        fprintf(stderr, "File upload error.\n");
        exit(EXIT_FAILURE);
    }
    printf("Closing socket...\n");

    printf("Finished.\n");
    */
    //free(xml);
    return 0;
}