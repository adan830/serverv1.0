server : server.o log.o
	g++ -std=c++11 -o server server.o log.o -lpthread

log.o : log.h log.cpp
	g++ -c log.cpp log.h 

server.o : server.cpp log.h
	g++ -std=c++11 -c  server.cpp log.h 

