#include <stdio.h>
#include <process.h>
#include <windows.h>


int count=0;

void thread1(void* pvoid)
{
	int i;
	for(i=1;i<1000000;i++)
       count ++;
}

void thread2(void* pvoid)
{
	int j;
	for(j=1;j<1000000;j++)
       count ++;
}

int main()
{
	
	_beginthread(thread1,0,NULL);
	_beginthread(thread2,0,NULL);

	_sleep(1000);
	printf("%d\n",count);

	return 0;
}
