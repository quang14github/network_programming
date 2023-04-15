#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char *read_file(char *filename)
{
    FILE *file = fopen(filename, "r"); // open the file for reading
    if (file == NULL)
    { // check if the file was opened successfully
        printf("Failed to open file: %s\n", filename);
        return NULL; // return NULL if file could not be opened
    }

    fseek(file, 0, SEEK_END); // move file pointer to end of file
    long size = ftell(file);  // get the size of the file
    fseek(file, 0, SEEK_SET); // move file pointer back to beginning of file

    char *content = (char *)malloc(size + 1); // allocate memory for the file content
    if (content == NULL)
    { // check if memory allocation was successful
        printf("Failed to allocate memory for file content\n");
        fclose(file);
        return NULL; // return NULL if memory allocation failed
    }

    fread(content, 1, size, file); // read file content into buffer
    content[size] = '\0';          // add null terminator to end of buffer

    fclose(file);   // close the file
    return content; // return pointer to file content
}

int append_to_file(char *filename, char *buf, int buf_size)
{
    FILE *file = fopen(filename, "a+"); // open the file for appending or create it if it doesn't exist
    if (file == NULL)
    { // check if the file was opened successfully
        printf("Failed to open file: %s\n", filename);
        return -1; // return -1 if file could not be opened
    }

    int bytes_written = fwrite(buf, 1, buf_size, file); // write buffer to file
    if (bytes_written != buf_size)
    { // check if all bytes were written
        printf("Failed to write buffer to file: %s\n", filename);
        fclose(file);
        return -1; // return -1 if writing to file failed
    }

    fclose(file); // close the file
    return 0;     // return 0 on success
}

int main(int argc, char *argv[])
{
    // Check arguments
    if (argc != 4)
    {
        printf("Wrong arguments\n");
        return 1;
    }
    int port = atoi(argv[1]);
    char *hello_file = argv[2];
    char *record_file = argv[3];

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
    addr.sin_port = htons(port);

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

    char *msg = read_file(hello_file);
    send(client, msg, strlen(msg), 0);

    char buf[256];

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

        printf("%d bytes received\n", ret);
        int result = append_to_file(record_file, buf, ret);
        if (result == -1)
        {
            printf("Failed to append content to file\n");
        }
        else
        {
            printf("Content appended to file\n");
        }
    }

    close(client);
    close(listener);
}