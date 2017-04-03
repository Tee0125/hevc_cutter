#include <stdio.h>
#include <stdint.h>

#define MIN(A,B) ((A>B)?B:A)

static FILE* in;

static int nal_pos;
static int bit_pos;

void init_stream(FILE* fp)
{
	in = fp;

	nal_pos = 0;
}

void seek_nal()
{
	int trailing_zeroes;
	int bytes[2];

	int code;

	// read first byte
	if ((code = fgetc(in)) == EOF)
		return;

	if (code == 0)
		trailing_zeroes = 1;
	else
		trailing_zeroes = 0;

	if ((code = fgetc(in)) == EOF)
		return;

	// read second byte
	if (code == 0)
		trailing_zeroes++;
	else
		trailing_zeroes = 0;

	// find start code pattern
	while ((code = fgetc(in)) != EOF)
	{
		if (code == 1 && trailing_zeroes >= 2)
		{
			// save nal start position
			nal_pos = ftell(in) - trailing_zeroes - 1;

			break;
		}
		else if (code == 3 && trailing_zeroes >= 2)
		{
			// Emulation prevent byte found, consume next byte 
			if (fgetc(in) == EOF)
				break;

			trailing_zeroes = 0;

			continue;
		}

		// update trailing bytes
		if (code == 0)
			trailing_zeroes = MIN(trailing_zeroes+1, 3);
		else
			trailing_zeroes = 0;
	}
}

int read_nal(int len)
{
	return 0;
}

void init_rbsp()
{
}

int read_rbsp(int len)
{
	return 0;
}

int main(int argc, char** argv)
{
	int i;
	FILE* in;

	if (argc < 2)
		return 1;

	in = fopen(argv[1], "rb");

	init_stream(in);

	for (i = 0 ; i < 5 ; i++)
	{
		seek_nal();
		printf("nal_start_pos: 0x%x\n", nal_pos);
	}

	fclose(in);

	return 0;	
}

