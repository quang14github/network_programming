#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

#define TIME_OUT 15

int main(int argc, char *argv[])
{
    // Check enough arguments
    // Arguments contains ./run_file_name + ip address + receiver port + sender port
    if (argc != 4)
    {
        printf("Missing arguments\n");
        exit(1);
    }

    // Declare sender socket
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sender == -1)
    {
        perror("Create Socket Failed\n");
        exit(1);
    }

    // Declare ip address
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(argv[1]);
    saddr.sin_port = htons(atoi(argv[2]));

    // Declare receiver socket
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiver == -1)
    {
        perror("Create Socket Failed\n");
        exit(1);
    }

    // Bind socket to ip address
    struct sockaddr_in raddr;
    raddr.sin_family = AF_INET;
    raddr.sin_addr.s_addr = htonl(INADDR_ANY);
    raddr.sin_port = htons(atoi(argv[3]));

    bind(receiver, (struct sockaddr *)&raddr, sizeof(raddr));
    printf("Waiting for message ...\n");

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(STDIN_FILENO, &fdread);
    FD_SET(receiver, &fdread);

    char buf[256];

    while (1)
    {
        fdtest = fdread;
        int ret = select(receiver + 1, &fdtest, NULL, NULL, NULL);
        if (ret < 0)
        {
            perror("select() failed");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fdtest))
        {
            fgets(buf, sizeof(buf), stdin);
            sendto(sender, buf, strlen(buf), 0,
                   (struct sockaddr *)&saddr, sizeof(saddr));
        }

        if (FD_ISSET(receiver, &fdtest))
        {
            ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);
            if (ret < sizeof(buf))
                buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }
    return 0;
}