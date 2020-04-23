#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "serverA.h" 
#include "serverC.h"
#include <queue>
#include <vector>
#include "client.h"
#include <algorithm>


using namespace std;
#define UDPC_PORT 32534
send_info msg_from_aws;
graph_info graph_msg;
vector<edge> v;

class node {
	public :
		int prev;
		int curr;
		float dis;
		node(int prev, int curr, float dis): prev(prev), curr(curr), dis(dis) {};
};

struct cmp {
	bool operator()(node a, node b) {
		return a.dis > b.dis;
	}
};

void build_graph(unordered_map<int, vector<pair<int,float>>> &graph, vector<edge> &v) {
	for(auto e : v) {
		int start = e.start;
		int end = e.end;
		float dis = e.dis;
		graph[start].push_back(make_pair(end, dis));
	}
};

float compute_path(int start, int end, unordered_map<int, vector<pair<int,float>>> &graph, vector<int> &path) {
	// int n = graph.size();
	// initinalize the visited arrary as -1 which means unvisited
	unordered_map<int, int> visited;
	priority_queue<node, vector<node>, cmp> pq;
	pq.push(node(-1, start, 0.00));
	while(!pq.empty()) {
		node curr_node = pq.top();
		pq.pop();
		// curr_idx means which node we are now visiting
		int curr_idx = curr_node.curr;
		float curr_dis = curr_node.dis;
		int prev_idx = curr_node.prev;
		if(visited.find(curr_idx) != visited.end())
			continue;
		visited[curr_idx] = prev_idx;
		if(curr_idx == end) {
			while(curr_idx != -1) {
				path.push_back(curr_idx);
				curr_idx = visited[curr_idx];
			}
			return curr_dis;
		}
		vector<pair<int, float>> v = graph[curr_idx];
		for(auto next : v) {
			int next_idx = next.first;
			float edge_len = next.second;
			float new_dis = curr_dis + edge_len;
			pq.push(node(curr_idx, next_idx, new_dis));
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int server_sockfd, num_edges, len;
	struct sockaddr_in my_addr;   //服务器网络地址结构体
    struct sockaddr_in remote_addr; //客户端网络地址结构体
	socklen_t sin_size;
	char buf_file[BUFSIZ];
	char buf_graph[BUFSIZ]; 
	char buf_node[BUFSIZ]; 
	//数据传送的缓冲区
	memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零
	my_addr.sin_family=AF_INET; //设置为IP通信
	my_addr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上
	my_addr.sin_port=htons(UDPC_PORT); //服务器端口号
	
	memset(&msg_from_aws, 0, sizeof(msg_from_aws));
	memset(&graph_msg, 0, sizeof(graph_msg));
	memset(&v, 0, sizeof(v));

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

	printf("The Server C is up and running using UDP on port <%d> \n", UDPC_PORT);
	sin_size=sizeof(struct sockaddr_in);

	while(1) {
		if((len=recvfrom(server_sockfd,buf_file,BUFSIZ,0,(struct sockaddr *)&remote_addr,&sin_size))>0) {
			memcpy(&msg_from_aws, buf_file, sizeof(send_info));
			// printf("map_id :  %c \n", msg_from_aws.map_id);
			// printf("start_idx : %d \n", msg_from_aws.start_idx);
			// printf("end_idx : %d \n", msg_from_aws.end_idx);
			// printf("file_size : %d \n", msg_from_aws.file_size);
		}
		
		if((len=recvfrom(server_sockfd,buf_graph,BUFSIZ,0,(struct sockaddr *)&remote_addr,&sin_size))>0) {
			memcpy(&graph_msg, buf_graph, sizeof(graph_info));
			// printf("prop_speed is : %f \n", graph_msg.prop_speed);
			// printf("tran_speed is : %d \n", graph_msg.trans_speed);
		}
		int num_edegs = graph_msg.num_edges;
		for(int i = 0; i < num_edegs; i++) {
			memset(&buf_node, 0, sizeof(buf_node));
			edge temp;
			recvfrom(server_sockfd,buf_node,BUFSIZ,0,(struct sockaddr *)&remote_addr,&sin_size);
			memcpy(&temp, buf_node, sizeof(edge));
			v.push_back(temp);
		}
		// for(auto e : v) {
		// 	printf("%d\t%d\t%f\n", e.start, e.end, e.dis);
		// }
		char map_id = msg_from_aws.map_id;
		int start = msg_from_aws.start_idx;
		int end = msg_from_aws.end_idx;
		int file_size = msg_from_aws.file_size;
		int trans_speed = graph_msg.trans_speed;
		float prop_speed = graph_msg.prop_speed;
		
		printf("The Server C has received data for calculation: \n");
		printf(" * Propagation speed: <%.2f> km/s \n", prop_speed);
		printf(" * Transmission speed <%d> KB/s \n", trans_speed);
		printf(" * map ID: <%c> \n", map_id);
		printf("* Source ID: <%d>    Destination ID: <%d> \n", start, end);

		unordered_map<int, vector<pair<int,float>>> city_graph;
		build_graph(city_graph, v);
		vector<int> path;
		msg_c result;
		float res_dis;
		res_dis = compute_path(start, end, city_graph, path);

		reverse(path.begin(),path.end());
		printf("The Server C has finished the calculation: \n");
		printf("Shortest path: ");
		result.tp = res_dis / prop_speed;
		result.tt = file_size / (float)trans_speed;
		result.total = result.tp + result.tt;
		result.res_dis = res_dis;
		result.len = path.size();
		for(auto i = 0; i < path.size() - 1; i++) {
			result.route[i] = path[i];
			printf("<%d> -- ", path[i]);
		}
			result.route[path.size() - 1] = path[path.size() - 1];
			printf("<%d> ", path[path.size() - 1]);
		printf("\n");
		printf("Shortest distance: <%.2f> km \n", result.res_dis);
       	printf("Transmission delay: <%.2f> s \n", result.tt);
       	printf("Propagation delay: <%.2f> s \n", result.tp);

		char buf_to_aws[BUFSIZ];
		memset(&buf_to_aws, 0, sizeof(buf_to_aws));
		memcpy(buf_to_aws, &result, sizeof(msg_c));
		sendto(server_sockfd,buf_to_aws,BUFSIZ,0,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr));
		printf("The Server C has finished sending the output to AWS \n");
		printf("-------------------------------------------\n");
	}
	/*关闭套接字*/
	close(server_sockfd);
}


