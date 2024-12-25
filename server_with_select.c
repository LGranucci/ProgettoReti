#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 4242
#define MAX_CATEGORIA_LENGHT 50
#define NUM_CATEGORIE 2

#define MAX_LENGHT_QUESTION 1024
#define MAX_LENGHT_ANSWER 1024


#define LOGIN 1
#define NUMERO_CAT 2
#define CATEGORIE 3
#define QUIZ 4
#define RISPOSTA 5
#define PUNTEGGIO 6
#define END 7


#define MAX_DOMANDE 5
struct question{
    char domanda[BUFFER_SIZE];
    char risposta[BUFFER_SIZE];
    
};
struct quiz{
    char categoria[MAX_CATEGORIA_LENGHT];
    struct question domande[MAX_DOMANDE];
    int isset;
    int numDomande;
};
struct client{
    int socket;
    char username[BUFFER_SIZE];
    int score[NUM_CATEGORIE];
    int completed_quiz[NUM_CATEGORIE];
    int current_stage;
    int current_quiz;
    int current_question;

   
    struct client* next_client;
};


struct client clients[MAX_CLIENTS];
struct quiz quiz_list[NUM_CATEGORIE];

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

    ret=send(sd, (void*)&lmsg, sizeof(int), 0);
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
    ret = send(sd, (void*)&lmsg, sizeof(lmsg), 0);
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
    return ntohl(lmsg);
}
void initialize_clients(){
    for(int i = 0; i < MAX_CLIENTS; i++){
        clients[i].socket = 0;
        strcpy(clients[i].username, "");
        clients[i].current_stage = 0;

    }
}



void print_classifiche(){
    struct client* classifiche[NUM_CATEGORIE];
    int lenghts_classifiche[NUM_CATEGORIE];
    int aux = 0;
    for(int i = 0; i < MAX_CLIENTS; i++){
        classifiche[i] = malloc(sizeof(struct client) * MAX_CLIENTS);
    }
    
    for(int i = 0; i < NUM_CATEGORIE; i++){
        for(int j = 0; j < MAX_CLIENTS; i++){
            //per ogni categoria, per ogni cliente, controlla se hanno giocato al quiz e metti il loro score
            if(clients[j].score[i] > 0){
                classifiche[i][aux] = clients[j];
                aux++;
            }
            lenghts_classifiche[i] = aux;
        }
        //ordina classifica[i]

    }
    for(int i = 0; i < NUM_CATEGORIE; i++){
        printf("Punteggio tema %d\n", i);
        for(int j = 0; j < lenghts_classifiche[i]; j++){
            printf("ustente %s con punteggio %d sul test %d\n", classifiche[i][j].username, classifiche[i][j].score[i], i);
        }
    }
    //stampa le classifiche
    
}


