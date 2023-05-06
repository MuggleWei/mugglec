/******************************************************************************
 *  @from  azrael.digipen.edu/~mmead/www/Courses/CS180/getopt.html
 *  @brief        
 *  	Associating Long Options with Short Options
 *
 *  |      command line options           |            output               |
 *  | --------------------------------------------------------------------- |
 *  | --delete=foo -c5 --add=yes --append | option d with value 'foo'       |
 *  |                                     | option c with value '5'         |
 *  |                                     | option a with value 'yes'       |
 *  |                                     | option p                        |
 *  | --------------------------------------------------------------------- |
 *  | --d=foo --ad=yes --ap               | option d with value 'foo'       |
 *  |                                     | option a with value 'yes'       |
 *  |                                     | option p                        |
 *  | --------------------------------------------------------------------- |
 *  | --create=5 --create 6 --c=7 --c 8   | option c with value '5'         |
 *  |                                     | option c with value '6'         |
 *  |                                     | option c with value '7'         |
 *  |                                     | option c with value '8'         |
 *  | --------------------------------------------------------------------- |
 *  | --file=5 --file 6 --file7           | option f with value '5'         |
 *  |                                     | option f with value 'NULL'      |
 *  |                                     | regular argument '6'            |
 *  |                                     | unknown option                  |
 *****************************************************************************/

#include "muggle/c/muggle_c.h"

int main(int argc, char *argv[])
{
	int c;
	while (1)
	{
		int option_index = 0;
		static struct option long_options[] =
		{
			{"add",     required_argument, NULL, 'a'},
			{"append",  no_argument,       NULL, 'p'},
			{"delete",  required_argument, NULL, 'd'},
			{"verbose", no_argument,       NULL, 'v'},
			{"create",  required_argument, NULL, 'c'},
			{"file",    optional_argument, NULL, 'f'},
			{NULL,      0,                 NULL, 0}
		};

		c = getopt_long( argc, argv, "-:a:pd:vc:f::", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) 
		{
			case 0:
				printf("long option %s", long_options[option_index].name);
				if (optarg)
					printf(" with arg %s", optarg);
				printf("\n");
				break;

			case 1:
				printf("regular argument '%s'\n", optarg);
				break;

			case 'a':
				printf("option a with value '%s'\n", optarg);
				break;

			case 'p':
				printf("option p\n");
				break;

			case 'd':
				printf("option d with value '%s'\n", optarg);
				break;

			case 'v':
				printf("option v\n");
				break;

			case 'c':
				printf("option c with value '%s'\n", optarg);
				break;

			case 'f':
				printf("option f with value '%s'\n", optarg ? optarg : "NULL");
				break;

			case '?':
				printf("Unknown option %c\n", optopt);
				break;

			case ':':
				printf("Missing option for %c\n", optopt);
				break;

			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}
	
	return 0;
}

