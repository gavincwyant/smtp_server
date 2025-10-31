// server.c (multi-session version)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define PORT 2525
#define BUFFER_SIZE 1024

char* email_data;
char* mail_from;

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    //char email_data[8192] = "";
    int in_data_mode = 0;

    send(client_fd, "220 Simple SMTP Server Ready\r\n", 31, 0);

    if (strlen(email_data) == 0){
        printf("no email data for you at this time!\n");
        //init email data
    }else{
        printf("we have some email data, send it to the client\n");
        //just send the data to the new client (super secure)
        //also, send to clientB who is the sender of the message.
        send(client_fd, mail_from, strlen(mail_from), 0);
        send(client_fd, email_data, strlen(email_data), 0);
        //we may want to check if the current user wants to send anything
        //but for now just return
        return;
    }

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        printf("Client: %s", buffer);

        if (in_data_mode) {
            if (strcmp(buffer, ".\r\n") == 0) {
                in_data_mode = 0;
                send(client_fd, "250 Message accepted\r\n", 22, 0);
                printf("\n=== Email Received ===\n%s\n=====================\n", email_data);
                //memset(email_data, 0, sizeof(email_data));
            } else {
                printf("adding to email_data\n");
                strcpy(email_data, buffer);
                int fd = open("emails.txt", O_WRONLY | O_CREAT);
                if (fd == -1){
                    printf("error opening emails.txt\n");
                }
                if (write(fd, email_data, strlen(email_data)) == -1){
                    printf("error writing to emails.txt\n");
                }
            }
            continue;
        }

        if (strncmp(buffer, "HELO", 4) == 0) {
            send(client_fd, "250 Hello\r\n", 11, 0);
        } else if (strncmp(buffer, "MAIL FROM:", 10) == 0) {
            strcpy(mail_from, buffer);
            send(client_fd, "250 OK\r\n", 8, 0);
        } else if (strncmp(buffer, "RCPT TO:", 8) == 0) {
            send(client_fd, "250 OK\r\n", 8, 0);
        } else if (strncmp(buffer, "DATA", 4) == 0) {
            printf("data encountered, in_data_mode set to 1\n");
            send(client_fd, "354 End data with <CR><LF>.<CR><LF>\r\n", 37, 0);
            in_data_mode = 1;
        } else if (strncmp(buffer, "QUIT", 4) == 0) {
            send(client_fd, "221 Bye\r\n", 9, 0);
            break;
        } else {
            send(client_fd, "500 Unknown command\r\n", 21, 0);
        }
    }

    close(client_fd);
    printf("Client disconnected.\n");
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Allow reusing the port
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("SMTP Server running on port %d...\n", PORT);

    email_data = mmap(NULL, BUFFER_SIZE,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS,
                  -1, 0);

    mail_from = mmap(NULL, BUFFER_SIZE,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS,
                  -1, 0);

    memset(email_data, 0, BUFFER_SIZE);
    memset(mail_from, 0, BUFFER_SIZE);

    // Keep accepting clients forever
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        pid_t pid = fork();
        if (pid == 0){
            printf("New client connected.\n");
            handle_client(client_fd);
        }else{
            //go back to accept
        }
    }

    close(server_fd);
    return 0;
}
