#include <stdio.h>
#include <string.h>
#define BUF_LEN 128

int main(int argc, char const *argv[])
{
	// Encode file from argv[0] and save to file argv[1]
	if(argc != 4)
	{
		printf("Usage : ./encoder <text_file> <encrypted_file> <key_file> \n");
		return -1;
	}

	FILE * inFile;
	FILE * encFile;
	FILE * keyFile;
	keyFile = fopen(argv[3], "w");
	if(!(keyFile != NULL))
	{
		printf("Invalid key file !!!\n");
		return -1;
	}
	inFile = fopen(argv[1], "r");
	if(!(inFile != NULL))
	{
		printf("Invalid input file !!!\n");
		fclose(keyFile);
		return -1;
	}	
	encFile = fopen(argv[2], "w");
	if(!(encFile != NULL))
	{
		printf("Invalid encoding file !!!\n");
		fclose(keyFile);
		fclose(inFile);
		return -1;
	}

	FILE* dev;
	dev = fopen("/dev/encdev","r+");
	if(!(dev != NULL))
	{
		printf("Cannot open device. Check if the driver is installed correctly !\n");
		fclose(keyFile);
		fclose(inFile);
		fclose(encFile);
		return -1;
	}

	/* actual code for encoder */

	/* Make a Pre Shared key */
	FILE* random;
	random = fopen("/dev/urandom", "r");

	char psk[BUF_LEN+1];
	fread(psk,1,BUF_LEN,random);
	fclose(random);

	fwrite(psk,1,BUF_LEN,keyFile);
	/* Write the Pre Shared key and save it */

	/* First write to device */
	fwrite(psk,1,BUF_LEN,dev);
	// printf("%s\n", psk);
	/* Write complete, now subsequent writes will be DATA */

	size_t dataS = 0;
	size_t dataDriver = 0;

	do
	{
		dataS = fread(psk,1,BUF_LEN,inFile);
		psk[BUF_LEN] = '\0';
		// printf("%s\n%lu\n", psk, dataS);
		if(dataS == 0)
		{
			break;
		}
		if(dataS < BUF_LEN)
		{
			// psk[dataS] = '\0';
			for(; dataS<BUF_LEN; dataS++)
			{
				psk[dataS] = ' ';
			}
		}

		fwrite(psk,1,BUF_LEN,dev);
		dataDriver = fread(psk,1,BUF_LEN,dev);
		fwrite(psk,1,dataDriver,encFile);
		printf("%zu\n", dataS);
	}while(dataS == BUF_LEN);

	fclose(dev);
	fclose(keyFile);
	fclose(inFile);
	fclose(encFile);
	return 0;
}