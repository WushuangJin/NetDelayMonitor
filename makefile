all: serverA.cpp serverB.cpp client.cpp aws.cpp
	  g++ -o serverA serverA.cpp
	  g++ -o serverB serverB.cpp
	  g++ -o client client.cpp
	  g++ -o aws aws.cpp

serverA: serverA.cpp
			./serverA

serverB: serverB.cpp
			./serverB

aws: aws.cpp
			./aws

