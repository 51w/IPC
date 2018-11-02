#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>

int main()
{
	printf("...\n");
	fflush(NULL);
	sync();
	sleep(1);
	reboot(RB_AUTOBOOT);
	
	return 0;
}
