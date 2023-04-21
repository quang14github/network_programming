#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define TIME_OUT 15

int main(int argc, char *argv[])
{
    // Check enough arguments
    // Arguments contain run_file_name, port_number, write_file_name
    if (argc != 3)
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

    // Receive message from sender
    char buff[256];
    struct sockaddr_in sender_addr;
    int sender_addr_len = sizeof(sender_addr);

    FILE *f = fopen(argv[2], "a+");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    int ret;
    while (1)
    {
        ret = recvfrom(receiver, buff, sizeof(buff), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        if (ret <= 0)
            break;

        // Reset the timeout
        if (setsockopt(receiver, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            perror("Error");
            break;
        }

        fwrite(buff, 1, ret, f);
        printf("Receive %d bytes message from: %s:%d\n", ret, inet_ntoa(sender_addr.sin_addr), sender_addr.sin_port);
    }
    fclose(f);

    // Close socket
    close(receiver);
    printf("Time out!\n");
    printf("Socket closed\n");
    return 0;
}