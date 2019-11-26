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

struct delay_info{
    
    int file_size;

	int tran_speed;

	int prop_speed;

	int num_vertix;

};

struct result_from_serverB {

	double tt;

	double tp[100];

};