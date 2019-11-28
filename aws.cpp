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
#include "client.h"
#include "serverA.h"
#include "serverB.h"


#define TCP_PORT 24730
#define UDPA_PORT 21730
#define UDPB_PORT 22730
#define UDP_PORT 23730
#define SERVER_IP "127.0.0.1" 
#define MAXDATASIZE 100

//define socket for tcp connection with client

int server_tcp_sockfd, client_tcp_sockfd;
socklen_t server_tcp_len,client_tcp_len;
struct sockaddr_in server_tcp_sockaddr,client_tcp_sockaddr; 
struct trans_info receive_file;
struct recv_tcp_info send_file;
// receive_file is file that client send to the aws server through tcp socket

//define socket for udp connection with server A and server B

int servera_udp_sockfd, num_vertix;
double tran_speed, prop_speed;
struct sockaddr_in servera_udp_addr, aws_clienta_addr; 
vector<std::pair<int, int> > distance_res;
socklen_t servera_len, clienta_len;
struct speed_info speed_info_recv;

char dest[MAXDATASIZE];
char dis_in_node[MAXDATASIZE];

int boot_up_udp_a() {
    char buff[MAXDATASIZE];
    printf("\n=================udp socket to server A initialization=================\n"); 
    if ((servera_udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { 
        perror("create udp socket a error ! \n"); 
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

    printf("The AWS has sent map ID and starting vertex to server A server A using UDP over port <%d>”\n", UDP_PORT);
    //sent to the server
    if(sendto(servera_udp_sockfd, buff, MAXDATASIZE, 0,  (struct sockaddr*)&servera_udp_addr, servera_len) == -1){ 
        perror("aws server send query to server A error"); 
    }
    
    // receive dijastra infomation from server A;
    char speed_arr[MAXDATASIZE];

    memset(dest, 0, MAXDATASIZE);
    memset(dis_in_node, 0, MAXDATASIZE);
    memset(speed_arr, 0, MAXDATASIZE);

    recvfrom(servera_udp_sockfd, dest, MAXDATASIZE, 0, (struct sockaddr*)&aws_clienta_addr, &clienta_len);
    recvfrom(servera_udp_sockfd, dis_in_node, MAXDATASIZE, 0, (struct sockaddr*)&aws_clienta_addr, &clienta_len);
    recvfrom(servera_udp_sockfd, speed_arr, MAXDATASIZE, 0, (struct sockaddr*)&aws_clienta_addr, &clienta_len);

    memcpy(&speed_info_recv, speed_arr ,sizeof(speed_info));

    prop_speed = speed_info_recv.prop_speed;
    tran_speed = speed_info_recv.tran_speed;
    num_vertix = speed_info_recv.num_vertix;


    printf("The AWS has received shortest path from server A:\n");
    printf("-------------------------------------------\n");
    printf("Destination\tMin Length\n");
    printf("-------------------------------------------\n");

    for(int i = 0; i < num_vertix; i++) {
        int node = dest[i] - '0';
        int dis = dis_in_node[i] - '0';
        printf("node : %d \t dis :%d \n", node, dis);
        distance_res.push_back(make_pair(node, dis));
    }
    return 0;
}


int serverb_udp_sockfd;
struct sockaddr_in serverb_udp_addr, aws_clientb_addr; 
socklen_t serverb_len, clientb_len;
struct delay_info delay_info_send;
struct result_from_serverB delay_info_recv;

double tp_res[MAXDATASIZE];
double delay_res[MAXDATASIZE];
char send_client_msg[10000];
double tt_res;

int boot_up_udp_b() {
    char buff[MAXDATASIZE];
    double tp_res[MAXDATASIZE];
    double delay_res[MAXDATASIZE];
    double tt_res;
    memset(tp_res, 0, MAXDATASIZE);
    memset(delay_res, 0, MAXDATASIZE);

    printf("\n=================udp socket to server B initialization=================\n"); 
    if ((serverb_udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { 
        perror("create udp socket b error ! \n"); 
        exit(1); 
    }
    memset(&serverb_udp_addr, 0, sizeof(serverb_udp_addr));
    serverb_udp_addr.sin_family = AF_INET; 
    serverb_udp_addr.sin_port = htons(UDPB_PORT); 
    serverb_udp_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverb_len = sizeof(serverb_udp_addr);
    clientb_len = sizeof(aws_clientb_addr); 

    bzero(&(serverb_udp_addr.sin_zero),sizeof(serverb_udp_addr.sin_zero)); 
    bzero(buff,MAXDATASIZE);
    printf("\nBegin connection to server B...\n");

    long file_size = receive_file.size;
    delay_info_send.file_size = file_size;
    delay_info_send.prop_speed = prop_speed;
    delay_info_send.tran_speed = tran_speed;
    delay_info_send.num_vertix = num_vertix;

    for(int i = 0; i < num_vertix; i++) {
        pair<int, int> edge = distance_res[i];
    }
    
    memcpy(buff, &delay_info_send ,sizeof(delay_info)); 

    if(sendto(serverb_udp_sockfd, buff, MAXDATASIZE, 0, (struct sockaddr*)&serverb_udp_addr, serverb_len) == -1){ 
        perror("aws server send query to server B error \n"); 
    }
    sendto(serverb_udp_sockfd, dest, MAXDATASIZE, 0,  (struct sockaddr*)&serverb_udp_addr, serverb_len);
    sendto(serverb_udp_sockfd, dis_in_node, MAXDATASIZE, 0,  (struct sockaddr*)&serverb_udp_addr, serverb_len);
    printf("The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <%d> \n", UDP_PORT);

    recvfrom(serverb_udp_sockfd, delay_res, MAXDATASIZE, 0, (struct sockaddr*)&aws_clientb_addr, &clientb_len);
    memcpy(&delay_info_recv, delay_res, sizeof(result_from_serverB));

    printf("The AWS has received delays from server B:\n");
    printf("--------------------------------------------------\n");
    printf("Destination \t Tt \t Tp \t Delay \n");
    printf("--------------------------------------------------\n");

    tt_res = delay_info_recv.tt;
    send_file.num = num_vertix;

    for(int i = 0; i < num_vertix; i++) {
        tp_res[i] = delay_info_recv.tp[i];
        delay_res[i] = tp_res[i] + tt_res;
        send_file.dest[i] = dest[i] - '0';
        send_file.dis[i] = dis_in_node[i] - '0';
        send_file.tp[i] = tp_res[i];
        send_file.tt[i] = tt_res; 
        send_file.delay[i] = delay_res[i];
        printf("%d \t %.2f \t %.2f \t %.2f \n", send_file.dest[i], tt_res / 1000, tp_res[i] / 1000, delay_res[i] / 1000);
    } 
    return 0;
}

int main() {
    char buf[100];
    char temp[100];
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
    while(1) {
        if((client_tcp_sockfd = accept(server_tcp_sockfd, (struct sockaddr*)&client_tcp_sockaddr, &client_tcp_len)) == -1){
            printf("aws client connection error");
            exit(1);
        } else {
            memset(temp, 0, 100);
            memset(&receive_file, 0, sizeof(trans_info));
            memset(&send_file, 0, sizeof(recv_tcp_info));
            memset(&send_client_msg, 0, sizeof(send_client_msg));
            printf("The AWS is up and running.\n");
            int error = send(client_tcp_sockfd, "You have conected the server", strlen("You have conected the server"), 0);
            recv(client_tcp_sockfd, temp, 100, 0);
            memcpy(&receive_file, temp, sizeof(trans_info));
            printf("The AWS has received map ID <%c>, startex <%d> and file size <%ld> from the client using TCP over port <%d>\n",receive_file.map_idx[0], receive_file.start_idx, receive_file.size, TCP_PORT);
            boot_up_udp_a();
            boot_up_udp_b(); 
            memcpy(send_client_msg, &send_file, sizeof(recv_tcp_info));   
            send(client_tcp_sockfd, send_client_msg, 10000, 0);
            printf("The AWS has sent calculated delay to client using client TCP over port <%d>.\n",TCP_PORT);
        }
    }
    close(server_tcp_sockfd); 
    return 0;
}
