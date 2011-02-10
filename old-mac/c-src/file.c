#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include "file.h"

char data[4096];

int r_file(const char *filename)
{
	FILE *f=fopen(filename, "r");

	if (f==NULL)
	{
		printf("ERROR: Could not open %s for reading\n", filename);
		//fclose(f);
		return -1;
	}

	else {
		unsigned char buffer[BUFSIZE];
		uint32_t read=fread(data, 1, 4096, f);

		while (read>0)
		{
			read=fread(data, 1, 4096, f);
			//printf(".");
		}

		//printf("\nINFO: Read sucessfully from %s\n", filename);

		return 0;
	}
}

void make_dir(const char *path)
{
	mkdir(path, 0777);
	return;
}
