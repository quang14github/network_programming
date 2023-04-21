#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define TIME_OUT 15

int main(int argc, char *argv[])
{
    // Check enough arguments
    // Arguments contain run_file_name, port_number
    if (argc != 2)
    {
        printf("Wrong arguments.\n");
        exit(1);
    }

    // Create socket
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiver == -1)
    {
        perror("Create Socket Failed\n");
        exit(1);
    }

    // Set timeout for socket
    struct timeval tv;
    tv.tv_sec = TIME_OUT;
    tv.tv_usec = 0;
    if (setsockopt(receiver, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("Error");
        exit(1);
    }

    // Bind socket to ip address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if (bind(receiver, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Bind Failed\n");
        exit(1);
    }

    printf("Waiting for message ...\n");

    // Receive message from multiclient
    char buff[512];
    struct sockaddr_in clientAddr;
    int size = sizeof(clientAddr);

    // identify sender based on the port number
    bool clientPort[65536];
    memset(clientPort, 0, 65536);
    FILE *f[65536];
    char tmpFileName[10];

    int ret;
    while (1)
    {
        ret = recvfrom(receiver, buff, sizeof(buff), 0, (struct sockaddr *)&clientAddr, &size);
        if (ret <= 0)
            break;

        // Reset the timeout
        if (setsockopt(receiver, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            perror("Error");
            break;
        }

        if (!clientPort[clientAddr.sin_port])
        {
            sprintf(tmpFileName, "%d.txt", clientAddr.sin_port);
            f[clientAddr.sin_port] = fopen(tmpFileName, "a+");
            if (f[clientAddr.sin_port] == NULL)
            {
                printf("Open file failed\n");
                exit(1);
            }
            clientPort[clientAddr.sin_port] = true;
        }

        fwrite(buff, 1, ret, f[clientAddr.sin_port]);
        printf("Receive %d bytes message from: %s:%d\n", ret, inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
    }
    // Close all files have created
    for (int i = 0; i < 65536; ++i)
    {
        if (clientPort[i])
            fclose(f[i]);
    }

    // Close socket
    close(receiver);
    printf("Time out!\n");
    printf("Socket closed\n");
    return 0;
}