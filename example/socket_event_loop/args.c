#include "args.h"

int parse_sys_args(int argc, char **argv, sys_args_t *args)
{
	memset(args, 0, sizeof(*args));

	char usage_str[4096];
	memset(usage_str, 0, sizeof(usage_str));
	snprintf(usage_str, sizeof(usage_str),
		"\nUsage: %s [-h] [-v] <-a action_type> <-H host> <-P port>\n"
		"\t-h: show help information\n"
		"\t-v: show version\n"
		"\t-a: action type <tcp_server|tcp_client|udp_recv|udp_send>\n"
		"\t-H: bind|listen|connect host\n"
		"\t-P: bind|listen|connect port\n", argv[0]);

	int opt = 0;
	while ((opt = getopt(argc, argv, "hva:H:P:")) != -1)
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
			case 'a':
			{
				LOG_INFO("Option 'a' has arg: %s", optarg);
				if (strcmp(optarg, "tcp_server") == 0)
				{
					args->action_type = ACTION_TCP_SERVER;
				}
				else if (strcmp(optarg, "tcp_client") == 0)
				{
					args->action_type = ACTION_TCP_CLIENT;
				}
				else if (strcmp(optarg, "udp_recv") == 0)
				{
					args->action_type = ACTION_UDP_RECV;
				}
				else if (strcmp(optarg, "udp_send") == 0)
				{
					args->action_type = ACTION_UDP_SEND;
				}
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

	return 0;
}
