all: serverA.cpp serverB.cpp serverC.cpp client.cpp aws.cpp
	g++ -std=c++11 -o client client.cpp
	g++ -std=c++11 -o aws aws.cpp
	g++ -std=c++11 -o serverA serverA.cpp
	g++ -std=c++11 -o serverB serverB.cpp
	g++ -std=c++11 -o serverC serverC.cpp

serverA: serverA.o
	./serverA.o

serverB: serverB.o
	./serverB.o

serverC: serverC.o
	./serverC.o

aws: aws.o
	./aws.o