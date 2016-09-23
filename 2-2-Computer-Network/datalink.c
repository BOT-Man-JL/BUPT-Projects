#include <stdio.h>
#include <string.h>

#include "protocol.h"
#pragma comment (lib, "Protocol.lib")

#define DATA_TIMER  3000
#define ACK_TIMER	240

#define MAX_SEQ 15
#define NR_BUFS ((MAX_SEQ + 1) / 2)
#define inc(k) (k = k < MAX_SEQ ? k + 1 : 0)

#define FRAME_DATA 1	// KIND(1) | SEQ(1) | ACK(1) | DATA(240~256) | CRC(4)
#define FRAME_ACK  2	// KIND(1) | ACK(1) | CRC(4)
#define FRAME_NAK  3	// KIND(1) | ACK(1) | CRC(4)

typedef enum { false, true } boolean;

struct FRAME
{
	unsigned char kind;
	unsigned char ack;
	unsigned char seq;
	unsigned char data[PKT_LEN];
	unsigned int  padding;		// perhaps checksum
};

// Add checksum to the end of a frame,
// and Put it to physical layer
static void put_frame (unsigned char *frame, int len)
{
	*(unsigned int *) (frame + len) = crc32 (frame, len);

	// sizeof (frame + checksum)
	send_frame (frame, len + sizeof (unsigned int));
}

static void send_data_frame (unsigned char frame_nr,
							 unsigned char frame_expected,
							 unsigned char *packet,
							 size_t packet_len)
{
	struct FRAME s;

	s.kind = FRAME_DATA;
	s.seq = frame_nr;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);

	if (packet_len > PKT_LEN)
	{
		dbg_frame (" :( Can't Send seq %d - ack %d of length %d\n",
				   s.seq, s.ack, packet_len);
		return;
	}

	memcpy (s.data, packet, packet_len);

	dbg_frame (" <- Send seq %d - ack %d, ID %d\n",
			   s.seq, s.ack, *(short *) s.data);

	// sizeof (kind + ack + seq + data)
	put_frame ((unsigned char *) &s, 3 + packet_len);
}

static void send_ack_frame (unsigned char frame_expected)
{
	struct FRAME s;

	s.kind = FRAME_ACK;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);

	dbg_frame (" <- Send ACK %d\n", s.ack);

	// sizeof (kind + ack)
	put_frame ((unsigned char *) &s, 2);
}

static void send_nak_frame (unsigned char frame_expected)
{
	struct FRAME s;

	s.kind = FRAME_NAK;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);

	dbg_frame (" <- Send NAK %d\n", s.ack);

	// sizeof (kind + ack)
	put_frame ((unsigned char *) &s, 2);
}

static boolean between (unsigned char a,
						unsigned char b,
						unsigned char c)
{
	// a <= b < c circularly
	return (
		((a <= b) && (b < c)) ||
		((c < a) && (a <= b)) ||
		((b < c) && (c < a)));
}

