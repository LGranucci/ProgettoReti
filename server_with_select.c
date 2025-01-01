#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 4242
#define MAX_CATEGORIA_LENGHT 64
#define NUM_CATEGORIE 2

#define MAX_LENGHT_QUESTION 128
#define MAX_LENGHT_ANSWER 128
#define MAX_USERNAME_LENGHT 64



#define LOGIN 1
#define NUMERO_CAT 2
#define CATEGORIE 3
#define QUIZ 4
#define DOMANDA 5
#define RISPOSTA 6
#define END 7

#define CLASSIFICA_NUMERO 8
#define CLASSIFICA_NUMERO_GIOCATORI 9
#define CLASSIFICA 10
#define MAX_DOMANDE 5


struct risposta{
    char risposta[MAX_LENGHT_ANSWER];
    struct risposta* next;
};
struct question{
    char domanda[MAX_LENGHT_QUESTION];
    struct risposta* risposte;
};
struct quiz{
    char categoria[MAX_CATEGORIA_LENGHT];
    struct question domande[MAX_DOMANDE];
    int isset;
    int numDomande;
};
struct client{
    int socket;
    char username[MAX_USERNAME_LENGHT];
    int score[NUM_CATEGORIE];
    int completed_quiz[NUM_CATEGORIE];
    int current_stage;
    int current_quiz;
    int current_question;
    int old_state;
    int categoria_classifiche;
    struct client** classifiche_locale;
    int lenghts_classifiche_locale[NUM_CATEGORIE];
    struct client* next_client;
};


struct client* client_head;
struct quiz quiz_list[NUM_CATEGORIE];
struct client* classifiche[NUM_CATEGORIE];
int lenghts_classifiche[NUM_CATEGORIE];
int server_sock;

void strip_newline_if_present(char* buffer){
    if(buffer[strlen(buffer) - 1] == '\n'){
        buffer[strlen(buffer) - 1] = '\0';
    }
}

void handle_number_cat(int client_sock, struct client* client, fd_set* acrive_fds);

void close_connection_with_client(struct client* client, int sd){
    //rimuovo il client dalla lista
    struct client* current = client_head;
    struct client* prev = NULL;
    while(current){
        if(current->socket == sd){
            if(prev == NULL){
                client_head = current->next_client;
            }
            else{
                prev->next_client = current->next_client;
            }
            break;
        }
        prev = current;
        current = current->next_client;
    }
    free(current);
    close(sd);

        
}
void signal_handler(int sig){
    printf("Server chiuso\n");
    
    close(server_sock);
    exit(0);
}


int recv_msg(struct client* client, int sd, char* buffer){
    int len, ret;
	int lmsg;
    ret=recv(sd, (void*)&lmsg, sizeof(lmsg), 0);
	if(ret==-1){
        perror("Errore: ");
    }
    if(ret == 0){
        close_connection_with_client(client, sd);
        return END;
    }
	len=ntohl(lmsg);
    ret=recv(sd, (void*)buffer, len, 0);
    if(ret==-1){
        perror("Errore: ");
        exit(1); 
    }
    if(ret == 0){
        close_connection_with_client(client, sd);
        return END;
    }
    if(strcmp(buffer, "CLASSIFICA") == 0){
        printf("classifica\n");
        client->current_stage = CLASSIFICA_NUMERO;
        return CLASSIFICA_NUMERO;
    }
    if(strcmp(buffer, "END") == 0){
        printf("end\n");
        client->current_stage = END;
        close_connection_with_client(client, sd);
        return END;
    }
    return 0;
}


