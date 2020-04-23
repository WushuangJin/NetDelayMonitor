#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "serverA.h" 
#include <fstream>
#include <string>
#include <iostream>
#include <cctype>
#include <sstream>
#include <map>

using namespace std;

#define UDPB_PORT 31534
// read information from map.txt and build map
void read_file(string path, unordered_map<char, city_info> & map) {
	string str;
	ifstream infile;
	infile.open(path, ios::in);
	if(!infile.is_open()) {
		printf("file did not exist");
		exit(0);
	}
	while(getline(infile, str)){
		char map_id;
		if(isalpha(str[0])) {
			map_id = str[0];
			getline(infile, str);
			float prop_speed = stof(str);
			getline(infile, str);
			int trans_speed = stoi(str);
			city_info temp;
			map[map_id] = temp; 
			map[map_id].map_id = map_id;
			map[map_id].prop_speed = prop_speed;
			map[map_id].trans_speed = trans_speed;

		} else {
			//split the str
			vector<string> arr;
			string temp;
			stringstream input(str);
			while(input >> temp) {
				arr.push_back(temp);
			}
			int start = stoi(arr[0]);
			int end = stoi(arr[1]);
			float dis = stof(arr[2]);
			map[map_id].graph[start].push_back(make_pair(end,dis));
			map[map_id].graph[end].push_back(make_pair(start, dis));
			map[map_id].num_edges += 2;
		}

	}

}

int main(int argc, char *argv[])
{
//读取地图
	string path = "map2.txt";
	unordered_map<char, city_info>  city_map;
	read_file(path, city_map);

	// for(auto iter = city_map.begin(); iter != city_map.end(); iter++) {
	// 	printf("the key is : %c ", iter->first);
	// 	if(iter->first == 'X')
	// 		printf("find it !!!!!!\n");
	// 	city_info city = iter->second;
	// 	printf("prop_speed is : %f \n", city.prop_speed);
	// 	printf("tran_speed is : %d \n", city.trans_speed);
		
	// };
	// if(city_map.count('X'))
	// 	printf("second time find it !!!!!!\n");

	int server_sockfd;
	int len, len_aws;
	struct sockaddr_in my_addr;   //服务器网络地址结构体
    struct sockaddr_in remote_addr; //客户端网络地址结构体
	socklen_t sin_size;
	char buf[BUFSIZ]; 
	memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零
	my_addr.sin_family=AF_INET; //设置为IP通信
	my_addr.sin_addr.s_addr=htonl(INADDR_ANY);//服务器IP地址--允许连接到所有本地地址上
	my_addr.sin_port=htons(UDPB_PORT); //服务器端口号


	char buf_header[BUFSIZ];
	char buf_graph[BUFSIZ];
	char buf_node[BUFSIZ];
	
	/*创建服务器端套接字--IPv4协议，面向无连接通信，UDP协议*/
	if((server_sockfd=socket(PF_INET,SOCK_DGRAM,0))<0)
	{  
		perror("socket error");
		return 1;
	}
	
	/*将套接字绑定到服务器的网络地址上*/
	if (::bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)
	{
		perror("bind error");
		return 1;
	}
	sin_size=sizeof(struct sockaddr_in);
	printf("The Server B is up and running using UDP on port <%d> \n", UDPB_PORT);
	/*接收客户端的数据并将其发送给客户端--recvfrom是无连接的*/
	while(1) {
		recvfrom(server_sockfd,buf,BUFSIZ,0,(struct sockaddr *)&remote_addr,&sin_size);
		// receive from tcp connection
		// release the memory all buff arr
		memset(&buf_header, 0, sizeof(buf_header));	
		memset(&buf_graph, 0, sizeof(buf_graph));		
	
		header head;
		char map_id = buf[0];
		
		printf("The Server B has received input for finding graph of map <%c> \n", map_id);
		
		head.map_id = map_id;
		if (!city_map.count(map_id)) {
			head.exist = false;
		} else {
			head.exist = true;
		}
		memcpy(buf_header, &head, sizeof(header));
		sendto(server_sockfd,buf_header,BUFSIZ,0,(struct sockaddr *)&remote_addr, sizeof(remote_addr));
		if(!head.exist) {
			printf("The Server B does not have the required graph id <%c> \n", map_id);
			printf("The Server B has sent “Graph not Found” to AWS \n");
			printf("-------------------------------------------\n");
		} else {
			graph_info info;
			city_info city = city_map[map_id];
			unordered_map<int, vector<pair<int,float>>> graph = city.graph;
			info.num_edges = city.num_edges;
			info.prop_speed = city.prop_speed;
			info.trans_speed = city.trans_speed;
			memcpy(buf_graph, &info, sizeof(graph_info));
			sendto(server_sockfd,buf_graph,BUFSIZ,0,(struct sockaddr *)&remote_addr, sizeof(remote_addr));
			
			for(auto iter = graph.begin(); iter != graph.end(); iter++) {
				int start = iter->first;
				for(auto pair : graph[start]) {
					edge n;
					n.start = start;
					n.end = pair.first;
					n.dis = pair.second;
					memset(&buf_node, 0, sizeof(buf_node));
					memcpy(buf_node, &n, sizeof(edge));
					sendto(server_sockfd,buf_node,BUFSIZ,0,(struct sockaddr *)&remote_addr, sizeof(remote_addr));
				}
			}
			printf("The Server B has sent Graph to AWS \n");
			printf("-------------------------------------------\n");
		}
	}
	/*关闭套接字*/
	close(server_sockfd);
	return 0;
}