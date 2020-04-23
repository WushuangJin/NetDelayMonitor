#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include <vector>
#include <iostream>
#include "serverC.h"

#define TCP_PORT 34534
using namespace std;

void parseArgs(vector<string> &args, send_info &send_file) {
    if(args.size() != 5) {
        printf("please input correct query !");
        exit(0);
    }
    auto iter = args.begin();
    iter++;
    int idx = 0;
    for(; iter!=args.end(); iter++) {
        string curr = *iter;
        if(idx == 0) {
            char temp[1] = ""; 
            strcpy(temp, curr.c_str());
            send_file.map_id = temp[0];
        } else if (idx == 1) {
            send_file.start_idx = stoi(curr);
        } else if (idx == 2) {
            send_file.end_idx = stoi(curr);
        } else if (idx == 3) {
            send_file.file_size = stoi(curr);
        }
        idx++;
    }
}

int main(int argc, char** argv)
{
	int client_sockfd;
	int len;
	struct sockaddr_in remote_addr; //服务器端网络地址结构体
	char buf[BUFSIZ];  //数据传送的缓冲区
	memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零
	remote_addr.sin_family=AF_INET; //设置为IP通信
	remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址
	remote_addr.sin_port=htons(TCP_PORT); //服务器端口号
	
	/*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
	if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		perror("client created socket error");
		return 1;
	}
	
	/*将套接字绑定到服务器的网络地址上*/
	if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
	{
		perror("client connect error");
		return 1;
	}
	printf("The client is up and running \n");
		// printf("Enter string to send: \n");
	vector<string> args(argv, argv + argc);
	send_info send_file;
	memset(&send_file, 0, sizeof(send_file));
	parseArgs(args, send_file);
	memset(&buf, 0, sizeof(buf));
	memcpy(buf, &send_file, sizeof(send_file)); //结构体转换成字符串
	len=send(client_sockfd, buf, sizeof(buf),0);
	printf("The client has sent query to AWS using TCP: start vertex <%d>; \n", send_file.start_idx);
	printf("destination vertex <%d>, map <%c>; file size <%d> \n", send_file.end_idx,send_file.map_id,send_file.file_size);
	printf("The client has received results from AWS: \n");
	printf("------------------------------------------------------ \n");
	char recvBuf[BUFSIZ];
	msg_c recv_file;
	memset(recvBuf, 0, sizeof(recvBuf));
	memset(&recv_file, 0, sizeof(recv_file));
	recv(client_sockfd,recvBuf,BUFSIZ,0);
	memcpy(&recv_file, recvBuf, sizeof(msg_c));


	if(!recv_file.map_existed) {
		printf("No map id <%c> found \n", send_file.map_id);
	} else if(!recv_file.start_existed || !recv_file.end_existed) {
		if(!recv_file.start_existed && !recv_file.end_existed) {
			printf("No vertex id <%d> and <%d> \n", send_file.start_idx, send_file.end_idx);
		} else if(!recv_file.start_existed) {
			printf("No vertex id <%d> \n ", send_file.start_idx);
		} else {
			printf("No vertex id <%d> \n ", send_file.end_idx);
		}
	} else {
		printf("Source Destination\tMin Length\tTt\tTp\tDelay\n");
		printf("------------------------------------------------------ \n");
		printf("%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\n",send_file.start_idx,send_file.end_idx,recv_file.res_dis,recv_file.tt,recv_file.tp,recv_file.total);
		printf("------------------------------------------------------ \n");
		printf("Shortest path: ");
		int route_len = recv_file.len;
		for(int i = 0 ;i < route_len - 1; i++) {
			printf("<%d> -- ", recv_file.route[i]);
		}
		printf("<%d>\n", recv_file.route[route_len - 1]);
	}
		/*关闭套接字*/
	close(client_sockfd);
    
	return 0;
}
