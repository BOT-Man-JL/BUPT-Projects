#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <Winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include <stdlib.h>		// rand ();
#include <math.h>		// pow ();

#include "Impl.h"
#include "protocol.h"

typedef struct _Socket_Recv_Raw
{
	unsigned int time_stamp;
	unsigned int read_offset;
	unsigned int size;
	struct _Socket_Recv_Raw *next_ptr;
	unsigned char data[256];

	// INT32[260]
	// [0] = timestamp
	// [1] = reading offset
	// [2] = size
	// [3] = next_ptr
	// [4-259] = data
} Socket_Recv_Raw;

static Socket_Recv_Raw *p_socket_recved_raw = NULL;

void* recving_scheme ()
{
	int c_recv_byte_count;

	// For recording Chain
	static Socket_Recv_Raw *p_prev_buf;

	// New Recving buffer
	Socket_Recv_Raw *p_socket_recved_raw_new =
		malloc (sizeof (Socket_Recv_Raw));

	// malloc error
	if (!p_socket_recved_raw_new)
	{
		lprintf (str_Newline);
		log_printf (str_FatalAbort, "No enough memory");
		exit (0);
	}

	// Set reading offset
	p_socket_recved_raw_new->read_offset = 0;

	// Recv data and Set size, but why not 256?
	c_recv_byte_count = recv (socket_mine, p_socket_recved_raw_new->data, 242, 0);
	p_socket_recved_raw_new->size = c_recv_byte_count;

	// Disconnected while recving
	if (c_recv_byte_count <= 0)
	{
		log_printf ("TCP disconnected.\n");
		exit (0);
	}

	// Update total recv count
	// Two recved bytes are actually one original byte
	c_recv_total_bit += (8 / 2) * c_recv_byte_count;

	// Impose noise
	if (c_noise != 0.0)
	{
		double f_fraction = 3.5;

		// Current error rate is not enough
		if ((double) c_err_count / (double) c_recv_total_bit <= c_noise)
			f_fraction = 6.0;		// HIDWORD(f_fraction) = 0x40180000;

		int c_threshold = (1.0 - pow (1.0 - c_noise, p_socket_recved_raw_new->size * f_fraction)) * (RAND_MAX+1.0) + 0.5;

		if (rand () <= c_threshold)
		{
			unsigned char *p_noise_pos = &p_socket_recved_raw_new->data[rand () % p_socket_recved_raw_new->size];
			if (*p_noise_pos & 0xF)
			{
				*p_noise_pos ^= (1 << (rand () % 8));
				++c_err_count;

				dbg_warning (
					"Impose noise on received data, %u/%u=%.1E\n",
					c_err_count,
					c_recv_total_bit,
					(double) c_err_count / (double) (unsigned int) c_recv_total_bit);
			}
		}
	}

	// Set timestamp and next_ptr
	p_socket_recved_raw_new->time_stamp = c_current_time + 260;
	p_socket_recved_raw_new->next_ptr = NULL;

	// result = Previous set Raw buf
	void* result = p_socket_recved_raw;

	// Raw buf Not Empty
	if (p_socket_recved_raw)
	{
		// Prev buf's next_ptr = New buf
		p_prev_buf->next_ptr = p_socket_recved_raw_new;

		// Next turn's Prev buf = New buf
		p_prev_buf = p_socket_recved_raw_new;
	}
	// Raw buf Empty
	else
	{
		// Next turn's Prev buf = New buf
		p_prev_buf = p_socket_recved_raw_new;

		// Raw buf = New buf
		p_socket_recved_raw = p_socket_recved_raw_new;
	}

	return result;
}

char recv_byte ()
{
	char ret;

	// raw_buffer not set or timestamp is newer than now
	if (!p_socket_recved_raw ||
		p_socket_recved_raw->time_stamp > c_current_time)
	{
		lprintf (str_Newline);
		log_printf (str_FatalAbort,
					"recv_byte(): Receiving Queue is empty");
		exit (0);
	}

	// Read current byte and inc offset
	ret = p_socket_recved_raw->data[p_socket_recved_raw->read_offset];
	p_socket_recved_raw->read_offset++;

	// Read to the end
	if (p_socket_recved_raw->read_offset == p_socket_recved_raw->size)
	{
		Socket_Recv_Raw *p_socket_recved_raw_tmp = p_socket_recved_raw;

		// Find Next buf in the Chain
		p_socket_recved_raw = p_socket_recved_raw->next_ptr;

		// Free old buffer
		free (p_socket_recved_raw_tmp);
	}

	return ret;
}

int send_bytes_in_buf (unsigned int from, unsigned int to)
{
	int result = 0;

	// Is valid range
	if (from < to)
	{
		// send buf in range (from ~ to - 1)
		result = send (socket_mine, &socket_send_buf[from], to - from, 0);
		// Disconnected while Sending
		if (result <= 0)
		{
			log_printf ("TCP Disconnected.\n");
			exit (0);
		}
	}

	return result;
}

