
#include "Impl.h"
#include "protocol.h"
#include <time.h>
#include <sys/timeb.h>

static unsigned int array_timer[MAX_TIMER_COUNT];
static unsigned int c_ack_timeout;

unsigned int get_ms ()
{
	time_t result;
	struct timeb struct_time;
	ftime (&struct_time);
	if (c_handshake_time)
		result = struct_time.millitm + 1000 * (struct_time.time - c_handshake_time);
	else
		result = 0;
	return result;
}

void start_timer (unsigned int nr, unsigned int ms)
{
	int phl_queue_len;

	if (nr >= MAX_TIMER_COUNT)
	{
		lprintf (str_Newline);
		log_printf (str_FatalAbort,
					"start_timer(): timer No. must be 0~128");
		// Minor bug: 0~127... :-(
		exit (0);
	}

	phl_queue_len = 8000 * phl_sq_len () / 8000;
	array_timer[nr] = ms + c_current_time + phl_queue_len;

	//return ms;
}

void stop_timer (unsigned int nr)
{
	if (nr < MAX_TIMER_COUNT)
		array_timer[nr] = 0;

	//return nr;
}

int get_timer (unsigned int nr)
{
	int c_timeout;
	int result;

	if (nr < MAX_TIMER_COUNT &&
		(c_timeout = array_timer[nr]) != 0 &&
		c_timeout > c_current_time)
		result = c_timeout - c_current_time;
	else
		result = 0;
	return result;
}

void start_ack_timer (unsigned int ms)
{
	if (!c_ack_timeout)
	{
		c_ack_timeout = ms + c_current_time;
	}
	//return result;
}

void stop_ack_timer ()
{
	c_ack_timeout = 0;
}

int handle_timeout(int *arg)
{
	unsigned int *ptimer;
	int n;

	n = 0;
	ptimer = array_timer;
	while (!*ptimer || *ptimer > c_current_time)
	{
		++ptimer;
		++n;
		if (ptimer >= &array_timer[128])
			return 0;
	}
	*arg = n;
	array_timer[n] = 0;
	return (n == 128) + 3;
}