void create_category_list(){
    //fetch categorie from data/categorie
    FILE* file = fopen("data/categorie", "r");
    if(file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    char line[BUFFER_SIZE];
    int i = 0;
    while(fgets(line, sizeof(line), file)){
        printf("categoria %s\n", line);
        //allocate 
        strcpy(quiz_list[i].categoria, line);
        quiz_list[i].isset = 1;
        i++;
    }
    fclose(file);
}   


void create_quiz_list(){

    //fetch quiz from data/quiz1
    char buffer[] = "data/quiz";
    for(int i = 0; i < NUM_CATEGORIE; i++){
        //concatenate buffer with i
        char quiz_file[BUFFER_SIZE];
       
        sprintf(quiz_file, "%s%d", buffer, i + 1);
        //open file
        printf("opening %s\n", quiz_file);

         FILE* file = fopen(quiz_file, "r");

        if(file == NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        char line[BUFFER_SIZE];
        int j = 0;
        while(fgets(line, sizeof(line), file)){
            strcpy(quiz_list[i].domande[j].domanda, line);
            fgets(line, sizeof(line), file);
            strcpy(quiz_list[i].domande[j].risposta, line);
            j++;
        }
        quiz_list[i].numDomande = j;
        for(int j = 0; j < MAX_DOMANDE; j++){
            printf("domanda %s\n", quiz_list[i].domande[j].domanda);
            printf("risposta %s\n", quiz_list[i].domande[j].risposta);
        }
        fclose(file);
    }
}


void handle_login(int client_sock, struct client* client, fd_set* acrive_fds){
    char buffer[BUFFER_SIZE];
    int response = 1;
    int numCat = NUM_CATEGORIE;
    printf("handle_login");
   
    //ricezione username    
    recv_msg(client_sock, buffer);
    printf("Client %d sent: %s\n", client_sock, buffer);
    
    for (int i = 0; i < MAX_CLIENTS; i++){
        printf("for iteration i = %d\n", i);
        if(strcmp(clients[i].username, buffer) == 0){
            printf("ENTERED IN WRONG STATE\n");
            response = 0;
            response = htonl(response);
            send(client_sock, &response, sizeof(response), 0);
            return;
        }
    }
    strcpy(client->username, buffer);
    printf("client username copied %s\n", client->username);
    client->current_stage = NUMERO_CAT;
        
    response = htonl(response);
    send(client_sock, &response, sizeof(response), 0);
    
   
    sprintf(buffer, "%d", numCat);
    send_msg(client_sock, buffer);
    client->current_stage = CATEGORIE;

}
    

void handle_number_cat(int client_sock, struct client* client, fd_set* acrive_fds){
    int numCat = NUM_CATEGORIE;
    char buffer[1024];
    printf("handle_number_cat\n");

    
    //mandare numero di categorie
    
    
    client->current_stage = CATEGORIE;


}


void handle_categorie(int client_sock, struct client* client, fd_set* acrive_fds){
    printf("handle_categorie\n");
    int ret, len;
    //mandare categorie

    //I need to send lenght of categoria and then the categoria, one at a time. there therefore needs to be a counter that has to be decremented
    //in order to get when it needs to stop working. this counter has to be on the user.
    char categoria[MAX_CATEGORIA_LENGHT];
    ret = recv_integer(client_sock);
    //strcpy(categoria, quiz_list[NUM_CATEGORIE - client->contatore].categoria);
    strcpy(categoria, quiz_list[ret].categoria);
    send_msg(client_sock, categoria);
   
  
    if(ret == NUM_CATEGORIE - 1) 
        client->current_stage = QUIZ;
    printf("current stage: %d\n", client->current_stage);
    return;
}



void send_question(int client_sock, struct client* client){
    printf("send_question");
    if(client->current_question >= quiz_list[client->current_quiz].numDomande){
        printf("quiz finito\n");
        char endQuiz[] = "END\n";
        //send endQuiz to client
        send(client_sock, endQuiz, strlen(endQuiz), 0);
        client->current_stage = PUNTEGGIO;
        print_classifiche();
        return;
    }
    printf("send question %d\n", client->current_question);
    char current_question[MAX_LENGHT_QUESTION];
    //send first quiz question 
    struct quiz current_quiz = quiz_list[client->current_quiz];
    strcpy(current_question, current_quiz.domande[client->current_question].domanda);

    send_msg(client_sock, current_question);
    printf("domanda %s\n", current_quiz.domande[client->current_question].domanda);
}


void handle_quiz(int client_sock, struct client* client, fd_set* active_fds){
    char buffer[1024];
    int scelta;
    printf("handle quiz\n");
    recv_msg(client_sock, buffer);

    printf("scelta %s\n", buffer);
    scelta = atoi(buffer);
    client->current_quiz = scelta - 1;
    client->current_stage = RISPOSTA;
    client->current_question = 0;
    client->score[scelta - 1] = 0;
    //send first quiz question 
    send_question(client_sock, client);
}




void handle_answers(int client_sock, struct client* client){
    printf("handle_answer\n");
    char current_answer[MAX_LENGHT_ANSWER];
    //send first quiz question 
    char given_answer[MAX_LENGHT_ANSWER];
    struct quiz current_quiz = quiz_list[client->current_quiz];
    strcpy(current_answer, quiz_list[client->current_quiz].domande[client->current_question].risposta);
    //recieve answer
    recv_msg(client_sock, given_answer);
    printf("risposta data %s\n", given_answer);
    printf("risposta corretta %s\n", current_answer);
    printf("lunghezza risposta data %ld\n", strlen(given_answer));
    printf("lunghezza risposta corretta %ld\n", strlen(current_answer));
    if(strcmp(current_answer, given_answer) == 0){
        char response[] = "OK\n";
        send(client_sock, response, strlen(response), 0);
        client->score[client->current_quiz]++;
    }
    else{
        char response[] = "NO\n";
        send(client_sock, response, strlen(response), 0);
    }
    client->current_question++;
    send_question(client_sock, client);
}

void print_current_players(){
    printf("Giocatori correnti:\n");
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].socket != 0){
            printf("%d) %s\n",i, clients[i].username);
        }
    }
}

