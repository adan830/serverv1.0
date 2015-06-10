/*************************************************************************
  > File Name: log.cpp
  > Author: ma6174
  > Mail: ma6174@163.com 
  > Created Time: Mon 01 Jun 2015 09:03:51 PM CST
 ************************************************************************/
#include "log.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

LOG::LOG()
{

	sem_init(&sem, 0, 0);
	flag = true;
	isWriting = false;
	fp = fopen("log.txt", "a+");
	if(fp == NULL)
	{
		printf("fopen failed\n");
	}
}

void LOG::append(string str)
{
	printf("LOG::append\n");
	if(!flag)
	{
		printf("buf\n");
		buf.push_back(str);
		if(buf.size() > 5 && !isWriting)
		{
			flag = true;
			isWriting = true;
			sem_post(&sem);
		}
	}
	else
	{
		printf("buf1\n");
		buf1.push_back(str);
		if(buf1.size() > 5 && !isWriting)
		{
			flag = false;
			isWriting = true;
			sem_post(&sem);
		}
	}
}

void LOG::writebuf(string str)
{
	printf("%s\n", str.c_str());
	if(fputs(str.c_str(), fp) == EOF)
	{
		perror("error : ");
		printf("fput failed\n");
	}
}

void LOG::write()
{
	if(flag)
	{
		while(!buf.empty())
		{
			writebuf("buf\n");
			buf.pop_front();
		}
	}
	else
	{
		while(!buf1.empty())
		{
			writebuf("buf1\n");
			buf1.pop_front();
		}
	}
	sync();
	isWriting = false;
}

static void *thread(void *arg)
{
	while(1)
	{
		LOG *log = (LOG *)arg;
		sem_wait(&log->sem);
		printf("log->write()\n");
		log->write();
	}
}
/*
static void testtime()
{
	time_t t;
	struct tm *tm1;
	time(&t);
	tm1 = localtime(&t);
	printf("%d-%d-%d %d:%d:%d\n", 1900 + tm1->tm_year, 1 + tm1->tm_mon, tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
//	printf("%s", asctime(tm1));
}

static void testlog()
{
		LOG log;
	string str("hello world!\n");
	int arg1 = 1;
	pthread_t th;
	int *thread_ret = NULL;
	int ret = pthread_create( &th, NULL, thread, &log );  
	if( ret != 0 ){  
		printf( "Create thread error!\n");  
	} 

//for(int i = 0; i < 5; ++i)
	while(1)
	{
		printf("append log\n");
		for(int i = 0; i < 10; ++i)
		{
			log.append(str);
		}
	//	sleep(2);
	}

//	pthread_join( th, (void**)&thread_ret ); 
}*/
/*
int main()
{
	testtime();

	return 0;
}
*/
