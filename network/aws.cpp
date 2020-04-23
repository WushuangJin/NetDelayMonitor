
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "serverA.h"
#include "serverC.h"
#include <iostream>
#include<cctype>
#include <sstream>
#include <map>
#include<set>

#define TCP_PORT 34534
#define UDPA_PORT 30534
#define UDPB_PORT 31534
#define UDPC_PORT 32534
#define UDP_PORT 33534
#define SERVER_IP "127.0.0.1" 

struct send_info recv_file;
msg_c result;
header head_msg_a , head_msg_b;
graph_info graph_msg_a, graph_msg_b;
vector<edge> edge_msg_lista, edge_msg_listb;

using namespace std;
//UDP_A
int udp_A() {
	int client_sockfd_A;
	int len_A;
	struct sockaddr_in remote_addr_A, client_addr_A;
	socklen_t client_size,remote_size;
	char buf_A[BUFSIZ];
	char buf_header[BUFSIZ];
	char buf_graph[BUFSIZ];
	char buf_edge[BUFSIZ];
	edge _edge;
	set <int> s;

	// clean the structure
	memset(&head_msg_a, 0, sizeof(head_msg_a));
	memset(&graph_msg_a, 0, sizeof(graph_msg_a));
	memset(&edge_msg_lista, 0, sizeof(edge_msg_lista));
	memset(&remote_addr_A,0,sizeof(remote_addr_A)); 

	// define the upd 
	remote_addr_A.sin_family=AF_INET; //设置为IP通信
	remote_addr_A.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址
	remote_addr_A.sin_port=htons(UDPA_PORT); //服务器端口号
	
	// create udp socket
	if((client_sockfd_A=socket(AF_INET,SOCK_DGRAM,0))<0)
	{  
		perror("socket error");
		return 1;
	}

	remote_size = sizeof(remote_addr_A);
	client_size = sizeof(client_addr_A);
	int start_idx = recv_file.start_idx;
	int end_idx = recv_file.end_idx;

	//send query to serverA
	buf_A[0]= recv_file.map_id;
	sendto(client_sockfd_A,buf_A,BUFSIZ,0,(struct sockaddr *)&remote_addr_A, remote_size);
	// receive header file from serverA to judge whether the map_id existed in serverA
	recvfrom(client_sockfd_A,buf_header,BUFSIZ,0,(struct sockaddr *)&remote_addr_A, &client_size);
	memcpy(&head_msg_a, buf_header, sizeof(header));
	// if existed then recevfrom city related information
	if(head_msg_a.exist) {
		recvfrom(client_sockfd_A,buf_graph,BUFSIZ,0,(struct sockaddr *)&remote_addr_A, &client_size);
		printf("The AWS has received map information from server <A>\n");
		memcpy(&graph_msg_a, buf_graph, sizeof(graph_info));
		int num_edges = graph_msg_a.num_edges;
		// printf("num_edges : %d \n", num_edges);
		for(int i = 0; i < num_edges; i++) {
			memset(&buf_edge, 0, sizeof(buf_edge));
			memset(&_edge, 0, sizeof(_edge));
			edge temp;
			recvfrom(client_sockfd_A,buf_edge,BUFSIZ,0,(struct sockaddr *)&remote_addr_A, &client_size);
			memcpy(&_edge, buf_edge, sizeof(edge));
			temp = _edge;
			edge_msg_lista.push_back(temp);
			s.insert(temp.start);
			s.insert(temp.end);	
		}
		if(s.find(start_idx) != s.end()) {
			result.start_existed = true;
		}
		if(s.find(end_idx) != s.end()) {
			result.end_existed = true;
		}

	}
	close(client_sockfd_A);
	return 0;
}

