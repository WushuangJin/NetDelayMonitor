#include <iostream>
#include <ctype.h>
#include <fstream>
#include <vector>
#include <unordered_map>
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

#define PORT 21730
#define BUFFER_SIZE 100
using namespace std;

int aws_udp_sock, servera_udp_sock;
struct sockaddr_in client_addr, servera_addr;
socklen_t servera_len, client_len;

char city_id;
int start_idx;
int end_to_end_dis, speed_prop, speed_trans;
char map_info[BUFFER_SIZE];
char start_point_info[BUFFER_SIZE];

class City{
public:
    // city name
    char map_id;
    // Propagation speed: km/s
    int prop_speed;
    // Transmission speed: Bytes/s
    int tran_speed;
    //
    unordered_map<int,vector< pair<int, int> > > graph;
    City(){
    }

};

int process_file(string filePath, unordered_map <char, City> &map){
    ifstream file;
    file.open(filePath,ios::in);
    if(!file.is_open()) {
        return 0;
    } else {
        string tempStr;
        char name;
        // the index of cities processed

        while(getline(file,tempStr))
        {
            if(tempStr.empty())
                continue;
             if(isalpha(tempStr[0])) {
                 name = tempStr[0];
                 printf("find city %c \n",name);
                 //reading the speed information
                 map[name] = City();
                 map[name].map_id = name;
                 getline(file,tempStr);
                 map[name].prop_speed = stoi(tempStr);
                 getline(file,tempStr);
                 map[name].tran_speed = stoi(tempStr);

             } else{
                 istringstream in(tempStr);
                 vector<string> temp;
                 string t;
                 while (in >> t) {
                     temp.push_back(t);
                 }
                 int start = stoi(temp[0]);
                 int end = stoi(temp[1]);
                 int dis = stoi(temp[2]);
                 printf("edge from %d to %d\n", start, end);
                 printf("edge from %d to %d\n", end, start);
                 map[name].graph[start].push_back(make_pair(end, dis));
                 map[name].graph[end].push_back(make_pair(start, dis));
             }
        }
    }
    return 0;
}

struct Compare {
    bool operator()(pair<int, int> const & a,pair<int, int> const & b){ 
        return a.second > b.second;
    }
};
typedef pair<int, int> edge;

int compute(int start, City city) {
    unordered_map<int,vector<pair<int, int> > > graph = city.graph;
    int N = graph.size();
    printf("there are %d node in this city ", N);
    vector <bool> visited(N+1,false);
    priority_queue< pair<int, int>, vector<pair<int,int> >, Compare> pq;
    pq.push(make_pair(start, 0));
    int res = 0;
    int cnt_visited = 0;
    while (!pq.empty()) {
        edge curr = pq.top(); 
        pq.pop();
        int node = curr.first;
        int dis = curr.second;
        if (visited[node]) 
            continue;
        visited[node] = true;
        res = max(res, dis);
        ++cnt_visited;
        if (cnt_visited == N) {
            return res;
        }
        for( auto& next : graph[node]) {
            int next_node = next.first;
            int next_dis = dis + next.second;
            pq.push(make_pair(next_node, next_dis));
        } 
    }
    printf("map_info error !");
    return -1;
}

int start_up_socket(unordered_map <char, City> &map) {
    // create udp socket
    servera_udp_sock = socket(AF_INET,SOCK_DGRAM,0);
    servera_addr.sin_family = AF_INET;
    servera_addr.sin_port = htons(PORT);
    servera_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    servera_len = sizeof(servera_addr);
    //create a socket
    int on;
    setsockopt(servera_udp_sock, SOL_SOCKET, SO_REUSEADDR,&on,sizeof(on));
    if(bind(servera_udp_sock, (struct sockaddr*)&servera_addr, servera_len) == -1){
        printf("server A bind error");
        exit(1);
    }
    if(listen(servera_udp_sock, 5) == -1){
        printf("server A listen error");
        exit(1);
    }

    client_len = sizeof(client_addr);

    while(1) {

        if((aws_udp_sock = accept(servera_udp_sock, (struct sockaddr*)&client_addr, &client_len)) == -1){
            printf("server A connect error");
            exit(1);
        } else {
            printf("server A create connection successfully\n");
            int error = send(aws_udp_sock, "You have conected to the server A", strlen("You have conected to the server A"), 0);
            memset(map_info, 0, BUFFER_SIZE);
            memset(start_point_info, 0, BUFFER_SIZE);
            
            recv(aws_udp_sock, map_info, BUFFER_SIZE, 0);
            recv(aws_udp_sock, start_point_info, BUFFER_SIZE, 0);
            city_id = map_info[0];
            start_idx = start_point_info[0] - '0';
            end_to_end_dis= compute(start_idx, map[city_id]);
            speed_prop = map[city_id].prop_speed;
            speed_trans = map[city_id].tran_speed;

            printf("end_to_end_dis : %d \n" , end_to_end_dis);
            printf("prop_speed : %d \n", speed_prop);
            printf("tran_speed : %d \n", speed_trans);

            printf("--------Send dijstra result back to the aws server --------");
            char serverA_to_aws_buf1[BUFFER_SIZE];
            char serverA_to_aws_buf2[BUFFER_SIZE];
            char serverA_to_aws_buf3[BUFFER_SIZE];
            serverA_to_aws_buf1[0] = end_to_end_dis + '0';
            serverA_to_aws_buf2[0] = speed_prop + '0';
            serverA_to_aws_buf3[0] = speed_prop + '0';
            send(aws_udp_sock, serverA_to_aws_buf1, BUFFER_SIZE, 0);
            send(aws_udp_sock, serverA_to_aws_buf2, BUFFER_SIZE, 0);
            send(aws_udp_sock, serverA_to_aws_buf3, BUFFER_SIZE, 0);
            printf("--------Success send the result------------");

        }
    } 

    
}

int main() {
    string filePath = "map.txt";
    unordered_map <char, City> map;
    process_file(filePath, map);
    printf("the size of map : %lu \n", map.size());
    // receive map info from aws
    start_up_socket(map);

    
    return 0;
}