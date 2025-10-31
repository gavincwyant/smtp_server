// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2525
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Connect to server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    recv(sock, buffer, BUFFER_SIZE - 1, 0);
    //printf("Server: %s", buffer);
    printf("%s\n", buffer);
    if (strlen(buffer) < 40){
        //no email for this client
        //printf("Server: %s", buffer);

        // 3. SMTP commands
        char *commands[] = {
            "HELO localhost\r\n",
            "MAIL FROM:<sender@example.com>\r\n",
            "RCPT TO:<recipient@example.com>\r\n",
            "DATA\r\n",
            "Subject: Test Email\r\n\r\nHello, this is a test email to Client B!\r\n.\r\n",
            "QUIT\r\n"
        };

        for (int i = 0; i < sizeof(commands); i++) {
            //printf("C: %s", commands[i]);
            send(sock, commands[i], strlen(commands[i]), 0);
            memset(buffer, 0, BUFFER_SIZE);
            recv(sock, buffer, BUFFER_SIZE - 1, 0);
            printf("S: %s", buffer);
        }
        printf("quit sent\n");
        close(sock);
     }
     else{
        printf("Server: %s", buffer);

     }
     return 0;
}
