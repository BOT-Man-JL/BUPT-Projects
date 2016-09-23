#include "Impl.h"
#include "protocol.h"

void memset32 (void *dest, UINT32 value, UINT32 size)
{
	unsigned int i;
	for (i = 0; i < (size & (~7)); i += 4)
	{
		memcpy (((char*) dest) + i, &value, 4);
	}
	for (; i < size; i++)
	{
		((char*) dest)[i] = ((char*) &value)[i & 3];
	}
}

int mem_stuffing ()
{
	int result;

	memset32 (stuffing_1, STUFFING1, sizeof (stuffing_1));
	result = STUFFING2;
	memset32 (stuffing_2, STUFFING2, sizeof (stuffing_2));

	return result;
}

void protocol_init(int argc, char **argv)
{
	struct
	{
		char dummy[4];		// Make it easier to check ".exe"
		char str[1028];
	} some_struct;

	SOCKET socket_tmp;
	SOCKET socket_tmp_2;

	struct sockaddr_in struct_sock;

	static time_t c_rand_seed = 0x98BCDE1;

	struct_sock.sin_port = 59144;
#if WIN32
	struct WSAData WSAData;
	if (WSAStartup (257, &WSAData))
	{
		lprintf (str_Newline);
		log_printf (str_FatalAbort, "Windows Socket DLL Error");
		exit (0);
	}
#endif
	mem_stuffing ();

	// Duplicate argc/argv
	int argc_copy = argc;
	char **argv_copy = argv;

	// bad argc
	if (argc < 2)
		goto tag_Err_Argument;


	// Set Console Title
	some_struct.str[0] = '\0';
	if (argc > 0)
	{
		char **argv_t = argv;
		do
		{
			sprintf (&some_struct.str[strlen (some_struct.str)], "%s ", *argv_t);
			++argv_t;
			--argc_copy;
		} while (argc_copy);
	}
#if WIN32
	SetConsoleTitleA (some_struct.str);
#endif
	some_struct.str[0] = '\0';

	// Arg_1 to lower-case
	char *arg_1 = argv[1];
	if (strlen (argv[1]) > 0)
	{
		unsigned int offset = 0;
		char ch;
		do
		{
			ch = arg_1[offset];
			if (ch >= 'A' && ch <= 'Z')
				arg_1[offset] = ch + 32;
			++offset;
		} while (offset < strlen (argv[1]));
	}

	// Read Arg_1
	if (arg_1 < &arg_1[strlen (argv[1])])
	{
		char ch;
		while (1)
		{
			ch = *arg_1;
			if (*arg_1 < '0' || *arg_1 > '9')
				break;
			dbg_mask = *arg_1 - '0';
tag_Read_FirstPart_Loop:
			if (++arg_1 >= argv[1] + strlen (argv[1]))
				goto tag_Read_FirstPart_Done;
		}

		switch (ch)
		{
		case 'a':
		case 'b':
			station_type = ch;
			goto tag_Read_FirstPart_Loop;
		case 'u':
			c_noise = 0;
			goto tag_Read_FirstPart_Loop;
		case 'i':
			is_idle_busy_mode = 1;
			goto tag_Read_FirstPart_Loop;
		case 'f':
			is_flood = 1;
			goto tag_Read_FirstPart_Loop;
		case 't':
			c_B_idle_count = 20;
			goto tag_Read_FirstPart_Loop;
		case 'r':
			c_rand_seed = time (0);
			goto tag_Read_FirstPart_Loop;
		case 'q':
			c_running_time_limit = 600000;
			goto tag_Read_FirstPart_Loop;
		case 'n':
			strcpy (some_struct.str, "NUL");
			goto tag_Read_FirstPart_Loop;
		case '-':
			goto tag_Read_FirstPart_Loop;
		default:
			printf ("ERROR: Unsupported option '%c'\n", *arg_1);
			break;
		}

tag_Err_Argument:
		printf ("\nUsage:\n  %s <options> [-port <port#>] [-ber <ber>] [-log <filename>]\n",
				argv_copy[0]);
		printf ("\nOptions : \n"
				"    A/B : Station name\n"
				"      u : Utopia channel (an error-free channel)\n"
				"      f : Flood traffic\n"
				"      i : Set station B layer 3 sender mode as IDLE-BUSY-IDLE-BUSY-...\n"
				"      n : Do Not create log file\n"
				"    0~3 : Debug mask (default: 0x%02x)\n\n"
				"  -port : TCP port number (default: %u)\n"
				"   -ber : Bit Error Rate (default: %.1E)\n"
				"   -log : Using assigned file as log file\n\n",
				dbg_mask, 59144, 0x88E368F1, 0x3EE4F8B5);
		exit (0);
	}

tag_Read_FirstPart_Done:

	// Verify station_type
	if (station_type != 'a' && station_type != 'b')
	{
		lprintf (str_Newline);
		log_printf (str_FatalAbort, "Station name must be 'A' or 'B'");
		exit (0);
	}

	// Print Log
	if (!some_struct.str[0])
	{
		strcpy (some_struct.str, argv[0]);
		if (!strcmpi (&some_struct.str[strlen (some_struct.str) - 4], ".eXe"))
			some_struct.str[strlen (some_struct.str) - 4] = '\0';

		const char *str_Postfix = "-A.log";
		if (station_type != 'a')
			str_Postfix = "-B.log";
		strcat (some_struct.str, str_Postfix);
	}

	// Read Other Args
	if (argc > 3)
	{
		int c_arg_read = 3;
		const char **arg_i = &argv[2];
		while (1)
		{
			if (strcmpi (*arg_i, "-ber"))
			{
				if (strcmpi (*arg_i, "-life"))
				{
					if (strcmpi (*arg_i, "-idle"))
					{
						if (strcmpi (*arg_i, "-tick"))
						{
							if (strcmpi (*arg_i, "-port"))
							{
								if (strcmpi (*arg_i, "-debug"))
								{
									if (strcmpi (*arg_i, "-log"))
									{
										argv_copy = argv;
										goto tag_Err_Argument;
									}
									strcpy (some_struct.str, *(arg_i + 1));
								}
								else
								{
									dbg_mask = strtol (*(arg_i + 1), 0, 0);
								}
							}
							else
							{
								struct_sock.sin_port = (u_short) strtol (*(arg_i + 1), 0, 0);
							}
						}
						else
						{
							c_sleep_time = strtol (*(arg_i + 1), 0, 0);
						}
					}
					else
					{
						c_B_idle_count = strtol (*(arg_i + 1), 0, 0);
					}
				}
				else
				{
					c_running_time_limit = 1000 * strtol (*(arg_i + 1), 0, 0);
				}
			}
			else
			{
				c_noise = strtod (*(arg_i + 1), 0);
				if (c_noise >= 0.0)
				{
					if (c_noise > 1.0)
						c_noise = 1.0;
				}
				else
					c_noise = 0;
			}
			c_arg_read += 2;
			arg_i += 2;
			if (c_arg_read >= argc)
			{
				argv_copy = argv;
				break;
			}
		}
	}

	// Open Log output file
	if (!strcmpi (some_struct.str, "nul") || !strcmpi (some_struct.str, "null"))
		sprintf (some_struct.str, "NUL");
	p_Log_File = fopen (some_struct.str, "w");

	if (!p_Log_File)
	{
		int *c_err_code = errno;
		char *str_err = strerror (*c_err_code);
		lprintf ("WARNING: Failed to create log file \"%s\": %s\n",
				 some_struct.str, str_err);
		strcpy (some_struct.str, "NUL");
		Sleep (1000u);
	}

	// Print Args
	if (argc > 0)
	{
		char **argv_copy_2 = argv_copy;
		int argc_copy_2 = argc;
		do
		{
			lprintf ("%s ", *argv_copy_2);
			++argv_copy_2;
			--argc_copy_2;
		} while (argc_copy_2);
	}

	// Print info
	lprintf (str_Newline);
	lprintf (
		"=========================================================================\n"
		"                          Station %s                                 \n"
		"-------------------------------------------------------------------------\n",
		station_name ());
	lprintf ("Protocol.lib, version %s, Build: %s (jiangy@public3.bta.net.cn)\n"
			 "\n\tHacked by BOT Man JL, ZPCCZQ, enjoy it :)\n\n",
			 "3.0", "Oct 19 2009");
	lprintf ("Channel: %d bps, %d ms propagation delay, bit error rate ",
			 8000, 270);
	if (c_noise <= 0.0)
		lprintf ("\n");
	else
		lprintf ("%.1E\n", c_noise);
	lprintf ("Log file \"%s\", TCP port %d, debug mask 0x%02x\n",
			 some_struct.str, struct_sock.sin_port, dbg_mask);

	// TCP Connecting for A
	if (station_type == 'a')
	{
		srand (c_rand_seed ^ 0x17BB9);

		// New Socket
		socket_tmp = socket (2, 1, 6);
		socket_tmp_2 = socket_tmp;

		if (socket_tmp < 0)
		{
			lprintf (str_Newline);
			log_printf (str_FatalAbort, "Create TCP socket");
			exit (0);
		}

		// Bind
		struct_sock.sin_family = AF_INET;
		struct_sock.sin_addr.s_addr = inet_addr ("127.0.0.1");
		struct_sock.sin_port = htons (struct_sock.sin_port);

		if (bind (socket_tmp, &struct_sock, 16) < 0)
		{
			log_printf ("Station A: Failed to bind TCP port %u",
						htons (struct_sock.sin_port));
			lprintf (str_Newline);
			log_printf (str_FatalAbort, "Station A failed to bind TCP port");
			exit (0);
		}

		// Listen
		listen (socket_tmp_2, 5);

		lprintf ("Station A is waiting for station B on TCP port %u ... ",
				 htons (struct_sock.sin_port));
		fflush (stdout);

		// Accept
		socket_mine = accept (socket_tmp_2, 0, 0);

		if ((socket_mine & 0x80000000) != 0)
		{
			lprintf (str_Newline);
			log_printf (str_FatalAbort,
						"Station A failed to communicate with station B");
			exit (0);
		}
		lprintf ("Done.\n");

		// Recv
		// Remark: Old sizeof (time_t) is 4
		//recv (socket_mine, (char *) &c_handshake_time, 4, 0);

		// Added by BOT Man JL :)
		recv (socket_mine, (char *) &c_handshake_time, sizeof (time_t), 0);
	}

	// TCP Connecting for B
	if (station_type == 'b')
	{
		srand (c_rand_seed ^ 18231);

		// New Socket
		socket_mine = socket (2, 1, 6);

		if ((socket_mine & 0x80000000) != 0)
		{
			lprintf (str_Newline);
			log_printf (str_FatalAbort, "Create TCP socket");
			exit (0);
		}

		// Connect
		struct_sock.sin_family = AF_INET;
		struct_sock.sin_addr.s_addr = inet_addr ("127.0.0.1");
		struct_sock.sin_port = htons (struct_sock.sin_port);

		int c_waiting_count = 0;
		while (1)
		{
			lprintf ("Station B is connecting station A (TCP port %u) ... ",
					 htons (struct_sock.sin_port));
			fflush (stdout);

			if (connect (socket_mine, &struct_sock, 16) >= 0)
				break;

			lprintf ("Failed!\n");
			Sleep (2000u);

			if (++c_waiting_count >= 60)
				goto tag_B_Connection_Failed;
		}
		lprintf ("Done.\n");

tag_B_Connection_Failed:
		if (c_waiting_count == 6)
		{
			lprintf (str_Newline);
			log_printf (str_FatalAbort,
						"Station B failed to connect station A");
			exit (0);
		}

		// Send
		time (&c_handshake_time);
		++c_handshake_time;

		// Remark: Old sizeof (time_t) is 4
		//send (socket_mine, (const char *) &c_handshake_time, sizeof (unsigned int), 0);

		// Added by BOT Man JL :)
		send (socket_mine, (const char *) &c_handshake_time, sizeof (time_t), 0);
	}

	lprintf ("New epoch: %s", asctime (localtime (&c_handshake_time)));
	lprintf ("====================================="
			 "====================================\n");

	while (time (0) < c_handshake_time)
		Sleep (10u);

	UINT32 bOptVal;
	UINT32 bOptLen = sizeof(UINT32);
	UINT32 iOptVal;
	UINT32 iOptLen = sizeof(UINT32);
#if WIN32
	iOptVal = 10;
	setsockopt(socket_mine, SOL_SOCKET, SO_SNDTIMEO, (const char *)&iOptVal, iOptLen);
	setsockopt(socket_mine, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iOptVal, iOptLen);
#else
	struct timeval timeout = {10, 0};
	setsockopt(socket_mine, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(struct timeval));
	setsockopt(socket_mine, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(struct timeval));
#endif

	iOptVal = 0x10000;
	setsockopt(socket_mine, SOL_SOCKET, SO_SNDBUF, (const char *)&iOptVal, iOptLen);
	setsockopt(socket_mine, SOL_SOCKET, SO_RCVBUF, (const char *)&iOptVal, iOptLen);

	bOptVal = 1;
	setsockopt(socket_mine, IPPROTO_TCP, TCP_NODELAY, (const char *)&bOptVal, bOptLen);

	get_ms ();
}
