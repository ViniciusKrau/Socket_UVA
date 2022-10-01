#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> // Impede Warning da função close
#define PORTA_SERV 25565

#pragma comment(lib,"ws2_32.lib") /* Winsock Library */

void main(){

    struct sockaddr_in saddr;  // Informações sobre o socket de recebimento    
    saddr.sin_family = AF_INET; // Protocolo ipv4
    saddr.sin_addr.s_addr = INADDR_ANY; //Qualquer IP
    saddr.sin_port = htons(PORTA_SERV); // Porta 25565

    // INICIA O WINSOCK
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("Erro ao iniciar Winsock: %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    // Criar um Socket para receber conexões
    SOCKET listening; //Socket de Recebimento
    if ((listening = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erro na criacao do socket: %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    //Associa o socket ao IP de porta definido no saddr Struct.
    if (bind(listening, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        printf("Falha no bind: %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    listen(listening, 1); // Define o Socket "listening" como socket de escuta
    printf("Servidor Iniciado\n");
    while(1){
        struct sockaddr_in client;
        int len_client = sizeof(client);
        SOCKET sock = accept(listening, (struct sockaddr*)&client, &len_client); //Aceita conexão de cliente e cria um socket de comunicação
        if (sock == INVALID_SOCKET){
            printf("Erro na criacao do socket de conexão: %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        unsigned char ip[4];
        memcpy(&ip, &client.sin_addr, 4);
        printf("Conexão de \"%d.%d.%d.%d\"\n", ip[0],ip[1],ip[2],ip[3]); // Imprime IP do cliente para verificação
        
        // Converte instrução recebida para inteiro
        uint32_t filelenU;
        int filelen = 0;
        
        FILE* envio = fopen("envio.txt", "r"); // Abre arquivo para leitura e envio
        if (envio == NULL){ // Verifica erro ao abrir o arquivo
            perror("Erro ao abrir o arquivo");
            close(sock);
            close(listening);
            exit(EXIT_FAILURE);
        }

        // Essa parte pode ser desnecessaria mas pode pegar bugs não previstos.//
        if (filelen == 0){
            while (fgetc(envio) != EOF){
                filelen++;
            }
        }
        rewind(envio);
        
        filelenU = htonl(filelen);                          // Converte len de buffer em network order
        if (send(sock, (char*)&filelenU, sizeof(filelenU), 0) < 0){  // Envia tamanho do arquivo a ser recebido para o cliente
            perror("Erro no envio do tamanho");
        }
        else printf("Tamanho enviado\n");

        char buffer[1024];
        // printf("%s\n", buffer);
        while (fgets(buffer, 1024, envio) != NULL){// envia arquivo linha por linha
            // printf("%s",buffer);
            // printf("%s\n", buffer);
            send(sock, buffer, 1024, 0);
        }
        printf("Arquivo Enviado\n\n");
        fclose(envio);
    }

    exit(EXIT_SUCCESS);
}