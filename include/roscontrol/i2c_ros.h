#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port
#include <cstdio>
#include <stdint.h> 



unsigned int microsecond = 1000000;
int file_i2c;
int length;
unsigned char buffer[60] = {0};
void  motordeclare();
void buffwrite(char LRcheck , int vel);
double testneg (uint16_t testval) ;
void buffread(double& lefty , double& righty,double& batterystatus);


void  motordeclare()
{
	//----- OPEN THE I2C BUS -----
	char *filename = (char*)"/dev/i2c-1";
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus");
	}
	
	int addr = 0x08;          //<<<<<The I2C address of the slave
	if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
	}
}

	

void buffwrite(char LRcheck , int vel)
{	
	//----- WRITE BYTES -----
	//int velocity = 50;
	buffer[0]= LRcheck;
	buffer[1] = vel;
	buffer[2] = vel >> 8;

	
	length = 3;			//<<< Number of bytes to write
	printf("Writing.\n");
	if (write(file_i2c, buffer, length) != length)		
	{
		/* ERROR HANDLING: i2c transaction failed */
		printf("Failed to write to the i2c bus.\n");
	}

}



void buffread(double& lefty , double& righty,double& batterystatus)
{
		//----- READ BYTES -----
	length = 6;			//<<< Number of bytes to read
	uint8_t posbuff[length];
	if (read(file_i2c, posbuff, length) != length)	
	{
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	else
	{
		 lefty = testneg((uint16_t)((posbuff[1] << 8 )|posbuff[0]));
		 righty = testneg((uint16_t)((posbuff[3] << 8 )|posbuff[2]));
		 batterystatus = ((posbuff[5] << 8 )|posbuff[4]);
		//printf("Data read on encod1: %d \n", testneg(x));
		//printf("Data read on encod2: %d \n", testneg(y));	
	}
}


double testneg (uint16_t testval) {
	const int negative = (testval & (1 << (sizeof(testval)*8-1))) != 0;

	if (negative)
	return (double)(testval | ~((1 << (sizeof(testval)*8)) - 1));
	else
	return (double)(testval);
}

