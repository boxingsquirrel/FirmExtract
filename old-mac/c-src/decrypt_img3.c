#include <stdio.h>
#include <string.h>
#include "xpwntool/abstractfile.h"
#include "xpwntool/img3.h"
#include "xpwntool/nor_files.h"
#include "decrypt_img3.h"

#define BUFFERSIZE (1024*1024)

int decrypt_img3(char *in, char *out, char *key_str, char *iv_str) {
	//int argc=0;
	//char argv[]="";
	//init_libxpwn(argc, argv);

	//printf("INFO: Going to decrypt %s now...\n", in);
	char* inData;
	size_t inDataSize;

	AbstractFile* template = NULL;
	AbstractFile* certificate = NULL;
	unsigned int* key = NULL;
	unsigned int* iv = NULL;
	int hasKey = FALSE;
	int hasIV = FALSE;
	int x24k = FALSE;
	int xn8824k = FALSE;
	int doDecrypt = FALSE;

			doDecrypt = TRUE;
			template = createAbstractFileFromFile(fopen(in, "rb"));
			if(!template) {
				fprintf(stderr, "error: cannot open template\n");
				return 1;
			}

			size_t bytes;
			hexToInts(key_str, &key, &bytes);
			hasKey = TRUE;

			bytes=0;
			hexToInts(iv_str, &iv, &bytes);
			hasIV = TRUE;

	AbstractFile* inFile;
	if(doDecrypt) {
		if(hasKey) {
			inFile = openAbstractFile3(createAbstractFileFromFile(fopen(in, "rb")), key, iv, 0);
		} else {
			inFile = openAbstractFile3(createAbstractFileFromFile(fopen(in, "rb")), NULL, NULL, 0);
		}
	} else {
		if(hasKey) {
			inFile = openAbstractFile2(createAbstractFileFromFile(fopen(in, "rb")), key, iv);
		} else {
			inFile = openAbstractFile(createAbstractFileFromFile(fopen(in, "rb")));
		}
	}
	if(!inFile) {
		fprintf(stderr, "error: cannot open infile\n");
		return 2;
	}

	AbstractFile* outFile = createAbstractFileFromFile(fopen(out, "wb"));
	if(!outFile) {
		fprintf(stderr, "error: cannot open outfile\n");
		return 3;
	}


	AbstractFile* newFile;

	if(template) {
		if(hasKey && !doDecrypt) {
			newFile = duplicateAbstractFile2(template, outFile, key, iv, certificate);
		} else {
			newFile = duplicateAbstractFile2(template, outFile, NULL, NULL, certificate);
		}
		if(!newFile) {
			fprintf(stderr, "error: cannot duplicate file from provided template\n");
			return 4;
		}
	} else {
		newFile = outFile;
	}

	if(hasKey && !doDecrypt) {
		if(newFile->type == AbstractFileTypeImg3) {
			AbstractFile2* abstractFile2 = (AbstractFile2*) newFile;
			abstractFile2->setKey(abstractFile2, key, iv);
		}
	}

	if(x24k) {
		if(newFile->type == AbstractFileTypeImg3) {
			exploit24kpwn(newFile);
		}
	}	

	if(xn8824k) {
		if(newFile->type == AbstractFileTypeImg3) {
			exploitN8824kpwn(newFile);
		}
	}


	inDataSize = (size_t) inFile->getLength(inFile);
	inData = (char*) malloc(inDataSize);
	inFile->read(inFile, inData, inDataSize);
	inFile->close(inFile);

	newFile->write(newFile, inData, inDataSize);
	newFile->close(newFile);

	free(inData);

	if(key)
		free(key);

	if(iv)
		free(iv);

	return 0;
}
