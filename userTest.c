#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
	int valueFromKernel; //value will get from module that has been loaded in kernel
	int fd;//file descriptor
	
//	fd=open("/dev/randnumber", O_RDONLY); //open device in kernel with mode read only
	
	openat(fd, "/dev/randnumber", O_RDONLY);	

	if (fd==-1) //failed when open file device
	{
		 perror("Acess device failed!\n");
        	return -1;
	}
	
	read(fd,&valueFromKernel,sizeof(valueFromKernel)); //max size of a number is 4 bytes
	
	printf("Rand Number is: %d\n",valueFromKernel);
	
	printf("Finish task!\n");
	close(fd);
		
	return 0;
}
