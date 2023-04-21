#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    // Declare socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Declare server's address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Attach address to port
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // Receive data from client
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);

    buf[ret] = 0;

    int pos = 0;

    // Spilt data into computer name and drive info
    char computer_name[64];
    strcpy(computer_name, buf);
    pos += strlen(computer_name) + 1;

    printf("Computer name: %s\n", computer_name);

    int num_drives = (ret - pos) / 3;
    printf("Number of drives: %d\n", num_drives);
    for (int i = 0; i < num_drives; i++)
    {
        char drive_letter;
        short int drive_size;

        drive_letter = buf[pos];
        pos++;

        memcpy(&drive_size, buf + pos, sizeof(drive_size));
        pos += sizeof(drive_size);

        printf("%c: %hd\n", drive_letter, drive_size);
    }
    close(client);
    close(listener);
}