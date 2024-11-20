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

int main(){
    int ret, sd, new_sd, porta;
    socklen_t len;
    pid_t pid;
    char buffer[BUF_LEN];
    struct sockaddr_in my_addr, cl_addr;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(4242);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
    ret = listen(sd, 10);

    while(1){
        len = sizeof(cl_addr);
        new_sd = accept(sd, (struct sockaddr*)&cl_addr, &len);
        printf("accepted connection\n");
        //do I ask for name here?
        pid = fork();
        if(pid == 0){
            close(sd);
            while(1){
                int leng;
                recv(new_sd, &leng, sizeof(leng), 0);
                leng = ntohl(leng);
                printf("received lenght %d\n", leng);
                recv(new_sd, (void*)buffer, leng, 0);
                printf("received buffer %s\n", buffer);
                if(strcmp(buffer, "bye\n") == 0){
                    close(new_sd);
                    break;
                }
                int bit = send(new_sd, (void*)buffer, leng, 0);
            }
        }
        else{
            close(new_sd);
        }
    }



    return 0;
}