//UDP_B
int udp_B() {

	int client_sockfd_B;
	int len_B;
	struct sockaddr_in remote_addr_B, client_addr_B;
	socklen_t client_size,remote_size;
	char buf_B[BUFSIZ];
	char buf_header[BUFSIZ];
	char buf_graph[BUFSIZ];
	char buf_edge[BUFSIZ];
	edge _edge;
	set <int> s;

	// clean the structure
	memset(&head_msg_b, 0, sizeof(head_msg_b));
	memset(&graph_msg_b, 0, sizeof(graph_msg_b));
	memset(&edge_msg_listb, 0, sizeof(edge_msg_listb));
	memset(&remote_addr_B,0,sizeof(remote_addr_B)); 

	// define the upd 
	remote_addr_B.sin_family=AF_INET; //设置为IP通信
	remote_addr_B.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址
	remote_addr_B.sin_port=htons(UDPB_PORT); //服务器端口号
	
	// create udp socket
	if((client_sockfd_B=socket(AF_INET,SOCK_DGRAM,0))<0)
	{  
		perror("socket error");
		return 1;
	}
	remote_size = sizeof(remote_addr_B);
	client_size = sizeof(client_addr_B);
	int start_idx = recv_file.start_idx;
	int end_idx = recv_file.end_idx;
	//send query to serverA
	buf_B[0]= recv_file.map_id;
	sendto(client_sockfd_B,buf_B,BUFSIZ,0,(struct sockaddr *)&remote_addr_B, remote_size);
	// receive header file from serverB to judge whether the map_id existed in serverA
	recvfrom(client_sockfd_B,buf_header,BUFSIZ,0,(struct sockaddr *)&remote_addr_B, &client_size);
	memcpy(&head_msg_b, buf_header, sizeof(header));
	// if existed then recevfrom city related information
	if(head_msg_b.exist) {
		recvfrom(client_sockfd_B,buf_graph,BUFSIZ,0,(struct sockaddr *)&remote_addr_B, &client_size);
		printf("The AWS has received map information from server <B>\n");
		memcpy(&graph_msg_b, buf_graph, sizeof(graph_info));
		int num_edges = graph_msg_b.num_edges;
		for(int i = 0; i < num_edges; i++) {
			memset(&buf_edge, 0, sizeof(buf_edge));
			memset(&_edge, 0, sizeof(_edge));
			edge temp;
			recvfrom(client_sockfd_B,buf_edge,BUFSIZ,0,(struct sockaddr *)&remote_addr_B, &client_size);
			memcpy(&_edge, buf_edge, sizeof(graph_info));
			temp = _edge;
			edge_msg_listb.push_back(temp);
			s.insert(temp.start);
			s.insert(temp.end);	
			if(s.find(start_idx) != s.end()) {
				result.start_existed = true;
			}
			if(s.find(end_idx) != s.end()) {
				result.end_existed = true;
			}
		}
	}
	close(client_sockfd_B);
	return 0;
}
int udp_C(graph_info &graph, vector<edge> &v) {

	int client_sockfd_C;
	int num_edegs = graph.num_edges;
	int len_C;
	struct sockaddr_in remote_addr_C; //服务器端网络地址结构体
	socklen_t sin_size_C;
	char buf_map[BUFSIZ];
	char buf_graph[BUFSIZ]; 
	char buf_edge[BUFSIZ]; //数据传送的缓冲区
	char buf_c_to_aws[BUFSIZ];

	memset(&buf_map, 0, sizeof(buf_map));
	memset(&buf_graph, 0, sizeof(buf_graph));
	memset(&buf_edge, 0, sizeof(buf_edge));
	memset(&remote_addr_C,0,sizeof(remote_addr_C)); //数据初始化--清零
	memset(&buf_c_to_aws, 0, sizeof(buf_c_to_aws));

	remote_addr_C.sin_family=AF_INET; //设置为IP通信
	remote_addr_C.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址
	remote_addr_C.sin_port=htons(UDPC_PORT); //服务器端口号
	sin_size_C = sizeof(struct sockaddr_in);

	//create socket to serverC
	if((client_sockfd_C=socket(AF_INET,SOCK_DGRAM,0))<0)
	{  
		perror("socket error");
		return 1;
	}
	sin_size_C = sizeof(struct sockaddr_in);
	
	//send to serverC
	// send map_id, start, end, file_size
	memcpy(buf_map, &recv_file, sizeof(buf_map));
	sendto(client_sockfd_C,buf_map,BUFSIZ,0,(struct sockaddr *)&remote_addr_C,sizeof(struct sockaddr));
	
	// send trans_speed, prop_speed, num_edges
	memcpy(buf_graph, &graph, sizeof(buf_graph));
	sendto(client_sockfd_C,buf_graph,BUFSIZ,0,(struct sockaddr *)&remote_addr_C,sizeof(struct sockaddr));

	// send all edges to serverC
	for(auto iter = v.begin(); iter != v.end(); iter++) {
		memset(buf_edge, 0, sizeof(buf_edge));
		edge temp = *iter;
		// printf("%d\t%d\t%f\n", temp.start, temp.end, temp.dis);
		memcpy(buf_edge, &temp, sizeof(buf_edge));
		sendto(client_sockfd_C,buf_edge,BUFSIZ,0,(struct sockaddr *)&remote_addr_C,sizeof(struct sockaddr));
	}
	printf("The AWS has sent map, source ID, destination ID, \n");
	printf("propagation speed and transmission speed to server C using UDP over port <%d>\n" , UDP_PORT);
	// receive result from serverC
	recvfrom(client_sockfd_C,buf_c_to_aws,BUFSIZ,0,(struct sockaddr *)&remote_addr_C,&sin_size_C);
	memcpy(&result, buf_c_to_aws, sizeof(buf_c_to_aws));
	printf("The AWS has received results from server C:\n");
	printf("Shortest path: ");
	int route_len = result.len;
	for(int i = 0 ;i < route_len - 1; i++) {
		printf("<%d> -- ", result.route[i]);
	}
	printf("<%d>", result.route[route_len - 1]);
	printf("\n");
	printf("Shortest distance: <%.2f> km \n", result.res_dis);
	printf("Transmission delay: <%.2f> s \n", result.tt);
	printf("Propagation delay: <%.2f> s \n", result.tp);
	// close udp connection
	close(client_sockfd_C);
	return 0;
}

