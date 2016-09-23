
#include "Impl.h"
#include "protocol.h"
#include <stdio.h>

static char str_log_buf[0x4000];

const char *str_03f = "%.03f ";
const char *str_s = "%s";

const char *str_station_name_bad = "XXX";
const char *str_station_b = "B";
const char *str_station_a = "A";

char *station_name (void)
{
	char *result;

	if (station_type == 'a')
		result = str_station_a;
	else
	{
		result = str_station_b;
		if (station_type != 'b')
			result = str_station_name_bad;
	}
	return result;
}

void log_printf (char *format, ...)
{
	FILE *result;
	signed long long c_time_stamp;
	va_list va;

	va_start (va, format);
	*str_log_buf = 0;
	c_time_stamp = (unsigned int) get_ms ();
	sprintf (str_log_buf, str_03f, (double) c_time_stamp * 0.001);
	*(str_log_buf + strlen (str_log_buf) + vsprintf (str_log_buf + strlen (str_log_buf), format, va)) = 0;
	printf (str_s, str_log_buf);
	result = p_Log_File;

	if (p_Log_File)
	{
		fprintf (p_Log_File, str_s, str_log_buf, c_time_stamp);
		result = (FILE *) fflush (p_Log_File);
	}
	//return result;
}

void lprintf (char *format, ...)
{
	FILE *result;
	va_list va;

	va_start (va, format);
	*str_log_buf = 0;
	*(str_log_buf + vsprintf (str_log_buf + strlen (str_log_buf), format, va)) = 0;
	printf (str_s, str_log_buf);
	result = p_Log_File;

	if (p_Log_File)
	{
		fprintf (p_Log_File, str_s, str_log_buf);
		result = (FILE *) fflush (p_Log_File);
	}
	//return result;
}

void dbg_event (char *format, ...)
{
	char result;
	signed long long c_time_stamp;
	va_list va;

	va_start (va, format);
	result = dbg_mask;
	if (dbg_mask & 1)
	{
		*str_log_buf = 0;
		c_time_stamp = (unsigned int) get_ms ();
		sprintf (str_log_buf, str_03f, (double) c_time_stamp * 0.001);
		*(str_log_buf + strlen (str_log_buf) + vsprintf (str_log_buf + strlen (str_log_buf), format, va)) = 0;
		printf (str_s, str_log_buf);
		result = (char) p_Log_File;

		if (p_Log_File)
		{
			fprintf (p_Log_File, str_s, str_log_buf, c_time_stamp);
			result = fflush (p_Log_File);
		}
	}
	//return result;
}

void dbg_frame (char *format, ...)
{
	char result;
	signed long long c_time_stamp;
	va_list va;

	va_start (va, format);
	result = dbg_mask;
	if (dbg_mask & 2)
	{
		*str_log_buf = 0;
		c_time_stamp = (unsigned int) get_ms ();
		sprintf (str_log_buf, str_03f, (double) c_time_stamp * 0.001);
		*(str_log_buf + strlen (str_log_buf) + vsprintf (str_log_buf + strlen (str_log_buf), format, va)) = 0;
		printf (str_s, str_log_buf);
		result = (char) p_Log_File;

		if (p_Log_File)
		{
			fprintf (p_Log_File, str_s, str_log_buf, c_time_stamp);
			result = fflush (p_Log_File);
		}
	}
	//return result;
}

void dbg_warning (char *format, ...)
{
	char result;
	signed long long c_time_stamp;
	va_list va;

	va_start (va, format);
	result = dbg_mask;
	if (dbg_mask & 4)
	{
		*str_log_buf = 0;
		c_time_stamp = (unsigned int) get_ms ();
		sprintf (str_log_buf, str_03f, (double) c_time_stamp * 0.001);
		*(str_log_buf + strlen (str_log_buf) + vsprintf (str_log_buf + strlen (str_log_buf), format, va)) = 0;
		printf (str_s, str_log_buf);
		result = (char) p_Log_File;

		if (p_Log_File)
		{
			fprintf (p_Log_File, str_s, str_log_buf, c_time_stamp);
			result = fflush (p_Log_File);
		}
	}
	//return result;
}
