#include "muggle/c/muggle_c.h"

// try
// -a foo -b bar -X
// -b bar -X
// -a foo -X
int main(int argc, char *argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, "a:b:X")) != -1)
	{
		switch (opt)
		{
		case 'a':
		{
			printf("Option 'a' has arg: %s\n", optarg);
		}break;
		case 'b':
		{
			printf("Option 'b' has arg: %s\n", optarg);
		}break;
		case 'X':
		{
			printf("Option 'X' was provided\n");
		}break;
		}
	}

	printf("bye bye\n");

	return 0;
}
