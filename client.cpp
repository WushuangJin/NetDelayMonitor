#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include "client.h"

#define SERVER_PORT 24730
#define MAXDATASIZE 100 
#define SERVER_IP "127.0.0.1" 


int main(int argc, char* argv[]) { 
    char city = *argv[1];
    char* from = argv[2];
    char* size = argv[3];
    int start_idx; 
    int file_size;
    sscanf(from, "%d", &start_idx); 
    sscanf(size, "%d", &file_size); 

    struct trans_info send_file;
    struct recv_tcp_info recv_file;
    send_file.map_idx[0] = city;
    send_file.size = file_size;
    send_file.start_idx = start_idx;

  
    int sockfd, numbytes; 
    char buf[MAXDATASIZE]; 
    struct sockaddr_in server_addr; 

    printf("\n======================client initialization======================\n"); 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { 
        perror("socket"); 
        exit(1); 
    }

    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERVER_PORT); 
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); 
    bzero(&(server_addr.sin_zero),sizeof(server_addr.sin_zero)); 

    if (connect(sockfd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)) == -1){
        perror("connect error"); 
        exit(1);
     } 
    
     while(1) { 
        bzero(buf,MAXDATASIZE); 
        printf("\nBegin connection...\n"); 
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1){  
            perror("recv"); 
            exit(1);
        } else if (numbytes > 0) { 
            char temp[100];
            char msg[10000];
            memset(buf, 0, sizeof(buf));
            memset(temp,0,sizeof(temp));
            memcpy(temp, &send_file, sizeof(trans_info));
            int len = sizeof(trans_info); 
            printf("The client is up and running.\n");
            printf("The client has sent query to AWS using TCP : start vertex <%d>; map <%c>; file size <%d>.\n", start_idx, city, file_size);
            //sent to the server
            if(send(sockfd, temp, len, 0) == -1){ 
                perror("send error"); 
            }
            recv(sockfd, msg, 10000, 0);
            memcpy(&recv_file, msg, sizeof(recv_tcp_info));
            printf("The client has received results from AWS: \n");
            printf("----------------------------------------------\n");
            printf("Destination \t Min \t Length \t Tt \t Tp \t Delay \n");
            printf("----------------------------------------------\n");

            for(int i = 0; i < recv_file.num; i++) {
                int dest = recv_file.dest[i];
                int dis = recv_file.dis[i];
                double tt = recv_file.tt[i];
                double tp = recv_file.tp[i];
                double delay = recv_file.delay[i];
                printf("%d\t%d\t%.2f\t%.2f\t%.2f\n", dest, dis, tt, tp, delay);
            }
            break;
        } else { 
            printf("soket end!\n"); 
            break;
        } 
    }  
    close(sockfd); 
    return 0;
}