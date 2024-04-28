#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> // Adicione esta linha

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

    // Aceitar conexões entrantes
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    new_socket = accept(fd, (struct sockaddr *)&client_address, &client_address_len);
    if (new_socket < 0) {
        perror("Erro ao aceitar conexão");
        close(fd);
        return 1;
    }

    // Agora você pode imprimir o endereço IP do cliente usando inet_ntoa
    printf("Conexão aceita de %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    // Fechar os soquetes quando terminar de usá-los
    close(new_socket);
    close(fd);

    return 0;
}