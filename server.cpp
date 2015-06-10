#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>
#include <mutex>
#include <deque>
#include <semaphore.h>

#include "log.h"

using namespace std;
using namespace boost;

#define MAX_EVENT_NUMBER 1024
#define TCP_BUFFER_SIZE 512
#define UDP_BUFFER_SIZE 1024
#define MAX_CONN_NUM 2

mutex m;
deque<int> actSockets;
sem_t sem;
deque<int> closingSockets;
pthread_mutex_t closingSocketLock;
int maxConnNum = 0;
pthread_mutex_t alock;         
int num = 0;

int sum1 = 0, sum2 = 0;

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd( int epollfd, int fd, bool oneshot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if(oneshot)
	{
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
	setnonblocking( fd );
}

void reset_oneshot(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void process(int sockfd)
{
	char buf[ TCP_BUFFER_SIZE ];
	memset( buf, '\0', TCP_BUFFER_SIZE );
	int	ret = recv( sockfd, buf, TCP_BUFFER_SIZE-1, 0 );
	if( ret < 0 )
	{
		if( ( errno == EAGAIN ))
		{
			cout << "error == EAGAIN OR EWOULDBLOCK" << endl;
		}
		else if(errno == EWOULDBLOCK)
		{
			cout << "error == EWOULDBLOCK" << endl; 
		}
	}
	else if( ret == 0 )
	{
		close(sockfd);
	}
	else
	{
		send( sockfd, buf, ret, 0 );
	}
}


void *thread(void *arg)
{
	int *i = (int *)arg;
	while(1)
	{
		if (*i == 1)
		{
			sum1++;
		}
		else
		{
			sum2++;
		}
		if (sum1 + sum2 == 1000)
		{
			cout << "1000" << endl;
		}
		sem_wait(&sem);
		pthread_mutex_lock(&alock);
		int sockfd = actSockets.front();
		actSockets.pop_front();
		pthread_mutex_unlock(&alock);
		process(sockfd);		
	}
}

int main( int argc, char* argv[] )
{
	if( argc <= 2 )
	{
		printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
		return 1;
	}

	sem_init(&sem, 0, 0);

	pthread_mutex_init(&alock,NULL);
	pthread_mutex_init(&closingSocketLock, NULL);

	int arg1 = 1;
	pthread_t th;
	int *thread_ret = NULL;
	int ret = pthread_create( &th, NULL, thread, &arg1 );  
	if( ret != 0 ){  
		printf( "Create thread error!\n");  
		return -1; 
	}  

	int arg2 = 2;
	ret = pthread_create( &th, NULL, thread, &arg2 );  
	if( ret != 0 ){ 
		printf( "Create thread error!\n"); 
		return -1; 
	} 

	const char* ip = argv[1];
	int port = atoi( argv[2] );

	struct sockaddr_in address;
	bzero( &address, sizeof( address ) );
	address.sin_family = AF_INET;
	inet_pton( AF_INET, ip, &address.sin_addr );
	address.sin_port = htons( port );

	int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
	assert( listenfd >= 0 );

	ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
	assert( ret != -1 );

	ret = listen( listenfd, 2000 );
	assert( ret != -1 );

	epoll_event events[ MAX_EVENT_NUMBER ];
	int epollfd = epoll_create( 5 );
	assert( epollfd != -1 );
	addfd( epollfd, listenfd, false);

	while( 1 )
	{
		int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
		if ( number < 0 )
		{
			printf( "epoll failure\n" );
			break;
		}

		for ( int i = 0; i < number; i++ )
		{
			int sockfd = events[i].data.fd;
			if ( sockfd == listenfd )
			{
				while(1)
				{
					struct sockaddr_in client_address;
					socklen_t client_addrlength = sizeof( client_address );
					int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
					if(connfd > 0)
					{
						num++;
						if(num % 100 == 0)
						{
							cout << num << endl;
							cout << sum1 << " " << sum2 << endl;
						}
						addfd( epollfd, connfd, true);
					}
					else if ((connfd == -1) && (errno == EAGAIN))
					{
						break;
					}
					else
					{
						cout << "confd == 0" << endl;
					}
				}
			}
			else if ( events[i].events & EPOLLIN )
			{
				pthread_mutex_lock(&alock);
				actSockets.push_back(sockfd);
				pthread_mutex_unlock(&alock);
				sem_post(&sem);
			}
			else
			{
				printf( "something else happened \n" );
			}
		}
	}

	pthread_join( th, (void**)&thread_ret ); 
	close( listenfd );
	return 0;
}
