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
#define NUM_CATEGORIE 2
#define LOGIN 1
#define QUIZ 2
#define MAX_LENGHT_QUESTION 1024
#define MAX_LENGHT_ANSWER 1024
#define RISPOSTA 3
#define PUNTEGGIO 4
#define END 5


#define MAX_DOMANDE 10
struct questions{
    char domanda[BUFFER_SIZE];
    char risposta[BUFFER_SIZE];
    
};
struct quiz{
    int categoria;
    struct questions domande[MAX_DOMANDE];
};
struct client{
    int socket;
    char username[BUFFER_SIZE];
    int score[NUM_CATEGORIE];
    int current_stage;
    int current_quiz;
    int current_question;
};


struct client clients[MAX_CLIENTS];
struct quiz quiz_list[NUM_CATEGORIE];


void create_quiz_list(){
    quiz_list[0].categoria = 1;
    //fetch quiz from data/quiz1
    FILE* file = fopen("data/quiz1", "r");
    if(file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    char line[BUFFER_SIZE];
    int i = 0;
    while(fgets(line, sizeof(line), file)){
        strcpy(quiz_list[0].domande[i].domanda, line);
        fgets(line, sizeof(line), file);
        strcpy(quiz_list[0].domande[i].risposta, line);
        i++;
    }
    for(int i = 0; i < MAX_DOMANDE; i++){
        printf("domanda %s\n", quiz_list[0].domande[i].domanda);
        printf("risposta %s\n", quiz_list[0].domande[i].risposta);
    }
    fclose(file);
    
}


void handle_login(int client_sock, fd_set* acrive_fds){
    char buffer[BUFFER_SIZE];
    //ricezione username
    int bytes_read = recv(client_sock, buffer, sizeof(buffer), 0);
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            printf("Client %d disconnected.\n", client_sock);
        } else {
            perror("recv");
        }
        close(client_sock);
        FD_CLR(client_sock, acrive_fds);
    } else {
        buffer[bytes_read] = '\0';
        printf("Client %d sent: %s\n", client_sock, buffer);
        // Simulazione risposta
        char response[] = "OK\n";
        for (int i = 0; i < MAX_CLIENTS; i++){
            if(strcmp(clients[i].username, buffer) == 0){
                char response[] = "NO\n";
                send(client_sock, response, strlen(response), 0);
                return;
            }
        }
        strcpy(clients[client_sock].username, buffer);
        clients[client_sock].current_stage = QUIZ;

        send(client_sock, response, strlen(response), 0);
    }
    
}

void send_question(int client_sock){
    for(int i = 0; i < MAX_DOMANDE; i++){
        printf("domanda %s\n", quiz_list[0].domande[i].domanda);
        printf("risposta %s\n", quiz_list[0].domande[i].risposta);
    }




    printf("send question %d\n", clients[client_sock].current_question);
    char current_question[MAX_LENGHT_QUESTION];
    //send first quiz question 
    struct quiz current_quiz = quiz_list[clients[client_sock].current_quiz];
    strcpy(current_question, quiz_list[0].domande[clients[client_sock].current_question].domanda);
    printf("domanda %s\n", quiz_list[0].domande[clients[client_sock].current_question].domanda);
    //for now, I will assume a MAX lenght of answers, this may change
    send(client_sock, current_question, strlen(current_question), 0);
}


void handle_quiz(int client_sock, fd_set* acrive_fds){
    printf("handle quiz\n");

    int ret;
    //ricezione scelta
    recv(client_sock, &ret, sizeof(ret), 0);
    printf("scelta %d\n", ret);
    clients[client_sock].current_quiz = ret;
    clients[client_sock].current_stage = RISPOSTA;
    //send first quiz question 
    send_question(client_sock);
}




void handle_answers(int client_sock){
    char current_answer[MAX_LENGHT_ANSWER];
    //send first quiz question 
    char given_answer[MAX_LENGHT_ANSWER];
    struct quiz current_quiz = quiz_list[clients[client_sock].current_quiz];
    strcpy(current_answer, current_quiz.domande[clients[client_sock].current_question].risposta);
    //recieve answer
    recv(client_sock, given_answer, MAX_LENGHT_ANSWER, 0);
    if(strcmp(current_answer, given_answer) == 0){
        char response[] = "OK\n";
        send(client_sock, response, strlen(response), 0);
        clients[client_sock].score[current_quiz.categoria]++;
    }
    else{
        char response[] = "NO\n";
        send(client_sock, response, strlen(response), 0);
    }
    clients[client_sock].current_question++;
    send_question(client_sock);
}



int main() {
    int server_sock, client_sock, max_sd, activity, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    fd_set active_fds, read_fds;

    // Creazione socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
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
    FD_SET(server_sock, &active_fds);
    max_sd = server_sock;

    while (1) {
        read_fds = active_fds;

        // Uso di select per gestire I/O multiplexing
        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("Select error");
        }

        // Verifica nuove connessioni
        if (FD_ISSET(server_sock, &read_fds)) {
            if ((new_socket = accept(server_sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("Accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            //creare struttura dati utente e aggiungerla alla lista
            clients[new_socket].socket = new_socket;
            clients[new_socket].current_stage = LOGIN;
            clients[new_socket].current_quiz = 0;
            clients[new_socket].score[0] = -1;
            clients[new_socket].score[1] = -1;
            clients[new_socket].current_question = 0;
            // Aggiungere il nuovo socket al set
            FD_SET(new_socket, &active_fds);
            if (new_socket > max_sd) {
                max_sd = new_socket;
            }
        }

        // Gestione di tutti i client connessi
        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i != server_sock) {
                    if(clients[i].current_stage == LOGIN){
                        //gestire login
                        handle_login(i, &active_fds);
                    }
                    else if(clients[i].current_stage == QUIZ){
                        //gestire quiz
                        handle_quiz(i, &active_fds);
                    }
                    else if(clients[i].current_stage == RISPOSTA){
                        //gestire risposta
                        handle_answers(i);
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
