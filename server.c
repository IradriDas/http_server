#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>

char *read_file(char *file_path, long *out_len)
{
    // open the file
    FILE *fp = fopen(file_path, "rb"); // rb: read binary mode
    if (!fp)
    {
        return NULL;
    }

    // calculate size
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *content = (char *)malloc(size); // allocate a buffer big enough to hold the data
    if (!content)
    {
        fclose(fp);
        return NULL;
    }

    // start reading the bytes
    fread(content, 1, size, fp);
    /*
        content: where to put the data
        1: size of each data
        size: how many element to read
        fp: where to read from
    */

    // close the file
    fclose(fp);

    *out_len = size;

    return content;
}

const char *get_content_type(char *path)
{
    if (strstr(path, ".html"))
    {
        return "text/html";
    }

    if (strstr(path, ".css"))
    {
        return "text/css";
    }

    if (strstr(path, ".png"))
    {
        return "image/png";
    }

    if (strstr(path, ".jpeg") || strstr(path, ".jpg"))
    {
        return "image/jpeg";
    }

    return "application/octet-stream"; // fallback for unknown types
}

int main()
{

    struct addrinfo requirements, *res;

    memset(&requirements, 0, sizeof requirements); // zero it out — same reasoning as before, avoid garbage
    requirements.ai_family = AF_UNSPEC;            // "I don't care — give me IPv4 or IPv6, whichever works"
    requirements.ai_socktype = SOCK_STREAM;        // TCP : same meaning as in socket()
    requirements.ai_flags = AI_PASSIVE;            // "I'm a server — auto-fill the local IP for me"

    getaddrinfo(NULL, "8080", &requirements, &res);

    int soc_id = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // no need to setup manually
    if (soc_id < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // --- lets the client to use the last used connection ---
    int opt = 1;
    if (setsockopt(soc_id, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    // -------------------------------------------------------------

    if (bind(soc_id, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if (listen(soc_id, 10) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("server listening on port 8080...\n");

    while (1)
    {
        struct sockaddr_storage client_addr; // client is gonna fill this block
        socklen_t client_addr_len = sizeof(client_addr);

        int client_id = accept(soc_id, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_id < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("client connected..\n");

        // request received
        char buffer[4096];
        recv(client_id, buffer, sizeof(buffer) - 1, 0);

        // separate method and path
        char method[8], path[256];
        sscanf(buffer, "%7s %255s", method, path);

        // mapping path -> file on disk
        char filepath[300];
        if (strcmp(path, "/") == 0)
        {
            snprintf(filepath, sizeof(filepath), "view/index.html");
        }
        else
        {
            snprintf(filepath, sizeof(filepath), "view%s", path);
        }

        // read the file and get the length
        long file_len;
        char *file_content = read_file(filepath, &file_len);

        if (file_content)
        {
            char header[256];
            int header_len = snprintf(header, sizeof(header),
                                      "HTTP/1.1 200 OK\r\n"
                                      "Content-Type: %s\r\n"
                                      "Content-Length: %ld\r\n"
                                      "\r\n",
                                      get_content_type(filepath), file_len);

            send(client_id, header, header_len, 0);
            send(client_id, file_content, file_len, 0);
            free(file_content);
        }
        else
        {
            const char *not_found =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 9\r\n"
                "\r\n"
                "Not Found";
            send(client_id, not_found, strlen(not_found), 0);
        }

        close(client_id); // done talking to this client
    }

    return 0;
}