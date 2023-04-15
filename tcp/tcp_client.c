#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    // Check arguments
    if (argc != 3)
    {
        printf("Wrong arguments.\n");
        return 1;
    }
    int port = atoi(argv[2]);
    // Declare socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Declare server's address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(port);
    // Connect to server
    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        printf("Connection failed");
        return 1;
    }

    char buf[256];
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        printf("Connection closed\n");
        return 1;
    }

    if (ret < sizeof(buf))
        buf[ret] = 0;
    printf("%d bytes received: %s\n", ret, buf);

    while (1)
    {
        printf("Enter string:");
        fgets(buf, sizeof(buf), stdin);
        if (strncmp(buf, "exit", 4) == 0)
            break;
        send(client, buf, strlen(buf), 0);
    }

    close(client);
    return 0;
}