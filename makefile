all: serverA.cpp serverB.cpp client.cpp aws.cpp
	g++ -std=c++11 -o serverA.o serverA.cpp
	g++ -std=c++11 -o serverB.o serverB.cpp
	g++ -std=c++11 -o client client.cpp
	g++ -std=c++11 -o aws.o aws.cpp

serverA: serverA.o
	./serverA.o

serverB: serverB.o
	./serverB.o

aws: aws.o
	./aws.o
