all: serverA.cpp serverB.cpp client.cpp aws.cpp
	g++ -o serverA.o serverA.cpp
	g++ -o serverB.o serverB.cpp
	g++ -o client client.cpp
	g++ -o aws.o aws.cpp

serverA: serverA.o
	./serverA.o

serverB: serverB.o
	./serverB.o

aws: aws.o
	./aws.o
