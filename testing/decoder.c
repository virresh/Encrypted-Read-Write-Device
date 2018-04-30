#include <stdio.h>
#include <string.h>
#define BUF_LEN 16

int main(int argc, char const *argv[])
{
	// Decode file from argv[0] and save to file argv[1]
	if(argc != 4)
	{
		printf("Usage : ./encoder <encrypted_file> <decrypted_file> <key_file> \n");
		return -1;
	}

	FILE * outFile;
	FILE * encFile;
	FILE * keyFile;
	keyFile = fopen(argv[3], "r");
	if(!(keyFile != NULL))
	{
		printf("Invalid key file !!!\n");
		return -1;
	}
	outFile = fopen(argv[2], "w");
	if(!(outFile != NULL))
	{
		printf("Invalid output file !!!\n");
		fclose(keyFile);
		return -1;
	}	
	encFile = fopen(argv[1], "r");
	if(!(encFile != NULL))
	{
		printf("Invalid encoding file !!!\n");
		fclose(keyFile);
		fclose(outFile);
		return -1;
	}

	FILE* dev;
	dev = fopen("/dev/decdev","r+");
	if(!(dev != NULL))
	{
		printf("Cannot open device. Check if the driver is installed correctly !\n");
		fclose(keyFile);
		fclose(outFile);
		fclose(encFile);
		return -1;
	}

	/* actual code for encoder */

	/* Read the Pre Shared key */
	char psk[BUF_LEN+1];
	fread(psk,1,BUF_LEN,keyFile);
	/* Write the Pre Shared key and save it */
	/* First write to device */
	fwrite(psk,1,BUF_LEN,dev);

	/* Write complete, now subsequent writes will be DATA */

	size_t dataS = 0;
	size_t dataDriver = 0;

	do
	{
		dataS = fread(psk,1,BUF_LEN,encFile);
		psk[BUF_LEN] = '\0';
		// printf("%s\n%lu\n", psk, dataS);
		if(dataS == 0)
		{
			break;
		}
		if(dataS < BUF_LEN)
		{
			psk[dataS] = '\0';
		}

		fwrite(psk,1,BUF_LEN,dev);
		dataDriver = fread(psk,1,BUF_LEN,dev);
		fwrite(psk,1,dataDriver,outFile);
		printf("%zu\n", dataS);
	}while(dataS != 0);

	fclose(dev);
	fclose(keyFile);
	fclose(outFile);
	fclose(encFile);
	return 0;
}