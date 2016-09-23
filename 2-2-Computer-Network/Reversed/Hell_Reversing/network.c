
#include "Impl.h"
#include "protocol.h"

void enable_network_layer ()
{
	is_enable_network = 1;
}

void disable_network_layer ()
{
	is_enable_network = 0;
}

int rand_1 ()
{
	static unsigned int c_for_rand = 0x65109BC4;
	c_for_rand = 214013 * c_for_rand + 2531011;
	return (c_for_rand >> 16) & 0x7FFF;
}

int rand_2 ()
{
	static unsigned int c_for_rand_2 = 0x1E459090;
	c_for_rand_2 = 214013 * c_for_rand_2 + 2531011;
	return (c_for_rand_2 >> 16) & 0x7FFF;
}

int get_packet (unsigned char *packet)
{
	static int c_sent_total_pack;

	int (*fn_rand)();

	// Set fn_rand (a <-> rand_1)
	fn_rand = rand_1;
	if (station_type != 'a')
		fn_rand = rand_2;

	// Check ready
	if (!is_net_ready)
	{
		lprintf (str_Newline);
		log_printf (str_FatalAbort,
					"get_packet(): Network layer is not ready for a new packet");
		exit (0);
	}

	// Fill data
	unsigned int offset = 2;
	do
		packet[offset++] = fn_rand ();
	while (offset < PKT_LEN);

	*(short *) &packet[0] = c_sent_total_pack++ % 10000 + 10000 * station_type + 23040;

	// Disable Network Layer
	is_net_ready = 0;
	return PKT_LEN;
}

void put_packet (unsigned char *packet, int size)
{
	int (*fn_rand)();

	static unsigned int c_last_time_output;

	static unsigned int c_recv_total_byte;
	static unsigned int c_recv_total_pack;

	// Set fn_rand (a <-> rand_2)
	fn_rand = rand_2;
	if (station_type != 'a')
		fn_rand = rand_1;

	// Check size
	if (size != PKT_LEN)
	{
		lprintf (str_Newline);
		log_printf (str_FatalAbort, "Bad Packet length");
		exit (0);
	}

	// Check data
	unsigned int offset = 2;
	do
	{
		if (packet[offset] != (unsigned char) fn_rand ())
		{
			lprintf (str_Newline);
			log_printf (str_FatalAbort,
						"Network Layer received a bad packet from data link layer");
			exit (0);
		}
		++offset;
	} while (offset < PKT_LEN);

	// Record
	c_recv_total_pack++;
	c_recv_total_byte += PKT_LEN;

	// Output
	if (c_current_time - c_last_time_output > 2000 &&
		c_current_time > c_started_time + 2000)
	{
		double c_bps_tmp =
			(double) c_recv_total_byte * 8000.0 /
			(double) (c_current_time - c_started_time);
		log_printf (
			".... %d packets received, %.0f bps, %.2f%%, Err %d (%.1e)\n",
			c_recv_total_pack,
			c_bps_tmp,
			c_bps_tmp * 0.0125,
			c_err_count,
			(double) c_err_count / (double) c_recv_total_bit);
		c_last_time_output = c_current_time;
	}
}
