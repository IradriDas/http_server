#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>

int main()
{

    struct addrinfo requirements, *res;

    memset(&requirements, 0, sizeof requirements); // zero it out — same reasoning as before, avoid garbage
    requirements.ai_family = AF_UNSPEC;            // "I don't care — give me IPv4 or IPv6, whichever works"
    requirements.ai_socktype = SOCK_STREAM;        // TCP : same meaning as in socket()
    requirements.ai_flags = AI_PASSIVE;            // "I'm a server — auto-fill the local IP for me"

    getaddrinfo(NULL, "8080", &requirements, &res);

    int soc_id = socket(res->ai_family, res->ai_socktype, res->ai_protocol);  // no need to setup manually
    if (soc_id < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (bind(soc_id, res->ai_addr, res->ai_addrlen) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    while (1)
    {
        // some kind of request must be received here
        printf("waiting for request ... \n");
        sleep(1);
    }

    return 0;
}