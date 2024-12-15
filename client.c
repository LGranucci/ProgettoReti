#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX_LENGHT_QUESTION 1024
#define MAX_LENGHT_ANSWER 1024

int main(){
	int ret, sd;
    uint16_t lsmg;
    int scelta = 0;
	struct sockaddr_in sv_addr;
    char buffer[1024];
    int net_message_lenght;
	sd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(4242);
	inet_pton(AF_INET, "127.0.0.1", &sv_addr.sin_addr);
	
    printf("connected\n");
    //qui print del messaggio di benvenuto
    printf("Trivia Quiz\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("Menù:\n");
    printf("1 - Comincia una sessione di Trivia\n");
    printf("2 - Esci\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("La tua scelta:\n");
    fgets(buffer, 1024, stdin);
    if(strcmp(buffer, "1\n") == 0){
        ret = connect(sd, (struct sockaddr*) &sv_addr, sizeof(sv_addr));
        if(ret == -1){
            perror("error connect");
            exit(1);
        }
    }
    else if(strcmp(buffer, "2\n") == 0){
        close(sd);
        exit(0);
    }
    else{
        printf("Scelta non valida\n");
        close(sd);
        exit(1);
    }
    printf("Trivia Quiz\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("Scegli un nickname: (deve essere univoco)\n");
    while(!scelta){
        fgets(buffer, 1024, stdin);
        int message_lenght = strlen(buffer) + 1;
        net_message_lenght = htonl(message_lenght);
        //send(sd, &net_message_lenght, sizeof(net_message_lenght), 0);
        send(sd, (void*)buffer, message_lenght, 0);
        
        memset(buffer,0,strlen(buffer));
        
        recv(sd, buffer, 3, 0);
        printf("%s buffer ricevuto\n", buffer);
        if(strcmp(buffer, "OK\n") == 0){
            scelta = 1;

        }
        else{
            printf("Nickname già in uso, scegline un altro\n");
        }
    }
    while(1){
        int score = 0;
        char categoriaScelta[1024];
        printf("Quiz disponibili\n");
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");  
        printf("1 - Curiosità sulla tecnologia\n");
        printf("2 - Cultura Generale\n");
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");  
        printf("La tua scelta:\n");
        fgets(buffer, 1024, stdin);
        if(strcmp(buffer, "1\n") == 0){
            ret = 1;
            strcpy(categoriaScelta, "Curiosità sulla tecnologia");
        }
        else if(strcmp(buffer, "2\n") == 0){
            ret = 2;
            strcpy(categoriaScelta, "Cultura Generale");
        }
        else{
            printf("Scelta non valida\n");
            continue;
        }
        send(sd, &ret, sizeof(ret), 0);
        //loop di domande
        while(1){
            int lunghezza;
            printf("Quiz - %s\n", categoriaScelta);
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            
            //recv(sd, &lunghezza, sizeof(lunghezza), 0);
            //lunghezza = ntohl(lunghezza);
            //printf("lunghezza 2 %d\n", lunghezza);
            recv(sd, buffer, MAX_LENGHT_QUESTION, 0);

            printf("%s\n", buffer);
            printf("La tua risposta:\n");
            fgets(buffer, 1024, stdin);
            int message_lenght = strlen(buffer) + 1;
            net_message_lenght = htonl(message_lenght);
            //send(sd, &net_message_lenght, sizeof(net_message_lenght), 0);
            send(sd, (void*)buffer, message_lenght, 0);
            memset(buffer,0,strlen(buffer));
            
            recv(sd, buffer, 3, 0);
            printf("%s\n", buffer);
            if(strcmp(buffer, "OK") == 0){
                printf("Risposta corretta\n");
                score++;
            }
            else{
                printf("Risposta sbagliata\n");
            }
        }

    }
    close(sd);
}