int main(int argc, char *argv[])
{//TCP
	int server_sockfd_tcp;//服务器端套接字
	int client_sockfd_tcp;//客户端套接字
	int len;
	struct sockaddr_in my_addr;   //服务器网络地址结构体
	struct sockaddr_in remote_addr; //客户端网络地址结构体
	socklen_t sin_size;
	char buf[BUFSIZ];  
	graph_info graph;
	vector<edge> v;

	memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零
	my_addr.sin_family=AF_INET; //设置为IP通信
	my_addr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上
	my_addr.sin_port=htons(TCP_PORT); //服务器端口号
	sin_size=sizeof(struct sockaddr_in);

	// create socket
	if((server_sockfd_tcp=socket(PF_INET,SOCK_STREAM,0))<0)
	{  
		perror("socket error");
		return 1;
	}
	// bind 
	if(::bind(server_sockfd_tcp,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)
	{
		perror("bind error");
		return 1;
	}
	//listen
	if(listen(server_sockfd_tcp,5)<0)
	{
		perror("listen error");
		return 1;
	};
	printf("The AWS is up and running. \n");
	while(1) {
		if((client_sockfd_tcp=accept(server_sockfd_tcp,(struct sockaddr *)&remote_addr,&sin_size))<0)
		{
			perror("accept error");
			return 1;
		}
		recv(client_sockfd_tcp,buf,BUFSIZ,0);
        memset(&recv_file, 0, sizeof(recv_file));
        memcpy(&recv_file, buf, sizeof(buf));
		memset(&result, 0 ,sizeof(result));
		
		printf("The AWS has received map ID <%c>, start vertex <%d>,\n", recv_file.map_id, recv_file.start_idx);
		printf("destination vertex <%d> and file size <%d> from the client using TCP over port <%d> \n", recv_file.end_idx, recv_file.file_size, TCP_PORT);
		
		printf("The AWS has sent map ID to server A using UDP over port <%d> \n", UDP_PORT);
		udp_A();
		
		// printf("The AWS has sent map ID to server B using UDP over port <%d> \n", UDP_PORT);
		// udp_B();
		if(head_msg_a.exist) {
			result.map_existed = true;
			graph = graph_msg_a;
			v = edge_msg_lista;
		} else{
			printf("The AWS has sent map ID to server B using UDP over port <%d> \n", UDP_PORT);
			udp_B();
		}
		if(head_msg_b.exist){
			result.map_existed = true;
			graph = graph_msg_b;
			v = edge_msg_listb;
		}
		if(result.map_existed) {
			if(result.end_existed && result.start_existed) {
				udp_C(graph, v);
				printf("The AWS has sent calculated results to client using TCP over port <%d> \n", TCP_PORT);
				result.map_existed = true;
				result.start_existed = true;
				result.end_existed = true;
			}
		}
		char sendbuf[BUFSIZ];
		memcpy(sendbuf, &result, sizeof(sendbuf));
		send(client_sockfd_tcp,sendbuf,BUFSIZ,0);
		printf("-------------------------------------------\n");
	}
	/*关闭套接字*/
	close(client_sockfd_tcp);
	close(server_sockfd_tcp);
    return 0;
}


// for(auto e : v) {
// 	printf("%d\t%d\t%f\n", e.start, e.end, e.dis);
// }