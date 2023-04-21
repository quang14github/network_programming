#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
char last_chars[10];
int count_string_in_buffer(const char *buffer, const char *string)
{
    int count = 0;
    size_t len = strlen(string);

    while ((buffer = strstr(buffer, string)))
    {
        count++;
        buffer += len;
    }

    return count;
}

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

    char buf[256];
    int count = 0;
    char *input_string = "0123456789";

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            printf("Connection closed.\n");
            break;
        }
        if (ret < sizeof(buf))
            buf[ret] = 0;
        printf("Received %d bytes: %s\n", ret, buf);
        // append last_chars of last received content to the start of buf
        char appended_buf[300];
        memcpy(appended_buf, last_chars, strlen(last_chars));
        memcpy(appended_buf + strlen(last_chars), buf, ret);
        count += count_string_in_buffer(appended_buf, input_string);
        // save last_chars of buf
        int append_len = strlen(input_string) - 1 > ret ? ret : strlen(input_string) - 1;
        memcpy(last_chars, buf + ret - append_len, append_len);
        last_chars[append_len] = 0;
    }
    printf("String %s appears %d times\n", input_string, count);
    close(client);
    close(listener);
}