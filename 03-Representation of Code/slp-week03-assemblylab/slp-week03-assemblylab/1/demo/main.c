//====================================================================
//程序1：一个空程序
//====================================================================

#include <stdio.h>


//int main ()
//{
//	printf("null\n");
//
//	return 0;
//}




//====================================================================
//程序2：一个只有赋值语句的程序
//====================================================================

//#include <stdio.h>
//
//int a;
//
//int main ()
//{
//
//	a=5;
//
//	printf("null\n");
//
//    return 0;
//}



//====================================================================
//程序3：一个打印 a=5对应指令的程序：from SSD6.chm文件的1.5.1.1
//====================================================================

//#include <stdio.h>
//
//int a;
//int main (int argc, char *argv[])
//{
//	int i;
//	unsigned char * c;
//
//	a = 5;
//
//	//Notes by 教师：这个数值可能需要修改，according to你的运行情况
//
//	//c = ((unsigned char *) 0x00401028);
//	c = ((unsigned char *)0x0040B768);
//
//	for (i = 0; i < 10; i++)
//	{
//		printf("%02X ", *c);
//		c++;
//	}
//
//	printf("\n");
//
//    return 0;
//}



//====================================================================
//程序4：一个带有算术运算的程序：from SSD6.chm文件的1.5.3.2
//====================================================================

//#include <stdio.h>
//
//int a;
//int b = 4;
//int c = 3;
//
//int main ( int argc, char * argv[])
//{
//
//	a = b + 3 * c;
//
//	printf("\n");
//	return 0;
//}



//====================================================================
//程序5：一个带有位运算的程序：
//====================================================================

//#include <stdio.h>
//
//int a =4;
//int b =5;
//int c =0;
//
//int main ( int argc, char * argv[])
//{
//
//	c = a&b;
//
//	printf("\n");
//	return 0;
//}



//====================================================================
//程序6：一个带有逻辑运算的程序：
//====================================================================

#include <stdio.h>


int a =4;
int b =5;
int c =0;

int main ( int argc, char * argv[])
{

	c = a&&b;

  	printf("\n");
	return 0;
}


//====================================================================
//程序7：一个if分支的程序
//====================================================================
/*
#include <stdio.h>


int a;

int main ( int argc, char * argv[])
{

	a = 5;

	if (a>0)
	{
		printf("a>0 \n");
	}

  	return 0;
}
*/

//====================================================================
//程序8：一个if分支的程序
//====================================================================
/*
#include <stdio.h>


int a;

int main ( int argc, char * argv[])
{

	a = 5;

	if (a>3)
	{
		printf("a>3 \n");
	}

  	return 0;
}
*/


//====================================================================
//程序9：一个if-else分支的程序
//====================================================================
/*
#include <stdio.h>


int a;

int main ( int argc, char * argv[])
{

	a = 5;

	if (a>0)
	{
		printf("a>0 \n");
	}
	else
	{
		printf("a<0 \n");
	}

  	return 0;
}
*/


//====================================================================
//程序10：一个switch分支的程序
//====================================================================
/*
#include <stdio.h>

int a;

int main ( int argc, char * argv[])
{

	a = 2;
	switch (a)
	{
		case 1:
			a=1;
			break;

		case 2:
			a=2;

		case 3:
			a=3;
			break;

		default:
			a=5;
	}

  	return 0;
}
*/




//====================================================================
//程序11：一个while循环的程序
//====================================================================
/*
#include <stdio.h>

int a;

int main ( int argc, char * argv[])
{

	int b;
	a = 5;

	while(a>0)
	{
		b++;
		a--;
	}

  	return 0;
}
*/

//====================================================================
//程序12：一个do-while循环的程序
//====================================================================
/*
#include <stdio.h>

int a;

int main ( int argc, char * argv[])
{

	int b;
	a = 5;

	do
	{
		b++;
		a--;
	}while(a>0);

  	return 0;

}
*/



//====================================================================
//程序13：一个for循环的程序
//====================================================================
/*
#include <stdio.h>

int a;
int b;

int main ( int argc, char * argv[])
{

	b=0;

	for(a=5;a>0;a--)
	{
		b++;
	}

  	return 0;
}
*/