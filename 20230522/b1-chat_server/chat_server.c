#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <ctype.h>

void standardizeText(char *text)
{
    // Remove redundant whitespace at the start and end of the sentence
    char *start = text;
    while (isspace(*start))
    {
        start++;
    }

    char *end = text + strlen(text) - 1;
    while (end > start && isspace(*end))
    {
        end--;
    }
    *(end + 1) = '\0';

    // Remove redundant spaces between words in the sentence
    char *src = start;
    char *dst = start;
    int prevSpace = 0; // Indicates if the previous character was a space

    while (*src != '\0')
    {
        if (isspace(*src))
        {
            if (!prevSpace)
            {
                *dst++ = ' '; // Add a single space
                prevSpace = 1;
            }
        }
        else
        {
            *dst++ = *src;
            prevSpace = 0;
        }

        src++;
    }
    *dst = '\0'; // Null-terminate the resulting string

    // Standardize the text by capitalizing the first letter of each word
    char *ptr = start;
    int capitalize = 1; // Indicates whether the next character should be capitalized

    while (*ptr != '\0')
    {
        if (isspace(*ptr))
        {
            capitalize = 1; // Capitalize the next character
        }
        else
        {
            if (capitalize)
            {
                *ptr = toupper(*ptr); // Capitalize the current character
                capitalize = 0;       // Reset capitalize flag
            }
            else
            {
                *ptr = tolower(*ptr); // Convert the current character to lowercase
            }
        }
        ptr++;
    }
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

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

    fd_set fdread, fdtest;

    // Xóa tất cả socket trong tập fdread
    FD_ZERO(&fdread);

    // Thêm socket listener vào tập fdread
    FD_SET(listener, &fdread);

    char buf[256];
    struct timeval tv;

    int num_users = 0; // So luong client da dang nhap

    while (1)
    {
        fdtest = fdread;

        // Thiet lap thoi gian cho
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // Chờ đến khi sự kiện xảy ra
        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        if (ret == 0)
        {
            printf("Timed out!!!\n");
            continue;
        }

        for (int i = listener; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &fdtest))
            {
                if (i == listener)
                {
                    int client = accept(listener, NULL, NULL);
                    if (client < FD_SETSIZE)
                    {
                        FD_SET(client, &fdread);
                        printf("New client connected: %d\n", client);
                        num_users++;
                        char sendbuf[256];
                        sprintf(sendbuf, "Xin chao. Hien co %d client dang ket noi.\n", num_users);
                        send(client, sendbuf, strlen(sendbuf), 0);
                    }
                    else
                    {
                        // Dang co qua nhieu ket noi
                        close(client);
                    }
                }
                else
                {
                    int ret = recv(i, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        FD_CLR(i, &fdread);
                        num_users--;
                        close(i);
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("Received from %d: %s\n", i, buf);

                        int client = i;
                        standardizeText(buf);
                        send(client, buf, strlen(buf), 0);
                    }
                }
            }
        }
    }

    close(listener);

    return 0;
}