int main() {
    int server_sock, client_sock, max_sd, activity, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    fd_set active_fds, read_fds;
    initialize_clients();
    // Creazione socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    create_category_list();
    create_quiz_list();
    // Binding
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Inizializzazione del set di file descriptor
    FD_ZERO(&active_fds);
    FD_ZERO(&read_fds);
    FD_SET(server_sock, &active_fds);
    max_sd = server_sock;

    while (1) {
        printf("starting loop\n");
        
        int sd;
        read_fds = active_fds;
        printf("blocking on select whith max_sd = %d\n", max_sd);
        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        printf("exiting select\n");
        if ((activity < 0) && (errno != EINTR)) {
            perror("Select error");
        }

        // Verifica nuove connessioni
        if (FD_ISSET(server_sock, &read_fds)) {
            if ((new_socket = accept(server_sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("Accept");
                exit(EXIT_FAILURE);
            }
            print_current_players();
            printf("New connection, socket fd is %d, ip is: %s, port: %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            //creare struttura dati utente e aggiungerla alla lista
            for(int i = 0; i < MAX_CLIENTS; i++){
                if(clients[i].socket == 0){
                    printf("added to array in position %d\n", i);
                    clients[i].socket = new_socket;
                    clients[i].current_stage = LOGIN;
                    clients[i].current_quiz = 0;
                    clients[i].score[0] = -1;
                    clients[i].score[1] = -1;
                    clients[i].current_question = 0;
                    
                    break;
                }
            }
            
            // Aggiungere il nuovo socket al set
            FD_SET(new_socket, &active_fds);
            if (new_socket > max_sd) {
                max_sd = new_socket;
            }
            continue;
        }

        // Gestione di tutti i client connessi
        printf("max_sd: %d\n", max_sd);

        for (int j = 0; j <= max_sd; j++) {
            printf("checking socket %d, result = %d", j, FD_ISSET(j, &read_fds));
            if (FD_ISSET(j, &read_fds)) {
                int i;
                for(i = 0; i < MAX_CLIENTS; i++){
                    if(clients[i].socket == j){
                        break;
                    }
                }
                printf("selected client %d, with socket %d", i, clients[i].socket);
                if (clients[i].socket != server_sock) {
                    if(clients[i].current_stage == LOGIN){
                        //gestire login
                        handle_login(clients[i].socket, &clients[i], &active_fds);
                    }
                    else if(clients[i].current_stage == NUMERO_CAT){
                        handle_number_cat(clients[i].socket, &clients[i], &active_fds);
                    }
                    else if(clients[i].current_stage == CATEGORIE){
                        //gestire categorie
                        handle_categorie(clients[i].socket, &clients[i], &active_fds);
                    }
                    else if(clients[i].current_stage == QUIZ){
                        //gestire quiz
                        handle_quiz(clients[i].socket, &clients[i], &active_fds);
                    }
                    else if(clients[i].current_stage == RISPOSTA){
                        //gestire risposta
                        handle_answers(clients[i].socket, &clients[i]);
                    }
                    else if(clients[i].current_stage == PUNTEGGIO){
                        //gestire punteggio
                    }
                    else if(clients[i].current_stage == END){
                        //gestire fine
                    }
                }
            }
        }
    }

    return 0;
}
