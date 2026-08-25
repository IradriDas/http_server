#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
int main(){

    int soc_id = socket(AF_INET, SOCK_STREAM, 0);
    if (soc_id < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    



    while (1)
    {
        // some kind of request must be received here
        printf("waiting for request ... \n");
        sleep(1);
    }

    return 0;
    
}