/******************************************************************************
 *  @from  azrael.digipen.edu/~mmead/www/Courses/CS180/getopt.html
 *  @brief        
 *  	Optional Option Arguments
 *
 *  | command line options     |         output                 |
 *  | --------------------------------------------------------- |
 *  | -a -b bar -X             | Option 'a' has arg: (none)     |
 *  |                          | Option 'b' has arg: bar        |
 *  |                          | Option 'X' was provided        |
 *  | --------------------------------------------------------- |
 *  | -afoo -b bar -X          | Option 'a' has arg: foo        |
 *  |                          | Option 'b' has arg: bar        |
 *  |                          | Option 'X' was provided        |
 *  | --------------------------------------------------------- |
 *  | -a foo -b bar -X         | Option 'a' has arg: (none)     |
 *  |                          | Non-option arg: foo            |
 *  |                          | Option 'b' has arg: bar        |
 *  |                          | Option 'X' was provided        |
 *****************************************************************************/

#include "muggle/c/muggle_c.h"

int main()
{
	int argc = 6;
	char *argv[] = {
		"example_getopt",
		"-a",
		"foo",
		"-b",
		"bar",
		"-X"
	};

	int opt;
	while ((opt = getopt(argc, argv, "-:a::b:X")) != -1)
	{
		switch (opt)
		{
		case 'a':
		{
			printf("Option 'a' has arg: %s\n", optarg ? optarg : "(none)");
		}break;
		case 'b':
		{
			printf("Option 'b' has arg: %s\n", optarg);
		}break;
		case 'X':
		{
			printf("Option 'X' was provided\n");
		}break;
		case '?':
		{
			printf("Unknown option: %c\n", optopt);
		}break;
		case ':':
		{
			printf("Missing arg for '%c'\n", (char)opt);
		}break;
		case 1:
		{
			printf("Non-option arg: %s\n", optarg);
		}break;
		}
	}

	printf("bye bye\n");

	return 0;
}
