#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

#define BUFFER_SIZE 1024

void handle_connection(int new_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("Erro ao receber dados");
        close(new_socket);
        return;
    }

    if (strstr(buffer, "GET") != NULL && strstr(buffer, ".html") != NULL) {
        FILE *html_file = fopen("index.html", "r");
        if (html_file == NULL) {
            perror("Erro ao abrir arquivo HTML");
            close(new_socket);
            return;
        }

        char response_header[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        send(new_socket, response_header, strlen(response_header), 0);

        while (!feof(html_file)) {
            bytes_received = fread(buffer, 1, BUFFER_SIZE, html_file);
            if (bytes_received < 0) {
                perror("Erro ao ler arquivo HTML");
                fclose(html_file);
                close(new_socket);
                return;
            }
            send(new_socket, buffer, bytes_received, 0);
        }

        fclose(html_file);
    } else {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
        send(new_socket, response, strlen(response), 0);
    }
    close(new_socket);
}

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main() {

    int fd_server = socket(PF_INET, SOCK_STREAM, 0);
    if (fd_server < 0)
        return 1;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY; // Escuta em todas as interfaces
    server_address.sin_port = htons(8080);

    if (bind(fd_server, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Erro ao associar o soquete ao endereço");
        close(fd_server);
        return 1;
    }

    if (listen(fd_server, SOMAXCONN) < 0) {
        perror("Erro ao escutar por conexões");
        close(fd_server);
        return 1;
    }

    struct pollfd fds[MAX_CLIENTS];
    fds[0].fd = fd_server;
    fds[0].events = POLLIN;
    int num_fds = 1;

    while (1) {
        int num_events = poll(fds, num_fds, -1);
        if (num_events < 0) {
            perror("Erro ao chamar poll");
            close(fd_server);
            return 1;
        }

        printf("num_fds: %d\n", num_fds);
        for (int i = 0; i < num_fds; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == fd_server) {
                    struct sockaddr_in client_address;
                    socklen_t client_address_len = sizeof(client_address);
                    int new_socket = accept(fd_server, (struct sockaddr *)&client_address, &client_address_len);
                    if (new_socket < 0) {
                        perror("Erro ao aceitar conexão");
                        close(fd_server);
                        return 1;
                    }
                    fds[num_fds].fd = new_socket;
                    fds[num_fds].events = POLLIN;
                    num_fds++;
                } else {
                    handle_connection(fds[i].fd);
                    num_fds--;
                }
            }
        }
    }
    close(fd_server);
    return 0;
}