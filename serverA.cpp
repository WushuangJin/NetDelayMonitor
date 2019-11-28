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
#include "serverA.h"

#define PORT 21730
#define BUFFER_SIZE 100

using namespace std;


class City{
public:
    // city name
    char map_id;
    // Propagation speed: km/s
    double prop_speed;
    // Transmission speed: Bytes/s
    double tran_speed;
    // Num_edge
    int num_edge;

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
                 //reading the speed information
                 map[name] = City();
                 map[name].map_id = name;
                 map[name].num_edge = 0;
                 getline(file,tempStr);
                 map[name].prop_speed = stod(tempStr);
                 getline(file,tempStr);
                 map[name].tran_speed = stod(tempStr);

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
                //  printf("edge from %d to %d\n", start, end);
                //  printf("edge from %d to %d\n", end, start);
                 map[name].graph[start].push_back(make_pair(end, dis));
                 map[name].graph[end].push_back(make_pair(start, dis));
                 map[name].num_edge = map[name].num_edge + 1;
                 
             }
        }
    }

    printf("The Server A has constructed a list of <%lu> maps: \n", map.size());
    printf("--------------------------------------------- \n");
    printf("Map ID\tNum Verices\tNum Edges\n");
    for(unordered_map<char,City>::iterator iter=map.begin();iter!=map.end();iter++) {
        char Map_ID = iter->first;
        City city = iter->second;
        int num_edges = city.num_edge;
        int num_vertices = city.graph.size();
        printf("%c\t%d\t%d\n", Map_ID, num_vertices, num_edges);
    }
    printf("--------------------------------------------- \n");

    return 0;
}

struct Compare {
    bool operator()(pair<int, int> const & a,pair<int, int> const & b){ 
        return a.second > b.second;
    }
};
typedef pair<int, int> edge;

vector<pair<int,int> > compute(int start, City city) {
    unordered_map<int,vector<pair<int, int> > > graph = city.graph;
    int N = graph.size();
    vector <bool> visited(N+1,false);
    vector <pair<int, int> > routes;
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
        if (dis > 0) {
            routes.push_back(make_pair(node, dis));
        }
        visited[node] = true;
        res = max(res, dis);
        ++cnt_visited;
        if (cnt_visited == N) {
            break;
        }
        for( auto& next : graph[node]) {
            int next_node = next.first;
            int next_dis = dis + next.second;
            pq.push(make_pair(next_node, next_dis));
        } 
    }

    printf("The Server A has identified the following shortest paths: \n");
    printf("------------------------------------- \n");
    printf("Destination \t Min Length\n");
    printf("------------------------------------- \n");
    for(pair<int,int> step : routes) {
        printf("%d\t%d\n", step.first, step.second);
    }
    return routes;
}


int aws_udp_sock, servera_udp_sock;
struct sockaddr_in client_addr, servera_addr;
socklen_t servera_len, client_len;
struct speed_info speed_info_send;

char city_id;
int start_idx;
char msg[BUFFER_SIZE];


int start_up_socket(unordered_map <char, City> &map) {
    // create udp socket
    servera_udp_sock = socket(AF_INET,SOCK_DGRAM,0);
    servera_addr.sin_family = AF_INET;
    servera_addr.sin_port = htons(PORT);
    servera_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    servera_len = sizeof(servera_addr);
    client_len = sizeof(client_addr);

    //create a socket
    if(bind(servera_udp_sock, (struct sockaddr*)&servera_addr, servera_len) == -1){
        printf("server A bind error \n");
        exit(1);
    }

    while(1) {
        int count;
        memset(msg, 0, BUFFER_SIZE);

        count = recvfrom(servera_udp_sock, msg, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if(count == -1) {
            printf("server A connect error");
            exit(1);
        }
        city_id = msg[0];
        start_idx = msg[1] - '0';
        printf("The Server A has received input for finding shortest paths: starting vertex <%d> of map <%c> \n", start_idx, city_id);
        vector<pair<int, int> > routes =  compute(start_idx, map[city_id]);
        char dest[BUFFER_SIZE];
        char dis_in_node[BUFFER_SIZE];
        char speed_arr[BUFFER_SIZE];

        memset(dest, 0, sizeof(dest));
        memset(dis_in_node, 0, sizeof(dis_in_node));
        memset(speed_arr, 0 , sizeof(speed_arr));

        int idx = 0;
        for(pair<int, int> route : routes) {
            dest[idx] = route.first + '0';
            dis_in_node[idx] = route.second + '0';
            idx++;
        }
        speed_info_send.prop_speed = map[city_id].prop_speed ;
        speed_info_send.tran_speed = map[city_id].tran_speed ;
        speed_info_send.num_vertix = idx;

        // store the speed info int a define struct
        memcpy(speed_arr, &speed_info_send ,sizeof(speed_info)); 
        // send corresponding file
        
        sendto(servera_udp_sock, dest, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, client_len);
        sendto(servera_udp_sock, dis_in_node, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, client_len);
        sendto(servera_udp_sock, speed_arr, sizeof(speed_info), 0, (struct sockaddr*)&client_addr, client_len);
        printf("The Server A has sent shortest paths to AWS. \n");
        printf("------------------------------------- \n");

    } 
    return 0;
}

int main() {
    string filePath = "map.txt";
    unordered_map <char, City> map;
    printf("The Server A is up and running using UDP on port <%d>.\n", PORT);
    process_file(filePath, map);
    start_up_socket(map);
    return 0;
}