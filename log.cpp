/*************************************************************************
  > File Name: log.cpp
  > Author: ma6174
  > Mail: ma6174@163.com 
  > Created Time: Mon 01 Jun 2015 09:03:51 PM CST
 ************************************************************************/
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
int main()
{

	struct timeval start;
	struct tm ptr = {0};
	gettimeofday(&start, NULL);
	localtime_r(&start.tv_sec, &ptr);
	char s[50];
	strftime(s, 50, "%F-%H-%M-%S", &ptr);
	printf("%s\n", s);
	FILE *fp = fopen("log.txt", "a+");
	if(fp == NULL)
	{
		printf("fopen failed\n");
		return 0;
	}

	char *buf = "hello world!\n";
	for(int i = 0; i < 5; ++i)
		if(fputs(buf, fp) == EOF)
		{
			perror("error : ");
			printf("fput failed\n");
		}

	return 0;
}