int sending_scheme ()
{
	static unsigned int c_last_time_sending;

	int result = c_current_time;

	unsigned int c_socket_buf_up_tmp;

	unsigned int c_actual_to_send;
	unsigned int c_actually_sent;

	UINT64 c_tmp;
	int c_tmp_2;

	// Has been here
	if (c_last_time_sending)
	{
		if (c_current_time > c_last_time_sending)
		{
			// Todo: how to calc this factor?
			c_tmp =
				((UINT64) (274877907ll * (8000 * (c_current_time - c_last_time_sending) / 8)) >> 32) >> 6;

			c_phl_capacity = 2 * ((c_tmp >> 31) + c_tmp);
			c_actual_to_send = phl_sq_len_2 ();

			if (c_actual_to_send > c_phl_capacity)
				c_actual_to_send = c_phl_capacity;

			// Calc the actual Upperbound
			c_socket_buf_up_tmp = (c_actual_to_send + c_socket_buf_low) % SOCKET_BUF_SIZE;

			// Wrap-around
			if (c_socket_buf_up_tmp < c_socket_buf_low)
			{
				c_tmp_2 = send_bytes_in_buf (c_socket_buf_low, SOCKET_BUF_SIZE);
				c_actually_sent = send_bytes_in_buf (0, c_socket_buf_up_tmp) + c_tmp_2;
			}
			// Non-wrapped
			else
				c_actually_sent = send_bytes_in_buf (c_socket_buf_low, c_socket_buf_up_tmp);

			// Update Lowerbound
			c_socket_buf_low = (c_actually_sent + c_socket_buf_low) % SOCKET_BUF_SIZE;

			// Able to send less bytes
			c_phl_capacity -= c_actually_sent;

			// Update timestamp
			c_last_time_sending = result = c_current_time;
		}
	}
	// First time to come
	else
		c_last_time_sending = c_current_time;

	return result;
}

int is_packet_ready ()
{
	static unsigned int c_last_time_checking;

	// Is Network Layer enabled by user
	if (!is_enable_network)
		return 0;

	// Flood Mode return 1 immediately
	if (!is_flood)
	{
		int c_current_time_copy = c_current_time;

		if (8000 * (c_current_time - c_last_time_checking) / 8 / 1000 < 192)
			return 0;

		if (station_type == 'b')
		{
			if (c_current_time / 1000 / c_B_idle_count % 2 != is_idle_busy_mode)
			{
				c_current_time_copy = c_current_time;
				if (c_current_time - c_last_time_checking < rand () % 500 + 4000)
					return 0;
			}
			if (c_current_time_copy < 1038)
				return 0;
		}

		c_last_time_checking = c_current_time_copy;
	}

	return 1;
}

UINT32 *check_stuffing ()
{
	UINT32 v0;
	UINT32 *v1;
	UINT32 v2;
	UINT32 *v3;

	// Head Checking
	v0 = 0;
	v1 = stuffing_2;
	do
	{
		if (*v1 != STUFFING2)
		{
			log_printf ("\nFATAL: Memory error, head[%d]=0x%02x\n",
						v0, stuffing_2[v0]);
			lprintf (str_Newline);
			log_printf (str_FatalAbort,
						"Memory used by 'protocol.lib' is corrupted by your program");
			exit (0);
		}
		++v1;
		++v0;
	}
	// Remark: Not safe here
	//while (v1 < &c_err_count);

	// Added by BOT Man JL :)
	while (v0 < STUFFING_SIZE);

	// Foot Checking
	v2 = 0;
	v3 = stuffing_1;
	do
	{
		if (*v3 != STUFFING1)
		{
			log_printf ("\nFATAL: Memory error, foot[%d]=0x%02x\n",
						v2, stuffing_2[v2]);
			lprintf (str_Newline);
			log_printf (str_FatalAbort,
						"Memory used by 'protocol.lib' is corrupted by your program");
			exit (0);
		}
		++v3;
		++v2;
	}
	// Remark: Not safe here
	//         New code removes the dependency,
	//         Seperating the vars
	//while (v3 < &c_last_time_long_sleeping);

	// Added by BOT Man JL :)
	while (v2 < STUFFING_SIZE);

	return v3;
}

