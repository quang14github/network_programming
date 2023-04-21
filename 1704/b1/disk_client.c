#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

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

    char buf[256];
    char computer_name[64];
    printf("Enter computer name: ");
    scanf("%s", computer_name);

    // Delete /n character in buffer
    getchar();

    strcpy(buf, computer_name);
    int pos = strlen(computer_name);
    buf[pos] = 0;
    pos++;

    int n;
    printf("Enter disk count: ");
    scanf("%d", &n);

    // Delete /n character in buffer
    getchar();

    char disk_letter;
    short int disk_size;

    for (int i = 0; i < n; i++)
    {
        printf("Enter disk's name: ");
        scanf("%c", &disk_letter);

        printf("Enter disk's size: ");
        scanf("%hd", &disk_size);

        // Delete /n character in buffer
        getchar();

        buf[pos] = disk_letter;
        pos++;
        memcpy(buf + pos, &disk_size, sizeof(disk_size));
        pos += sizeof(disk_size);
    }

    printf("Buffer size: %d\n", pos);
    send(client, buf, pos, 0);
    close(client);
    return 0;
}