void go_back_n (int argc, char **argv)
{
	int event, arg;

	struct FRAME frame;

	boolean no_nak = true;
	boolean phl_ready = false;

	int frame_len;
	unsigned char buffer[MAX_SEQ + 1][PKT_LEN];
	int packet_len[MAX_SEQ + 1];

	unsigned char nbuffered = 0;

	unsigned char ack_expected = 0;			// Lowerbound of Sending Window
	unsigned char next_frame_to_send = 0;	// Upperbound of Sending Window

	unsigned char frame_expected = 0;		// L/U bound of Receiving Window

	protocol_init (argc, argv);
	disable_network_layer ();

	while (1)
	{
		event = wait_for_event (&arg);

		switch (event)
		{
		case NETWORK_LAYER_READY:

			// From Network Layer
			packet_len[next_frame_to_send]
				= get_packet (buffer[next_frame_to_send]);
			nbuffered++;

			// To Physical Layer
			send_data_frame (next_frame_to_send,
							 frame_expected,
							 buffer[next_frame_to_send],
							 packet_len[next_frame_to_send]);

			start_timer (next_frame_to_send, DATA_TIMER);
			stop_ack_timer ();

			inc (next_frame_to_send);

			phl_ready = false;
			break;

		case PHYSICAL_LAYER_READY:

			phl_ready = true;
			break;

		case FRAME_RECEIVED:

			// From Physical Layer
			frame_len = recv_frame ((unsigned char *) &frame,
									sizeof frame);

			// Checksum Error
			if (frame_len < 5 ||
				crc32 ((unsigned char *) &frame, frame_len) != 0)
			{
				dbg_frame ("\t\t\t\t\t :( Checksum Error seq %d - ack %d \n",
						   frame.seq, frame.ack);

				if (no_nak)
				{
					send_nak_frame (frame_expected);
					no_nak = false;
					stop_ack_timer ();
				}
				// Ignore bad Frame
				break;
			}

			switch (frame.kind)
			{
			case FRAME_DATA:
				dbg_frame ("\t\t\t\t\t -> Recv seq %d - ack %d, ID %d\n",
						   frame.seq, frame.ack, *(short *) frame.data);

				// Is Expected Frame
				if (frame.seq == frame_expected)
				{
					// To Network Layer
					put_packet (frame.data,
								frame_len - 3 - sizeof (unsigned int));

					no_nak = true;
					inc (frame_expected);
					start_ack_timer (ACK_TIMER);
				}
				else if (no_nak)
				{
					send_nak_frame (frame_expected);
					no_nak = false;
					stop_ack_timer ();
				}
				break;

			case FRAME_ACK:
				dbg_frame ("\t\t\t\t\t -> Recv Ack %d\n", frame.ack);
				break;

			case FRAME_NAK:
				dbg_frame ("\t\t\t\t\t -> Recv Nak %d\n", frame.ack);
				break;

			default:
				dbg_event (" :( Error Frame Kind");
				break;
			}

			// Update ack_expected
			while (between (ack_expected, frame.ack, next_frame_to_send))
			{
				nbuffered--;
				stop_timer (ack_expected);
				inc (ack_expected);
			}

			if (frame.kind == FRAME_NAK)
			{
				stop_timer (ack_expected + 1);
				goto tag_Resend;
			}

			break;

		case DATA_TIMEOUT:
			dbg_event (" :( Frame Timeout %d\n", arg);

tag_Resend:
			next_frame_to_send = ack_expected;
			for (unsigned char i = 0; i < nbuffered; i++)
			{
				send_data_frame (next_frame_to_send,
								 frame_expected,
								 buffer[next_frame_to_send],
								 packet_len[next_frame_to_send]);

				start_timer (next_frame_to_send, DATA_TIMER);
				stop_ack_timer ();

				inc (next_frame_to_send);
			}

			phl_ready = false;
			break;

		case ACK_TIMEOUT:

			send_ack_frame (frame_expected);
			stop_ack_timer ();
			break;
		}

		// Control packet input
		if (nbuffered < MAX_SEQ && phl_ready)
			enable_network_layer ();
		else
			disable_network_layer ();
	}
}

