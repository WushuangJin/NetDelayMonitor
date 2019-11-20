#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <vector>
#include "../client/client.h"


#define TCP_PORT 24730
#define UDPA_PORT 21730
#define UDPB_PORT 22730
#define SERVER_IP "127.0.0.1" 
#define MAXDATASIZE 100

//define socket for tcp connection with client
// receive_file is file that client send to the aws server through tcp socket

int server_tcp_sockfd, client_tcp_sockfd;
socklen_t server_tcp_len,client_tcp_len;
struct sockaddr_in server_tcp_sockaddr,client_tcp_sockaddr; 
struct trans_info receive_file;

//boot up the tcp connection between aws server and the client
int boot_up_tcp(){
    char buf[100];
    char temp[100];
    memset(buf, 0, 100);
    memset(temp, 0, 100);
    // receive_file is file from client to aws

    //server_tcp_sockaddr is socket as a server for client
    /*create a socket.type is AF_INET,sock_stream*/
    server_tcp_sockfd = socket(AF_INET,SOCK_STREAM,0);
    server_tcp_sockaddr.sin_family = AF_INET;
    server_tcp_sockaddr.sin_port = htons(TCP_PORT);
    server_tcp_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_tcp_len = sizeof(server_tcp_sockaddr);
    client_tcp_len = sizeof(client_tcp_sockaddr);

    int on;
    setsockopt(server_tcp_sockfd, SOL_SOCKET, SO_REUSEADDR,&on,sizeof(on));
    // bind the tcp socket for the client to connect with
    if(bind(server_tcp_sockfd, (struct sockaddr*)&server_tcp_sockaddr, server_tcp_len) == -1){
        printf("aws server tcp socket bind error");
        exit(1);
    }
    if(listen(server_tcp_sockfd, 5) == -1){
        printf("aws server tcp socket listen error");
        exit(1);
    }
    if((client_tcp_sockfd = accept(server_tcp_sockfd, (struct sockaddr*)&client_tcp_sockaddr, &client_tcp_len)) == -1){
        printf("aws client connection error");
        exit(1);
    } else {
        printf("create connection successfully\n");
        int error = send(client_tcp_sockfd, "You have conected the server", strlen("You have conected the server"), 0);
        recv(client_tcp_sockfd, temp, 100, 0);
        memcpy(&receive_file, temp, sizeof(trans_info));
        printf("%d\n", receive_file.size);
        printf("%d\n", receive_file.start_idx);
        printf("%c\n", receive_file.map_idx[0]);   
    }
    return 0;
}


//define socket for udp connection with server A and server B

int servera_udp_sockfd, tran_speed, prop_speed;
struct sockaddr_in servera_udp_addr, aws_clienta_addr; 
vector<std::pair<int, int> > distance_res;
socklen_t servera_len, clienta_len;

int boot_up_udp_a() {
    char buff[MAXDATASIZE];
    printf("\n=================udp socket to server A initialization=================\n"); 
    if ((servera_udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { 
        perror("create udp socket error ! \n"); 
        exit(1); 
    }
    memset(&servera_udp_addr, 0, sizeof(servera_udp_addr));
    servera_udp_addr.sin_family = AF_INET; 
    servera_udp_addr.sin_port = htons(UDPA_PORT); 
    servera_udp_addr.sin_addr.s_addr = inet_addr(SERVER_IP); 

    bzero(&(servera_udp_addr.sin_zero),sizeof(servera_udp_addr.sin_zero)); 
    bzero(buff,MAXDATASIZE);
    printf("\nBegin connection to server A...\n");

    buff[0] = receive_file.map_idx[0];
    buff[1] = receive_file.start_idx + '0';
    servera_len = sizeof(servera_udp_addr);
    clienta_len = sizeof(aws_clienta_addr);

    printf("success send the query info from aws server to server A\n");
    //sent to the server
    if(sendto(servera_udp_sockfd, buff, MAXDATASIZE, 0,  (struct sockaddr*)&servera_udp_addr, servera_len) == -1){ 
        perror("aws server send query to server A error"); 
    }
    
    // receive dijastra infomation from server A;
    char dest[MAXDATASIZE];
    char dis_in_node[MAXDATASIZE];
    char speed_arr[MAXDATASIZE];

    recvfrom(servera_udp_sockfd, dest, MAXDATASIZE, 0, (struct sockaddr*)&aws_clienta_addr, &clienta_len);
    recvfrom(servera_udp_sockfd, dis_in_node, MAXDATASIZE, 0, (struct sockaddr*)&aws_clienta_addr, &clienta_len);
    recvfrom(servera_udp_sockfd, speed_arr, MAXDATASIZE, 0, (struct sockaddr*)&aws_clienta_addr, &clienta_len);

    int num_vertix;
    prop_speed = speed_arr[0] - '0';
    tran_speed = speed_arr[1] - '0';
    num_vertix = speed_arr[2] - '0';

    for(int i = 0; i < num_vertix; i++) {
        int node = dest[i] - '0';
        int dis = dis_in_node[i] - '0';
        printf("node : %d \t dis :%d \n", node, dis);
        //distance_res.push_back(make_pair(node, dis));
    }
    return 0;
}

int boot_up_udp_b() {
    return 0;
}

int main() {
    boot_up_tcp();
    boot_up_udp_a();
    // boot_up_udp_b();
}
