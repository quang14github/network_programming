#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

void signalHandler(int signal)
{
    int status;
    int pid = wait(&status);
    if (status == EXIT_SUCCESS)
        printf("Child process %d terminated\n", pid);
}

int main(int argc, char *argv[])
{

    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1)
    {
        perror("Create socket failed: ");
        exit(1);
    }

    // Create struct sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ;
    addr.sin_port = htons(9000);

    // Bind socket to sockaddr
    if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Binding Failed\n");
        exit(1);
    }

    // Listen
    if (listen(serverSocket, 5) == -1)
    {
        printf("Listening Failed\n");
        exit(1);
    }
    printf("Waiting for client connecting ...\n");

    // For logging address of connection
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);

    // Listening
    signal(SIGCHLD, signalHandler);
    while (1)
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        DIR *dir = opendir(".");
        struct dirent *entry;
        char buff[256];
        char *fileName[256];
        int countFile = 0;
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG)
            {
                countFile++;
                fileName[countFile] = (char *)malloc(strlen(entry->d_name) + 1);
                strcpy(fileName[countFile], entry->d_name);
            }
        }
        if (countFile == 0)
        {
            sprintf(buff, "ERROR No files to download\r\n");
            close(clientSocket);
            exit(EXIT_SUCCESS);
        }
        else
        {

            sprintf(buff, "OK %d\r\n", countFile);
            send(clientSocket, buff, strlen(buff), 0);
            for (int i = 1; i <= countFile; i++)
            {
                if (i == countFile)
                    sprintf(buff, "%s\r\n\r\n", fileName[i]);
                else
                    sprintf(buff, "%s\r\n", fileName[i]);
                send(clientSocket, buff, strlen(buff), 0);
            }
            if (fork() == 0)
            {
                // Child process
                close(serverSocket);
                char cmd[32], format[32], tmp[32];

                while (1)
                {
                    int ret = recv(clientSocket, buff, sizeof(buff), 0);
                    if (ret <= 0)
                        break;
                    buff[ret] = 0;
                    if (buff[ret - 1] == '\n')
                        buff[ret - 1] = 0;
                    printf("Receive: %s\n", buff);
                    FILE *f = fopen(buff, "rb");

                    if (f == NULL)
                    {
                        // File khong mo duoc. Gui thong bao loi.
                        char *msg = "ERROR Cannot open file.\n";
                        send(clientSocket, msg, strlen(msg), 0);
                    }
                    else
                    {
                        // Gui noi dung file cho client
                        fseek(f, 0, SEEK_END);
                        long file_size = ftell(f);
                        fseek(f, 0, SEEK_SET);

                        sprintf(buff, "OK %ld\r\n", file_size);
                        send(clientSocket, buff, strlen(buff), 0);

                        while (1)
                        {
                            ret = fread(buff, 1, sizeof(buff), f);
                            if (ret <= 0)
                                break;
                            send(clientSocket, buff, ret, 0);
                        }

                        fclose(f);
                        break;
                    }
                }

                close(clientSocket);
                exit(EXIT_SUCCESS);
            }
            else
            {
                close(clientSocket);
            }
        }
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    printf("Main process terminated\n");
    killpg(0, SIGKILL);
    return 0;
}