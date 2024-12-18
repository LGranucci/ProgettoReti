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

void ricevi_categorie(int sd, int numCategorie){
    char buffer[1024];
    
    //for(int i = 0; i < numCategorie; i++){
        int lunghezza;
        
        //recv(sd, &lunghezza, sizeof(lunghezza), 0);
        //lunghezza = ntohl(lunghezza);
        //printf("lunghezza %d\n", lunghezza);
        

        //WHAT HAPPENS FOR NOW (MIGHT NEED FIXING) 
        //since max lenght question is higher than lenght of both questions combined, for now just
        //one recieve is enough to accomodate for two sends
        //this is ok-ish for now, but might need fixing in the future

        int bytes_read = recv(sd, buffer, MAX_LENGHT_QUESTION, 0);
        buffer[bytes_read] = '\0';
        printf("%s\n", buffer);
    //}
    return;
}
void question_loop(int sd, char categoriaScelta[1024], int score){
    char buffer[1024];
    int net_message_lenght;
    while(1){
            int lunghezza;
            printf("Quiz - %s\n", categoriaScelta);
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            
            //recv(sd, &lunghezza, sizeof(lunghezza), 0);
            //lunghezza = ntohl(lunghezza);
            //printf("lunghezza 2 %d\n", lunghezza);
            int bytes_read = recv(sd, buffer, MAX_LENGHT_QUESTION, 0);
            buffer[bytes_read] = '\0';
            printf("%s\n", buffer);
            if(strcmp("END\n", buffer) == 0){
                printf("ENDQUIZZED\n");
                return;
            }
            printf("La tua risposta:\n");
            fgets(buffer, 1024, stdin);
            int message_lenght = strlen(buffer) + 1;
            net_message_lenght = htonl(message_lenght);
            //send(sd, &net_message_lenght, sizeof(net_message_lenght), 0);
            send(sd, (void*)buffer, message_lenght, 0);
            memset(buffer,0,MAX_LENGHT_ANSWER);
            
            recv(sd, buffer, 3, 0);
            
           
            printf("%s\n", buffer);

            if(strcmp(buffer, "OK\n") == 0){
                printf("Risposta corretta\n");
                score++;
            }
            else{
                printf("Risposta sbagliata\n");
            }
        }
    return;
}
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
        int numCategorie;
        recv(sd, &numCategorie, sizeof(numCategorie), 0);
        numCategorie = ntohl(numCategorie);
        printf("numero categorie %d\n", numCategorie);

        printf("Quiz disponibili\n");
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");  
        ricevi_categorie(sd, numCategorie);
       
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
        //loop di domanderintf("%d", numCategorie);
        question_loop(sd, categoriaScelta, score);
        break;

    }
    close(sd);
}