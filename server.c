#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <time.h>

#define BUF_LEN 1024
#define REQUEST_LEN 4

struct quiz
{
    char domanda[BUF_LEN];

    struct quiz *next;
    char risposta[BUF_LEN];
};

struct quiz *fetch_questions(int selected_quiz)
{
    printf("fetch_questions\n");
    struct quiz *head = NULL;
    struct quiz *current = NULL;
    struct quiz *new = NULL;
    FILE *file;
    char line[BUF_LEN];
    char *token;
    char *domanda;
    char *risposta;

    switch (selected_quiz)
    {
    case 1:
        printf("selected quiz 1\n");
        file = fopen("data/quiz1", "r");
        break;
    case 2:
        file = fopen("data/quiz2", "r");
        break;
    default:
        file = fopen("data/quiz1", "r");
        break;
    }
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    while (fgets(line, sizeof(line), file))
    {

        new = (struct quiz *)malloc(sizeof(struct quiz));
        printf("line: %s\n", line);

        strcpy(new->domanda, line);
        printf("new domanda: %s\n", new->domanda);
        fgets(line, sizeof(line), file);

        strcpy(new->risposta, line);
        printf("new risposta: %s\n", new->risposta);
        if(new->risposta[strlen(new->risposta) - 1] != '\n'){
            strcat(new->risposta, "\n");
        }
        new->next = NULL;
        if (head == NULL)
        {
            head = new;
            current = new;
        }
        else
        {
            current->next = new;
            current = new;
        }
    }
    return head;
}

int main()
{
    int ret, sd, new_sd, porta;
    socklen_t len;
    pid_t pid;
    char username[BUF_LEN];
    char buffer[BUF_LEN];
    struct sockaddr_in my_addr, cl_addr;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(4242);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(sd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    ret = listen(sd, 10);

    while (1){
        len = sizeof(cl_addr);
        printf("blocking on accept\n");
        new_sd = accept(sd, (struct sockaddr *)&cl_addr, &len);
        printf("accepted connection\n");

        pid = fork();
        if (pid == 0){
            int selected_quiz = 0;
            int punteggio = 0;
            close(sd);
            // ask for nickname
            // make an entry in database with nickname and current score (0)
            int found = 1;
            while (found)
            {
                found = 0;
               
                int leng = 0;
                recv(new_sd, &leng, sizeof(leng), 0);
                leng = ntohl(leng);

                printf("received lenght %d\n", leng);
                recv(new_sd, (void *)buffer, leng, 0);

                printf("received buffer %s\n", buffer);
                strcpy(username, buffer);
                printf("username: %s\n", username);

                FILE *file;
                char line[BUF_LEN];

                file = fopen("data/username", "r");
                if (file == NULL)
                {
                    perror("Error opening file");
                    exit(EXIT_FAILURE);
                }

                while (fgets(line, sizeof(line), file))
                {

                    printf("username: %s\n", username);
                    if (strcmp(line, username) == 0)
                    {
                        found = 1;
                        break;
                    }
                }

                if (!found)
                {
                    FILE *file_append = fopen("data/username", "a");
                    if (file_append == NULL)
                    {
                        perror("Error opening file for appending");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(file_append, "%s\n", username);
                    fclose(file_append);
                }

                fclose(file);

                if (found)
                {
                    strcpy(buffer, "NO");
                }
                else
                {
                    strcpy(buffer, "OK");
                }

                printf("sending buffer: %s\n", buffer);
                send(new_sd, (void *)buffer, 3, 0);
            }
            // end of nickname loop

            recv(new_sd, &selected_quiz, sizeof(selected_quiz), 0);
            printf("received selected_quiz %d\n", selected_quiz);
            struct quiz *list_question = fetch_questions(selected_quiz);
            struct quiz *current = list_question;
            while (1)
            {
                char risposta[BUF_LEN];
                int net_message_lenght;
                int leng = 0;

                printf("domanda: %s\n", current->domanda);
                printf("risposta: %s\n", current->risposta);
                leng = strlen(current->domanda) + 1;
                net_message_lenght = htonl(leng);
                send(new_sd, &net_message_lenght, sizeof(net_message_lenght), 0);
                printf("sending buffer %s\n", current->domanda);
                send(new_sd, (void *)current->domanda, leng, 0);

                recv(new_sd, &leng, sizeof(leng), 0);
                leng = ntohl(leng);
                recv(new_sd, (void *)risposta, leng, 0);
                printf("received buffer %s\n", risposta);
                printf("strlen risposta %ld\n", strlen(risposta));
                printf("strlen current->risposta %ld\n", strlen(current->risposta));

                

                if (strcmp(risposta, current->risposta) == 0)
                {
                    printf("correct answer\n");
                    punteggio++;
                    strcpy(buffer, "OK");

                    
                }
                else
                {
                    printf("wrong answer\n");
                    strcpy(buffer, "NO");
                }
                current = current->next;
             
                send(new_sd, (void *)buffer, strlen(buffer) + 1, 0);
                memset(risposta, 0, strlen(risposta));
                   if(current == NULL){
                    printf("ho finito le domande\n");
                    printf("punteggio: %d\n", punteggio);
                    break;
                }
            }
            exit(0);    
        }

        else
        {
            close(new_sd);
        }
       
    }

    return 0;
}