#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <signal.h>
#define MAX_LENGHT_QUESTION 1024
#define MAX_LENGHT_ANSWER 1024


//redefine signal for closure of program with ctrl+c
char** arrayCategorie;

void signal_handler(int sig){
    printf("Il server si è disconnesso, per cui il quiz è finito\n");
    exit(0);
}

void recv_msg(int sd, char* buffer){
    int len, ret;
	int lmsg;
    ret=recv(sd, (void*)&lmsg, sizeof(lmsg), 0);
	if(ret==-1){
        perror("Errore: ");
        close(sd);
        exit(1);
    }
    if(ret == 0){
        printf("il server ha chiuso la connessione\n");
        close(sd);
        exit(1);
    }
	len=ntohl(lmsg);
    ret=recv(sd, (void*)buffer, len, 0);
    if(ret==-1){
        perror("Errore: ");
        printf("il server ha chiuso la connessione\n");
        close(sd);
        exit(1);
    }
    if(ret == 0){
        printf("il server ha chiuso la connessione\n");
        close(sd);
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
    if(ret == 0){
        printf("il server ha chiuso la connessione\n");
        close(sd);
        exit(1);
    }
    ret=send(sd, (void*)buffer, len, 0);
    if(ret==-1){
        perror("Errore: ");
        printf("il server ha chiuso la connessione\n");
        close(sd);
        exit(1);
    }
    if(ret == 0){
        printf("il server ha chiuso la connessione\n");
        close(sd);
        exit(1);
    }
}

void send_integer(int sd, int integer){
    int ret;
    int lmsg;
    lmsg = htonl(integer);
    ret = send(sd, (void*)&lmsg, sizeof(int), 0);
    if(ret == 0){
        printf("il server ha chiuso la connessione\n");
        close(sd);
        exit(1);
    }
    if(ret == -1){
        perror("Errore: ");
        exit(1);
    }
   
}
int recv_integer(int sd){
    int ret;
    int lmsg;
    ret = recv(sd, (void*)&lmsg, sizeof(lmsg), 0);
    if(ret == -1){
        perror("Errore: ");
        exit(1);
    }
    if(ret == 0){
        printf("il server ha chiuso la connessione\n");
        close(sd);
        exit(1);
    }
    return ntohl(lmsg);
}


void richiesta_classifica(int sd){
    char buffer[1024];
    int numCategorie;
    strcpy(buffer, "CLASSIFICA");
    send_msg(sd, buffer);

    //ricevi numero di categorie
    send_integer(sd, 0);
    numCategorie = recv_integer(sd);
    for(int i = 0; i < numCategorie; i++){
        printf("Tema %d:\n", i);
        int numGiocatori;
        send_integer(sd, i);
        numGiocatori = recv_integer(sd);
        for(int j = 0; j < numGiocatori; j++){
            memset(buffer,0, 1024);
            send_integer(sd, j);
            recv_msg(sd, buffer);
            printf("- %s\n", buffer);
        }
    }
    send_integer(sd, numCategorie);
    return;
}

void handle_end(int sd){
    char buffer[1024];
    strcpy(buffer, "END");
    send_msg(sd, buffer);
    close(sd);
    return;
}


int get_msg_from_console(char* buffer, int sd){
    
   
        fgets(buffer, 1024, stdin);
        //se il messaggio è show score, chiede al server di inviare la classifica
        if(strcmp(buffer, "show score\n") == 0){
            printf("Classifica\n");
            richiesta_classifica(sd);
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            //devo richiedere il messaggio senza ritoranre al main loop
            get_msg_from_console(buffer, sd);
        }
        //se il messaggio è end quiz, chiede al server di terminare il quiz
        if(strcmp(buffer, "endquiz\n") == 0){
            printf("Termina quiz\n");
            handle_end(sd);
           
            return -1;
        }    
        return 0;
}




void ricevi_categorie(int sd, int numCategorie){
    char buffer[1024];
    printf("%d", numCategorie);
    arrayCategorie = (char**)malloc(numCategorie * sizeof(char*));
    for(int i = 0; i < numCategorie; i++){
        send_integer(sd, i);
        memset(buffer,0, 1024);
        
        recv_msg(sd, buffer);
        arrayCategorie[i] = (char*)malloc(strlen(buffer) + 1);
        strcpy(arrayCategorie[i], buffer);
        printf("%s\n", buffer);

    }
    return;
}
int question_loop(int sd, char categoriaScelta[1024]){
    char buffer[1024];
   
    while(1){
            int ret;
            printf("Quiz - %s", categoriaScelta);
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            send_integer(sd, 1);
            recv_msg(sd, buffer);
           
            if(strcmp("END\n", buffer) == 0){
                return 0;
            }
            printf("%s\n", buffer);
            printf("La tua risposta:\n");
            ret = get_msg_from_console(buffer, sd);
            if(ret == -1){
                return -1;
            }
            send_msg(sd, buffer);
            memset(buffer,0,MAX_LENGHT_ANSWER);
            
            ret = recv_integer(sd);
            
           
            printf("%s\n", buffer);
            if(ret){
                printf("Risposta corretta\n");
            }
            else{
                printf("Risposta sbagliata\n");
            }
        }
    return 0;
}
int login_loop(int sd){
    char buffer[1024];
    int scelta = 0;
    while(!scelta){
        int check;
        check = get_msg_from_console(buffer, sd);
        if(check == -1){
            return -1;
        }
        send_msg(sd, buffer);
        memset(buffer,0,strlen(buffer));
        recv(sd, &check, sizeof(check), 0);
        scelta = ntohl(check);
        if(scelta){
            printf("check passato\n");
        }
        else{
            printf("Nickname già in uso, scegline un altro\n");
        }
    }
    return 0;
}
int scelta_categoria_loop(int sd, char* buffer, char* categoriaScelta, int numCategorie){
    while(1){
        int scelta, ret;
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");  
        printf("La tua scelta:\n");
        ret = get_msg_from_console(buffer, sd);
        if(ret == -1){
            return -1;
        }
        if(!atoi(buffer)){
            printf("Scelta non valida\n");
            continue;
        }
        scelta = atoi(buffer);
        if(scelta > numCategorie){
            printf("Scelta non valida\n");
            continue;
        }
        strcpy(categoriaScelta, arrayCategorie[scelta - 1]);
        send_msg(sd, buffer);
        recv_msg(sd, buffer);

        if(strcmp(buffer, "RETRY\n") != 0){
            return 0;
        }
        printf("hai scelto di partecipare ad un quiz che hai già completato. Seleziona un'altra categoria!\n");
    }
}





int main(int argc, char** argv){
	while(1){
        int ret, sd, port;
        
        /*if(argc != 2){
            printf("Devi immettere il numero di porta");
            exit(1);
        }

        port = atoi(argv[1]);
        */
        port = 4242;
        struct sockaddr_in sv_addr;
        char buffer[1024];
        
        sd = socket(AF_INET, SOCK_STREAM, 0);

        memset(&sv_addr, 0, sizeof(sv_addr));
        sv_addr.sin_family = AF_INET;
        sv_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &sv_addr.sin_addr);
        //ridefinizione di sigpipe
        signal(SIGPIPE, signal_handler);
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
        ret = login_loop(sd);
        if(ret == -1){
            continue;
        }
        char categoriaScelta[1024];
        int numCategorie;
        
        send_integer(sd, 1);
        numCategorie = recv_integer(sd);    
         while(1){
            int ret;
            printf("Quiz disponibili\n");
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");  
            ricevi_categorie(sd, numCategorie);
            
            ret = scelta_categoria_loop(sd, buffer, categoriaScelta, numCategorie);
            if(ret == -1){
                break;
            }
            ret = question_loop(sd, categoriaScelta);
            if(ret == -1){
               break;
            }
        }            
    }
    
    return 0;
}