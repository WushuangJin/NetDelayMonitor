#include <iostream>
#include <ctype.h>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <string>
#include <sstream>
#include <queue>
#include <algorithm> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <unordered_map>
#include "server_b.h"

#define PORT 22730
#define BUFFER_SIZE 100
using namespace std;

int aws_udp_sock, serverb_udp_sock;
struct sockaddr_in client_addr, serverb_addr;
socklen_t serverb_len, client_len;
vector<pair<int, int> > edges;
unordered_map<int, pair<double, double> > res_map;
struct delay_info delay_info_recv;
struct result_from_serverB result_to_aws;


int compute(vector<pair<int, int> > &edges, double Tt, int prop_speed, unordered_map<int, pair<double, double> > &res_map) {
    for(pair< int, int > edge : edges) {
        int dest = edge.first;
        int dis = edge.second;
        double Tp = 1000 * ((double) dis / double(prop_speed));
        double Delay = Tt + Tp;
        res_map[dest] = make_pair(Tp, Delay);
    }
    return 0;
}


int start_up_socket() {
    serverb_udp_sock = socket(AF_INET,SOCK_DGRAM,0);
    serverb_addr.sin_family = AF_INET;
    serverb_addr.sin_port = htons(PORT);
    serverb_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverb_len = sizeof(serverb_addr);
    client_len = sizeof(client_addr);

    //create a socket
    if(bind(serverb_udp_sock, (struct sockaddr*)&serverb_addr, serverb_len) == -1){
        printf("server B bind error \n");
        exit(1);
    }
    printf("The Server B is up and running using UDP on port <%d>. \n", PORT);

    char msg[BUFFER_SIZE];
    char dest[BUFFER_SIZE];
    char dis_to_node[BUFFER_SIZE];
    char delay_res[BUFFER_SIZE];
    
    while(1) {
        int count;
        int num_vertices, file_size;
        int prop_speed, trans_speed;
        double Tt;
        
        memset(msg, 0, BUFFER_SIZE);
        memset(dest, 0, BUFFER_SIZE);
        memset(dis_to_node, 0, BUFFER_SIZE);
        memset(delay_res, 0, BUFFER_SIZE);

        count = recvfrom(serverb_udp_sock, msg, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if(count == -1) {
            printf("server B connect error");
            exit(1);
        }
        recvfrom(serverb_udp_sock, dest, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        recvfrom(serverb_udp_sock, dis_to_node, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        // load the result of server A 
        // Propagation speed: km/s
        // Transmission speed: Bytes/s
        // Distance: km
        // file_size: bit

        memcpy(&delay_info_recv, msg ,sizeof(delay_info));
        
        file_size = delay_info_recv.file_size;
        prop_speed = delay_info_recv.prop_speed;
        trans_speed = delay_info_recv.tran_speed;
        num_vertices = delay_info_recv.num_vertix;
        
        printf("The Server B has received data for calculation:\n");
        printf("The file_size is %d\n", file_size);
        printf("* Propagation speed: <%d> km/s;\n", prop_speed);
        printf("* Transmission speed <%d> Bytes/s;\n", trans_speed);

        for(int i = 0; i < num_vertices; i++) {
            int node = dest[i] - '0';
            int dis = dis_to_node[i] - '0';
            edges.push_back(make_pair(node, dis));
            printf("* Path length for destination <%d>: <%d> \n", node, dis);
        }
        printf("------------------------\n");

        // compute Tt 
        Tt = 1000 * (double(file_size) / ((8.0) * (double) trans_speed));
        // compute the prop_delay to each node and store the result into res_map
        // the key of res_map is each node and value is pair of (Tp, Delay);
        compute(edges, Tt, prop_speed, res_map);
        printf("The Server B has finished the calculation of the delays:\n");
        printf("------------------------\n");
        printf("Destination \t Delay \n");
        printf("------------------------\n");

        result_to_aws.tt = Tt;

        for(int i = 0; i < num_vertices; i++) {
            int node = dest[i] - '0';
            pair<double, double> t_pair = res_map[node];
            double tp = t_pair.first;
            double delay = t_pair.second;
            result_to_aws.tp[i] = tp;
            printf("%d \t %.2f \n", node, delay);
        }
        memcpy(delay_res, &result_to_aws ,sizeof(result_from_serverB));
        sendto(serverb_udp_sock, delay_res, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, client_len);
        printf("The Server B has finished sending the output to AWS \n");
    } 
}

int main() {
    start_up_socket();
    return 0;
}