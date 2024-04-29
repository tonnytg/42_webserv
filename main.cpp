#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 1024

void handle_connection(int new_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("Erro ao receber dados");
        close(new_socket);
        return;
    }

    // Verifica se a solicitação HTTP contém "GET" e se solicita um arquivo HTML
    if (strstr(buffer, "GET") != NULL && strstr(buffer, ".html") != NULL) {
        // Envie o arquivo HTML para o cliente
        FILE *html_file = fopen("index.html", "r");
        if (html_file == NULL) {
            perror("Erro ao abrir arquivo HTML");
            close(new_socket);
            return;
        }

        char response_header[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        send(new_socket, response_header, strlen(response_header), 0);

        // Envia o conteúdo do arquivo HTML para o cliente
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
        // Caso contrário, envie uma mensagem de erro ao cliente
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
        send(new_socket, response, strlen(response), 0);
    }

    // Fechar a conexão com o cliente
    close(new_socket);
}

int main() {

    int domain, type, protocol;
    int fd, new_socket;

    domain = PF_INET;
    type = SOCK_STREAM;
    protocol = 0;

    fd = socket(domain, type, protocol);
    if (fd < 0) {
        perror("Erro ao criar o socket");
        return 1;
    }

    // Estrutura para armazenar o endereço de escuta
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;         // Família de endereços IPv4
    server_address.sin_addr.s_addr = INADDR_ANY; // Escuta em todas as interfaces
    server_address.sin_port = htons(8080);       // Porta 8080 (em ordem de bytes da rede)

    // Associando o soquete ao endereço de escuta
    if (bind(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Erro ao associar o soquete ao endereço");
        close(fd);
        return 1;
    }

    printf("Socket associado ao endereço localhost:8080\n");

    // Escutar por conexões
    if (listen(fd, 5) < 0) {
        perror("Erro ao escutar por conexões");
        close(fd);
        return 1;
    }

    printf("Aguardando por conexões entrantes...\n");

    while (1) {
        // Aceitar conexões entrantes
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        new_socket = accept(fd, (struct sockaddr *)&client_address, &client_address_len);
        if (new_socket < 0) {
            perror("Erro ao aceitar conexão");
            close(fd);
            return 1;
        }

        printf("Conexão aceita de %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Manipular a conexão
        handle_connection(new_socket);
    }

    // Fechar o soquete de escuta
    close(fd);

    return 0;
}