int wait_for_event (int *arg_ret)
{
	unsigned int c_byte_to_extract;
	int c_countdown;

	char ch_recv_byte;

	time_t c_time_beg;
	time_t c_time_end;

	struct timeval timeout;
	fd_set writefds;
	fd_set readfds;

	static Socket_Recv *p_socket_recved_new = NULL;
	static unsigned int c_last_time_long_sleeping;

	int handle_timeout (int *arg_ret);

	while (1)
	{
		c_current_time = get_ms ();

		// recv_buf set and timestamp is not newer than now
		if (p_socket_recved_raw && p_socket_recved_raw->time_stamp <= c_current_time)
		{
			// extract recv_buf size
			c_byte_to_extract = p_socket_recved_raw->size - p_socket_recved_raw->read_offset;

			// Init started time
			if (!c_started_time)
			{
				if (c_current_time >= c_byte_to_extract / 2)
					c_started_time = c_current_time - c_byte_to_extract / 2;
				else
					c_started_time = c_current_time;
			}

			// recv_buf_raw can be extracted
			if (c_byte_to_extract > 0)
			{
				c_countdown = c_byte_to_extract;
				do
				{
					// Get original bytes from raw bytes
					ch_recv_byte = recv_byte ();

					// -1 indicates Beg_Frame / End_Frame
					if (ch_recv_byte == -1)
					{
						// End_Frame
						if (p_socket_recved_new)
						{
							// New buf Not Empty
							if (p_socket_recved_new->size > 0)
							{
								// Curr buf Not Empty
								if (p_socket_recved)
								{
									// Prev buf's Next buf = New buf
									p_socket_recved_prev->next_ptr = p_socket_recved_new;

									// Next turn's Prev buf = New buf
									p_socket_recved_prev = p_socket_recved_new;

									// Not Trace New buf
									p_socket_recved_new = 0;
								}
								// Curr buf Empty
								else
								{
									// Curr buf = New buf
									p_socket_recved = p_socket_recved_new;

									// Next turn's Prev buf = New buf
									p_socket_recved_prev = p_socket_recved_new;

									// Not Trace New buf
									p_socket_recved_new = 0;
								}
							}
						}
						// Beg_Frame
						else
						{
							p_socket_recved_new = calloc (1, sizeof (Socket_Recv));
						}
					}

					// Ensure p_socket_recved_new is allocated successfully
					else if (p_socket_recved_new)
					{
						// size not over 2048
						if (p_socket_recved_new->size < 2048)
						{
							// Is Low_4_Written
							if (p_socket_recved_new->flag)
							{
								// Write High-4-bit
								p_socket_recved_new->data[p_socket_recved_new->size] |=
									(BYTE) (ch_recv_byte << 4) ^ ch_recv_byte & 0xF0;

								// Inc size
								++(p_socket_recved_new->size);

								// Low_4_Written = 0 for next byte
								p_socket_recved_new->flag = 0;
							}
							else
							{
								// Write Low-4-bit
								p_socket_recved_new->data[p_socket_recved_new->size] =
									(BYTE) ch_recv_byte;

								// Low_4_Written = 1 for this byte
								p_socket_recved_new->flag = 1;
							}
						}
					}

					--c_countdown;
				} while (c_countdown);
			}

			// Actual buffer set
			if (p_socket_recved)
				return FRAME_RECEIVED;
		}

		// Check Socket I/O State
#if WIN32
		readfds.fd_array[0] = socket_mine;
		writefds.fd_array[0] = socket_mine;
		readfds.fd_count = 1;
		writefds.fd_count = 1;
#else
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_SET(socket_mine, &readfds);
		FD_SET(socket_mine, &writefds);
#endif

		timeout.tv_usec = 0;
		timeout.tv_sec = 0;

		if (select (socket_mine + 1, &readfds, &writefds, 0, &timeout) < 0)
		{
			lprintf (str_Newline);
			log_printf (str_FatalAbort, "system select()");
			exit (0);
		}

		//if (_WSAFDIsSet (socket_mine, &writefds))
		//	sending_scheme ();
		//if (_WSAFDIsSet (socket_mine, &readfds))
		//	recving_scheme ();

		// Is Socket I/O ready
#if WIN32
		for (unsigned int i = 0; i < writefds.fd_count; ++i)
			if (writefds.fd_array[i] == socket_mine)
			{
				sending_scheme ();
				break;
			}
		for (unsigned int i = 0; i < readfds.fd_count; ++i)
			if (readfds.fd_array[i] == socket_mine)
			{
				// Get p_socket_recved_raw
				recving_scheme ();
				break;
			}
#else
		if(FD_ISSET(socket_mine, &writefds))
			sending_scheme();
		if (FD_ISSET(socket_mine, &readfds))
			recving_scheme();
#endif
		// Network Layer Ready
		if (is_packet_ready ())
		{
			is_net_ready = 1;
			return NETWORK_LAYER_READY;
		}

		// Timeout
		int c_timeout_type = handle_timeout (arg_ret);
		if (c_timeout_type)
			return c_timeout_type;

		// Physical Layer Ready
		if (is_phl_ready && phl_sq_len () < 50)
		{
			is_phl_ready = 0;
			return PHYSICAL_LAYER_READY;
		}

		// Check stuffing and Sleep
		c_time_beg = get_ms ();
		check_stuffing ();
		Sleep (c_sleep_time);
		c_time_end = get_ms () - c_time_beg;

		// Check is_sleep_too_long
		if (c_time_end > c_sleep_time + 50 && time (0) > c_last_time_long_sleeping + 1)
		{
			log_printf ("** WARNING: System too busy, sleep %d ms,"
						" but be awakened %d ms later\n",
						c_sleep_time, c_time_end);
			c_last_time_long_sleeping = time (0);
		}

		// Quit?
		if (c_current_time > c_running_time_limit)
		{
			log_printf ("Quit.\n");
			exit (0);
		}
	}
}