void send_msg(struct client* client, int sd, char* buffer){
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

void send_integer(struct client* client, int sd, int integer){
    int ret;
    int lmsg;
    lmsg = htonl(integer);
    ret = send(sd, (void*)&lmsg, sizeof(lmsg), 0);
    if(ret == -1){
        perror("Errore: ");
        close_connection_with_client(client, sd);
    }
}
int recv_integer(struct client* client, int sd){
    int ret;
    int lmsg;
    ret = recv(sd, (void*)&lmsg, sizeof(lmsg), 0);
    if(ret == -1){
        perror("Errore: ");
        close_connection_with_client(client, sd);
    }
    return ntohl(lmsg);
}
void initialize_clients(){
    client_head = 0;
}



void print_classifiche(){ 
    for(int i = 0; i < NUM_CATEGORIE; i++){
        int aux = 0;
        struct client* current = client_head;
        //calcolo la lunghezza di ogni classifica: se il client è allocato e
        //ha almeno un punto al quiz, lo aggiunge nella classifica
        while(current){
            if(current->socket > 0 && current->score[i] > 0){
                //classifiche[i][aux] = *current;
                aux++;
            }
            current = current->next_client;
        }
        lenghts_classifiche[i] = aux;
        //alloca lo spazio per la classifica
        classifiche[i] = (struct client*)malloc(aux * sizeof(struct client));    

    }
    //riempie le classifiche
    for(int i = 0; i < NUM_CATEGORIE; i++){
        int aux = 0;
        struct client* current = client_head;
        while(current){
            if(current->socket > 0 && current->score[i] > 0){
                classifiche[i][aux] = *current;
                aux++;
            }
            current = current->next_client;
        }
    }
    //ordina tutte le classifiche in ordine decrescente con selection sort
    for(int i = 0; i < NUM_CATEGORIE; i++){
        for(int j = 0; j < lenghts_classifiche[i]; j++){
            for(int k = j + 1; k < lenghts_classifiche[i]; k++){
                if(classifiche[i][j].score[i] < classifiche[i][k].score[i]){
                    struct client temp = classifiche[i][j];
                    classifiche[i][j] = classifiche[i][k];
                    classifiche[i][k] = temp;
                }
            }
        }
    }
    //stampa le classifiche
    for(int i = 0; i < NUM_CATEGORIE; i++){
        printf("Punteggio tema %d\n", i + 1);
        
        for(int j = 0; j < lenghts_classifiche[i]; j++){
            printf("- %s %d\n", classifiche[i][j].username, classifiche[i][j].score[i]);
        }
    }
    //stampa per ogni quiz i giocatori che li hanno completati
    for(int i = 0; i < NUM_CATEGORIE; i++){
        printf("Quiz Tema %d completato\n", i + 1);
        struct client* current = client_head;
        while(current){
            if(current->completed_quiz[i]){
                printf("- %s\n", current->username);
            }
            current = current->next_client;
        }
    }
}


void create_category_list(){
    //prende le categorie da data/categorie
    FILE* file = fopen("data/categorie", "r");
    if(file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    char line[BUFFER_SIZE];
    int i = 0;
    while(fgets(line, sizeof(line), file)){
        printf("categoria %s\n", line);
        //salva le categorie in quiz_list[i].categoria
        strcpy(quiz_list[i].categoria, line);
        quiz_list[i].isset = 1;
        i++;
    }
    fclose(file);
}   


void create_quiz_list(){
    
    char buffer[] = "data/quiz";
    for(int i = 0; i < NUM_CATEGORIE; i++){
        char quiz_file[BUFFER_SIZE];
        char line[BUFFER_SIZE];
        //contatore per il numero di domande
        int j = 0;
        //concatena il buffer con i
        sprintf(quiz_file, "%s%d", buffer, i + 1);
        
        FILE* file = fopen(quiz_file, "r");

        if(file == NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        while(fgets(line, sizeof(line), file)){
            char* token;
            //copia la domanda
            strcpy(quiz_list[i].domande[j].domanda, line);
            fgets(line, sizeof(line), file);
            //devo parsare la risposta: ogni risposta è divisa da | 
            //se non rimuovo il newline, l'ultima risposta sarebbe terminata da \n, mentre le altre no
            strip_newline_if_present(line);

            token = strtok(line, "|");
            
            //salva la prima risposta
            //malloc di new_risposta
            struct risposta* new_risposta = (struct risposta*)malloc(sizeof(struct risposta));
            strcpy(new_risposta->risposta, token);
            new_risposta->next = NULL;
            quiz_list[i].domande[j].risposte = new_risposta;

            struct risposta* current = new_risposta;
            while (token != NULL) {
                token = strtok(NULL, "|");
                //finchè ci sono token da estrarre
                if(token != NULL){
                    struct risposta* new_risposta = (struct risposta*)malloc(sizeof(struct risposta));
                    strcpy(new_risposta->risposta, token);
                    new_risposta->next = NULL;
                    //metto in coda la risposta estratta
                    current->next = new_risposta;
                    current = new_risposta;
                }
                
            }
            j++;
        }

        quiz_list[i].numDomande = j;
        fclose(file);
    }
}


void handle_login(int client_sock, struct client* client, fd_set* acrive_fds){
    char buffer[BUFFER_SIZE];
    int response = 1;
    int ret;
    //ricezione username    
    ret = recv_msg(client,client_sock, buffer);
    if(ret != 0){
        return;
    }
    strip_newline_if_present(buffer);
    //scorro tutti i client per vedere se il nome è già stato usato
    struct client* current = client_head;
    while(current){
        if(strcmp(current->username, buffer) == 0){
            response = 0;
            send_integer(client, client_sock, response);
            return;
        }
        current = current->next_client;
    }
    //se non è stato usato, lo salvo
    strcpy(client->username, buffer);
    send_integer(client, client_sock, response);
    client->current_stage = NUMERO_CAT;
}
    

void handle_number_cat(int client_sock, struct client* client, fd_set* acrive_fds){
    int numCat = NUM_CATEGORIE;
  
    recv_integer(client, client_sock);
    send_integer(client, client_sock, numCat);
    client->current_stage = CATEGORIE;

}


void handle_categorie(int client_sock, struct client* client, fd_set* acrive_fds){
    int ret;
    //mandare categorie
    char categoria[MAX_CATEGORIA_LENGHT];
    ret = recv_integer(client, client_sock);
    //strcpy(categoria, quiz_list[NUM_CATEGORIE - client->contatore].categoria);
    strcpy(categoria, quiz_list[ret].categoria);
    send_msg(client, client_sock, categoria);
   
  
    if(ret == NUM_CATEGORIE - 1) 
        client->current_stage = QUIZ;
    return;
}



void send_question(int client_sock, struct client* client){
    recv_integer(client, client_sock);
    if(client->current_question >= quiz_list[client->current_quiz].numDomande){
        printf("quiz finito\n");
        char buffer[1024];
        strcpy(buffer, "END\n");
        send_msg(client, client_sock, buffer);
        //send endQuiz to client
        client->completed_quiz[client->current_quiz] = 1;
        client->current_stage = CATEGORIE;
        
        return;
    }
    char current_question[MAX_LENGHT_QUESTION];
    //send first quiz question 
    struct quiz current_quiz = quiz_list[client->current_quiz];
    strcpy(current_question, current_quiz.domande[client->current_question].domanda);
    
    send_msg(client, client_sock, current_question);
    client->current_stage = RISPOSTA;
}


void handle_quiz(int client_sock, struct client* client, fd_set* active_fds){
    char buffer[1024];
    int scelta, ret;
     
    ret = recv_msg(client, client_sock, buffer);
    if(ret != 0){
        client->old_state = QUIZ;
        return;
    }
    scelta = atoi(buffer);
    
    if(client->completed_quiz[scelta - 1]){

        strcpy(buffer, "RETRY\n");
        send_msg(client, client_sock, buffer);
        return;
    }
    
    strcpy(buffer, "OK\n");
    send_msg(client, client_sock, buffer);

    client->current_quiz = scelta - 1;
    client->current_stage = DOMANDA;
    client->current_question = 0;
    client->score[scelta - 1] = 0;
}

int check_answer(struct client* client, char* answer){
    struct quiz current_quiz = quiz_list[client->current_quiz];
    struct question current_question = current_quiz.domande[client->current_question];
    struct risposta* current_risposta = current_question.risposte;

    //converto answer in lowercase
    for(int i = 0; i < strlen(answer); i++){
        answer[i] = tolower(answer[i]);
    } 
    while(current_risposta != NULL){
        printf("risposta %s\n", current_risposta->risposta);
        printf("risposta mandata %s\n", answer);
        printf("lunghezza risposta %ld\n", strlen(current_risposta->risposta));
        printf("lunghezza risposta mandata %ld\n", strlen(answer));
        if(strcmp(current_risposta->risposta, answer) == 0){
            return 1;
        }
        current_risposta = current_risposta->next;
    }
    return 0;
}


void handle_answers(int client_sock, struct client* client){
    int ret;
   
    char given_answer[MAX_LENGHT_ANSWER];
    int correct_answer;
    //recieve answer
    ret = recv_msg(client, client_sock, given_answer);
    //tolgo \n
    strip_newline_if_present(given_answer);

    if(ret != 0){
        client->old_state = RISPOSTA;
        printf("messaggio speciale ricevuto\n");
        return;
    }
    correct_answer = check_answer(client, given_answer);
    if(correct_answer){
        send_integer(client, client_sock, 1);
        client->score[client->current_quiz]++;
    }
    else{
        send_integer(client, client_sock, 0);
    }
    client->current_question++;
    
    client->current_stage = DOMANDA;
    
}

void handle_numero_giocatori(int client_sock, struct client* client){
    int categoria = 0;
  
    categoria = recv_integer(client, client_sock);
    if(categoria == NUM_CATEGORIE){
        client->current_stage = client->old_state;
        free(client->classifiche_locale);
        return;
    }
    send_integer(client, client_sock, client->lenghts_classifiche_locale[categoria]);
    if(client->lenghts_classifiche_locale[categoria] == 0){
        return;
    }
    
    
    client->current_stage = CLASSIFICA;
    client->categoria_classifiche = categoria;

}

void handle_classifiche(int client_sock, struct client* client){
    int categoria = client->categoria_classifiche;
    int giocatore_corrente = recv_integer(client, client_sock);
    char buffer[1032];
    //format message with username and score
   
    sprintf(buffer, "%s %d", client->classifiche_locale[categoria][giocatore_corrente].username, client->classifiche_locale[categoria][giocatore_corrente].score[categoria]);    
    send_msg(client, client_sock, buffer);
    if(giocatore_corrente == client->lenghts_classifiche_locale[categoria] - 1){
         client->current_stage = CLASSIFICA_NUMERO_GIOCATORI;
    }
   
}



void print_current_players(){
    int aux = 0;
   
    struct client* client = client_head;
    while(client){
        aux++;
        client = client->next_client;
    }
    printf("Partecipanti: (%d)\n", aux);
   
    client = client_head;
    while(client){
        printf("- %s\n", client->username);
        client = client->next_client;
    }
}

int main() {
    int max_sd, activity, new_socket;
    struct sockaddr_in address, client_address;
    socklen_t client_addr_len;
    //int addrlen = sizeof(address);
    
    fd_set active_fds, read_fds;
    initialize_clients();
    // Creazione socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    create_category_list();
    create_quiz_list();
    //ridefinire il signal handler
    signal(SIGINT, signal_handler);

    // Binding
    memset(&address, 0, sizeof(address));
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
        FD_ZERO(&read_fds);

        FD_SET(server_sock, &read_fds);
       
        struct client* client = client_head;
        while(client){
            FD_SET(client->socket, &read_fds);
            if(client->socket > max_sd){
                max_sd = client->socket;
            }
            client = client->next_client;
        }
        //read_fds = active_fds;

        activity = select(max_sd + 1, &read_fds,NULL,  NULL, NULL);
        print_current_players();
        print_classifiche();
        if ((activity < 0) && (errno != EINTR)) {
            perror("Select error");
        }

        // Verifica nuove connessioni
        if (FD_ISSET(server_sock, &read_fds)) {
            client_addr_len = sizeof(client_address);
            if ((new_socket = accept(server_sock, (struct sockaddr *)&client_address, &client_addr_len)) < 0) {
                perror("Accept");
                exit(EXIT_FAILURE);
            }
            print_current_players();
            printf("New connection, socket fd is %d, ip is: %s, port: %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            //creare struttura dati utente e aggiungerla alla lista
            struct client* new_client = (struct client*)malloc(sizeof(struct client));
            new_client->socket = new_socket;
            new_client->current_stage = LOGIN;
            new_client->current_quiz = 0;
            for(int j = 0; j < NUM_CATEGORIE; j++){
                new_client->score[j] = -1;
                new_client->completed_quiz[j] = 0;
            }
            new_client->current_question = 0;
            new_client->next_client = NULL;
            if(client_head == NULL){
                client_head = new_client;
            }
            else{
                struct client* current = client_head;
                while(current->next_client != NULL){
                    current = current->next_client;
                }
                current->next_client = new_client;
            }
            FD_SET(new_socket, &active_fds);
            if (new_socket > max_sd) {
                max_sd = new_socket;
            }
            continue;
        }

        // Gestione di tutti i client connessi
       
        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                struct client* client = client_head;
                while(client){
                    if(client->socket == i){
                        break;
                    }
                    client = client->next_client;
                }
                if (client->socket != server_sock) {
                    if(client->current_stage == LOGIN){
                        //gestire login
                        handle_login(client->socket, client, &active_fds);
                    }
                    else if(client->current_stage == NUMERO_CAT){
                        handle_number_cat(client->socket, client, &active_fds);
                    }
                    else if(client->current_stage == CATEGORIE){
                        //gestire categorie
                        handle_categorie(client->socket, client, &active_fds);
                    }
                    else if(client->current_stage == QUIZ){
                        //gestire quiz
                        handle_quiz(client->socket, client, &active_fds);
                    }
                    else if(client->current_stage == DOMANDA){
                        //gestire domanda
                        send_question(client->socket, client);
                    }
                    else if(client->current_stage == RISPOSTA){
                        //gestire risposta
                        handle_answers(client->socket, client);
                    }
                    else if(client->current_stage == CLASSIFICA_NUMERO){
                        //gestire punteggio
                        //manda il numero di categorie, che il client potrebbe non sapere
                        handle_number_cat(client->socket, client, &active_fds);
                        client->current_stage = CLASSIFICA_NUMERO_GIOCATORI;
                        //updata la classifica
                     
                        //malloc spazio per classifica locale
                        client->classifiche_locale = (struct client**)malloc(sizeof(struct client*) * NUM_CATEGORIE);
                        for(int j = 0; j < NUM_CATEGORIE; j++){
                            client->classifiche_locale[j] = (struct client*)malloc(sizeof(struct client) * lenghts_classifiche[j]);
                            for(int k = 0; k < lenghts_classifiche[j]; k++){
                                client->classifiche_locale[j][k] = classifiche[j][k];
                            }
                            client->lenghts_classifiche_locale[j] = lenghts_classifiche[j];
                        }
                    }
                    else if(client->current_stage == CLASSIFICA_NUMERO_GIOCATORI){
                        
                        handle_numero_giocatori(client->socket, client);
                    }
                    else if(client->current_stage == CLASSIFICA){
                        //manda la classifica
                        handle_classifiche(client->socket, client);
                 
                    }
                }
            }
        }
    }

    return 0;
}
