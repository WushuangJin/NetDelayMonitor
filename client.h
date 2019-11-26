#include<string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>

using namespace std;
struct trans_info{

	char map_idx[1];

	int start_idx;

	int size;

};

struct recv_tcp_info {
	int num;
	int dest[20];
	int dis[20];
	double tp[20];
	double delay[20];
	double tt[20];
};