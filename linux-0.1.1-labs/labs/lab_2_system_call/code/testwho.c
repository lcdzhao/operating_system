#include <who.h>
#include <stdio.h>

int main()
{
	char * name = "Without";
	printf("call iam\r\n");
	iam(name,7);
	printf("call iam end\r\n");
	printf("call whoami\r\n");
	whoami(7);
	printf("call whoami end\r\n");
	return 0;
}
