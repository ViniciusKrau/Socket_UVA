#include <stdio.h>
#include <winsock2.h>
#include <stdint.h>
#include <unistd.h>                                     // Impede Warning da função close
#include <time.h>
#define PORTA_SERV 25565

#pragma comment(lib,"ws2_32.lib")                       /* Winsock Library */

void main(int argc, char *argv[]){                      //Passar nome ou ip do servidor em prompt de comando

    if (argc < 2) {
        printf("Nome do servidor não informado\nLocalhost será usado\n");
        argv[1] = "localhost";
    }
    else if (argc > 2){
        printf("Muitos argumentos informados, Fechando programa\n");
        exit(1);
    }
        
    struct sockaddr_in saddr;                           // Informações sobre o socket de recebimento

    // INICIA O WINSOCK
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("Erro ao iniciar Winsock: %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //Criar socket de conexão
    SOCKET conn;
    if ((conn = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erro na criacao do socket: %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //Encontra e armazena as informações do servidor baseado no nome informado no CMD
    //gethostbyname está deprecado mas o getaddrinfo requer outra biblioteca
    struct hostent *hp; 
    if ((hp = gethostbyname(argv[1])) == 0) {
        printf("%s: host desconhecido", argv[1]);
        close(conn);
        exit(EXIT_FAILURE);
    }
    
                                                        //copia as informaçoes de endereço encontrado por gethostbyname
    memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);
    saddr.sin_family = AF_INET;                         //Servidor ipv4
    saddr.sin_port = htons(PORTA_SERV);                 // Porta 25565 definida acima serio possivel passar a porta como argumento junto do Ip do servidor.

                                                        //Connecta ao servidor
    if (connect(conn, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        perror("Conexao.");
        close(conn);
        exit(EXIT_FAILURE);
    }

    //Recebe informação enviada pelo servidor
    clock_t clocks = clock();                          // Marca inicio das transferencias

    uint32_t filelenU;                                  //Formato de recebimento de int em network byte order
    if(recv(conn, (char*)&filelenU, sizeof(filelenU), 0) < 0){
        perror("Erro ao receber tamanho do arquivo");
    }
    int filelen = ntohl(filelenU);                      // Conversão de network byte order to host byte order
    FILE* output = fopen("output.txt", "w+");

    int n = 0;                                          //Int para contagem de char
    char buffer[1024];                                  //buffer a ser preenchido com dados recebidos
    while (filelen > n){                                // Enquanto o arquivo todo nao for recebido
        if (recv(conn, buffer, 1024, 0) < 0){
            perror("Erro ao receber o arquivo");
        }
        n = n + strlen(buffer);                         // Recebe e escreve em buffer
        fprintf(output, buffer);                        // Imprime buffer no arquivo
    }
    long seconds = (((long)(clock() - (long)clocks))/CLOCKS_PER_SEC);
    fseek(output, 0L, SEEK_END);
    long size = ftell(output);
    printf("%d\n", seconds);
    if (seconds == 0) printf("\nTaxa de Transferencia = %d mbps\n", (size*8));
    else printf("\nTaxa de Transferencia = %d mbps\n", (size*8)/seconds); // Calcula e imprime a taxa de transferencia no arquivo

    fclose(output);
    printf("Finalizado\n");
    system("pause");
}