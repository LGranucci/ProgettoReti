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


void recv_msg(int sd, char* buffer){
    int len, ret;
	int lmsg;
    ret=recv(sd, (void*)&lmsg, sizeof(lmsg), 0);
	if(ret==-1){
                perror("Errore: ");
                exit(1);
                }
	len=ntohl(lmsg);
    ret=recv(sd, (void*)buffer, len, 0);
    if(ret==-1){
        perror("Errore: ");
        exit(1);
    }
}


void send_msg(int sd, char* buffer){
    int len, ret;
    int lmsg;
	len=strlen(buffer)+1;
	lmsg=htonl(len);

    ret=send(sd, (void*)&lmsg, sizeof(lmsg), 0);
	if(ret==-1){
		perror("Errore: ");
		exit(1);
		}
    ret=send(sd, (void*)buffer, len, 0);
    if(ret==-1){
        perror("Errore: ");
        exit(1);
    }
}

void send_integer(int sd, int integer){
    int ret;
    int lmsg;
    lmsg = htonl(integer);
    ret = send(sd, (void*)&lmsg, sizeof(int), 0);
    if(ret == -1){
        perror("Errore: ");
        exit(1);
    }
}


void ricevi_categorie(int sd, int numCategorie){
    char buffer[1024];
    printf("%d", numCategorie);
    for(int i = 0; i < numCategorie; i++){
        int lunghezza = 0;
        send_integer(sd, i);
        memset(buffer,0, 1024);
        
        recv_msg(sd, buffer);
        printf("%s\n", buffer);
    }
    return;
}
void question_loop(int sd, char categoriaScelta[1024]){
    char buffer[1024];
    int net_message_lenght;
    while(1){
            int lunghezza;
            printf("Quiz - %s\n", categoriaScelta);
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            recv_msg(sd, buffer);
            printf("%s\n", buffer);
            if(strcmp("END\n", buffer) == 0){
                printf("ENDQUIZZED\n");
                return;
            }
            printf("La tua risposta:\n");
            fgets(buffer, 1024, stdin);
            send_msg(sd, buffer);
            memset(buffer,0,MAX_LENGHT_ANSWER);
            
            recv(sd, buffer, 3, 0);
            
           
            printf("%s\n", buffer);
            if(strcmp(buffer, "OK\n") == 0){
                printf("Risposta corretta\n");
            }
            else{
                printf("Risposta sbagliata\n");
            }
        }
    return;
}
void login_loop(int sd){
    char buffer[1024];
    int scelta = 0;
    while(!scelta){
        int check;
        fgets(buffer, 1024, stdin);
       
        send_msg(sd, buffer);
        memset(buffer,0,strlen(buffer));
        recv(sd, &check, sizeof(check), 0);
        scelta = ntohl(check);
        printf("%d check ricevuto\n", scelta);
        if(scelta){
            printf("check passato");
        }
        else{
            printf("Nickname già in uso, scegline un altro\n");
        }
    }
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
    login_loop(sd);
    

    
    char categoriaScelta[1024];
    int numCategorie;
    int score = 0;
    printf("richiesta numero categorie\n");
    recv_msg(sd, categoriaScelta);
    numCategorie = atoi(categoriaScelta);
       
    printf("numero categorie %d\n", numCategorie);

    printf("Quiz disponibili\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");  
    ricevi_categorie(sd, numCategorie);
       
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");  
    printf("La tua scelta:\n");
    fgets(buffer, 1024, stdin);
    if(strcmp(buffer, "1\n") == 0){
           
        strcpy(categoriaScelta, "Curiosità sulla tecnologia");
    }
    else if(strcmp(buffer, "2\n") == 0){
          
        strcpy(categoriaScelta, "Cultura Generale");
    }
    else{
        printf("Scelta non valida\n");
            
    }

    send_msg(sd, buffer);
        //loop di domanderintf("%d", numCategorie);
    question_loop(sd, categoriaScelta);
        

    
    close(sd);
}