void selective (int argc, char **argv)
{
	int event, oldest_frame;

	struct FRAME frame;

	boolean no_nak = true;
	boolean phl_ready = false;

	unsigned char ack_expected = 0;			// Lowerbound of Sender
	unsigned char frame_to_send = 0;		// Upperbound of Sender
	unsigned char frame_expected = 0;		// Lowerbound of Receiver
	unsigned char too_far = NR_BUFS;		// Lowerbound of Receiver

	// Out buf
	unsigned char out_buf[NR_BUFS][PKT_LEN];
	int packet_len[NR_BUFS];
	boolean arrived[NR_BUFS];

	// In buf
	unsigned char in_buf[NR_BUFS][PKT_LEN];

	unsigned char nbuffer = 0;
	int frame_len;

	for (int i = 0; i != NR_BUFS; ++i) arrived[i] = false;

	protocol_init (argc, argv);
	disable_network_layer ();

	while (1)
	{
		event = wait_for_event (&oldest_frame);

		switch (event)
		{
		case NETWORK_LAYER_READY:
			++nbuffer;
			packet_len[frame_to_send % NR_BUFS] =
				get_packet (out_buf[frame_to_send % NR_BUFS]);


			send_data_frame (frame_to_send,
							 frame_expected,
							 out_buf[frame_to_send % NR_BUFS],
							 packet_len[frame_to_send % NR_BUFS]);

			start_timer (frame_to_send % NR_BUFS, DATA_TIMER);
			stop_ack_timer ();

			inc (frame_to_send);
			break;

		case PHYSICAL_LAYER_READY:
			phl_ready = true;
			break;

		case FRAME_RECEIVED:
			frame_len = recv_frame ((unsigned char*) & frame, sizeof frame);
			if (crc32 ((unsigned char *) & frame, frame_len))
			{
				dbg_frame ("\t\t\t :( Checksum Error seq %d - ack %d \n",
						   frame.seq, frame.ack);
				if (no_nak)
				{
					send_nak_frame (frame_expected);
					no_nak = false;
					stop_ack_timer ();
				}
				// Ignore bad Frame
				break;
			}

			if (frame.kind == FRAME_DATA)
			{
				if ((frame.seq != frame_expected) && no_nak)
				{
					dbg_frame ("\t\t\t -> Received a correct frame seq %d, ack %d, ID %d,\n",
							   frame.seq, frame.ack, *(short *) frame.data);
					dbg_event ("\t\t\t    but no expected.\n");
					send_nak_frame (frame_expected);
					no_nak = false;
					stop_ack_timer ();
				}
				else start_ack_timer (ACK_TIMER);

				if (between (frame_expected, frame.seq, too_far) &&
					(arrived[frame.seq % NR_BUFS] == false))
				{
					arrived[frame.seq % NR_BUFS] = true;
					memcpy (in_buf[frame.seq % NR_BUFS], frame.data, PKT_LEN);
					while (arrived[frame_expected % NR_BUFS])
					{
						dbg_frame ("\t\t\t -> Received a correct frame seq %d, ack %d, ID %d.\n",
								   frame.seq, frame.ack, *(short *) frame.data);
						put_packet (in_buf[frame_expected % NR_BUFS], PKT_LEN);

						no_nak = true;
						arrived[frame_expected % NR_BUFS] = false;

						inc (frame_expected);
						inc (too_far);
						start_ack_timer (ACK_TIMER);
					}
				}
			}

			if ((frame.kind == FRAME_NAK) &&
				between (ack_expected, (frame.ack + 1) % (MAX_SEQ + 1), frame_to_send))
			{
				dbg_event ("\t\t\t -> Received an NAK.\n");
				send_data_frame ((frame.ack + 1) % (MAX_SEQ + 1),
								 frame_expected,
								 out_buf[(frame.ack + 1) % NR_BUFS],
								 packet_len[(frame.ack + 1) % NR_BUFS]);

				start_timer ((frame.ack + 1) % NR_BUFS, DATA_TIMER);
				stop_ack_timer ();
			}

			while (between (ack_expected, frame.ack, frame_to_send))
			{
				--nbuffer;
				stop_timer (ack_expected % NR_BUFS);
				inc (ack_expected);
			}

			break;

		case DATA_TIMEOUT:
			if (!between (ack_expected, oldest_frame, frame_to_send))
				oldest_frame += NR_BUFS;

			dbg_event (" :( Frame Timeout %d\n", oldest_frame);

			send_data_frame (oldest_frame,
							 frame_expected,
							 out_buf[oldest_frame % NR_BUFS],
							 packet_len[oldest_frame % NR_BUFS]);

			start_timer (oldest_frame % NR_BUFS, DATA_TIMER);
			stop_ack_timer ();
			break;

		case ACK_TIMEOUT:
			send_ack_frame (frame_expected);
			stop_ack_timer ();
			break;
		}

		if (nbuffer < NR_BUFS && phl_ready) enable_network_layer ();
		else disable_network_layer ();
	}
}

int main (int argc, char **argv)
{
	if (argc < 2)
	{
		lprintf ("Too less arguments.\n");
		return 0;
	}

	switch (argv[1][strlen (argv[1]) - 1])
	{
	case 'G':
	case 'g':
		argv[1][strlen (argv[1]) - 1] = 0;
		go_back_n (argc, argv);
		break;

	case 'S':
	case 's':
		argv[1][strlen (argv[1]) - 1] = 0;
		selective (argc, argv);
		break;

	default:
		lprintf ("Please select a mode before starting.\n");
		return 0;
	}

	// Will not hit, because of hell exit(); inside
	return 0;
}
