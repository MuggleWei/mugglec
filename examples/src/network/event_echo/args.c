#include "args.h"

int parse_sys_args(int argc, char **argv, sys_args_t *args)
{
	memset(args, 0, sizeof(*args));

	char usage_str[4096];
	memset(usage_str, 0, sizeof(usage_str));
	snprintf(usage_str, sizeof(usage_str),
		"\nUsage: %s [-h] [-v] <-H host> <-P port>\n"
		"\t-h: show help information\n"
		"\t-v: show mugglec version\n"
		"\t-H: bind&listen host\n"
		"\t-P: bind&listen port\n", argv[0]);

	int opt = 0;
	while ((opt = getopt(argc, argv, "hvH:P:")) != -1)
	{
		switch (opt)
		{
			case 'h':
			{
				LOG_INFO("%s", usage_str);
				exit(EXIT_SUCCESS);
			}break;
			case 'v':
			{
				LOG_INFO("mugglec version: %s", mugglec_version());
				exit(EXIT_SUCCESS);
			}break;
			case 'H':
			{
				LOG_INFO("Option 'H' has arg: %s", optarg);
				strncpy(args->host, optarg, sizeof(args->host));
			}break;
			case 'P':
			{
				LOG_INFO("Option 'P' has arg: %s", optarg);
				strncpy(args->port, optarg, sizeof(args->port));
			}break;
		}
	}

	if (args->host[0] == '\0' || args->port[0] == '\0')
	{
		LOG_ERROR("%s", usage_str);
		return -1;
	}

	return 0;
}
