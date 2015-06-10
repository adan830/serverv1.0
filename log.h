/*************************************************************************
	> File Name: log.h
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Tue 02 Jun 2015 08:55:38 PM CST
 ************************************************************************/
#ifndef _LOG_H_
#define	_LOG_H_

#include <deque>
#include <string>
#include <semaphore.h>
#include <stdio.h>
#include <pthread.h>
using namespace std;

class LOG
{
	public:
		LOG();
		void append(string str);
		void writebuf(string str);
		void write();
		void run();
	private:
		deque<string> buf, buf1;
		bool flag;
	public:
		sem_t sem;
	private:
		bool isWriting;
		FILE *fp;
};

static void *thread(void *arg);

#endif
