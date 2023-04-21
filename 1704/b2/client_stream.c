#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

int main()
{
    // Declare socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Declare server's address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);
    // Connect to server
    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        printf("Connection failed");
        return 1;
    }

    printf("Start sending file content\n");
    // Declare input file name
    char *input_file = "input.txt";
    // send file content to server, each time 20 bytes
    FILE *f = fopen(input_file, "r");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    // Set the limit to buffer size to test the case when string "0123456789" is split into 2 parts in 2 packets
    char buf[20];
    while (!feof(f))
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        if (send(client, buf, ret, 0) == -1)
        {
            printf("Send Failed\n");
            exit(1);
        }
        // Delay 0.5s to prevent the server from receiving 2 packets at the same time
        usleep(500000);
    }
    printf("Sent\n");
    fclose(f);
    close(client);
    return 0;
}