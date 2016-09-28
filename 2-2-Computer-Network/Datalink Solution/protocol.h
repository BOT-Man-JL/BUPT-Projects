#ifndef __PROTOCOLAPI_HEAD_FILE_6tfr12hn__

#define __PROTOCOLAPI_HEAD_FILE_6tfr12hn__

#ifdef  __cplusplus
extern "C" {
#endif

#if WIN32
#ifdef PROTOCOLAPI_EXPORTS
#define PROTOCOL_API __declspec(dllexport)
#else
#define PROTOCOL_API __declspec(dllimport)
#endif
#else
#define PROTOCOL_API extern
#endif

/* Initalization */

PROTOCOL_API void protocol_init (int argc, char **argv);

/* Event Driver */

PROTOCOL_API int wait_for_event (int *arg);

#define NETWORK_LAYER_READY  0
#define PHYSICAL_LAYER_READY 1
#define FRAME_RECEIVED       2
#define DATA_TIMEOUT         3
#define ACK_TIMEOUT          4

/* Network Layer functions */

#define PKT_LEN 256

PROTOCOL_API void enable_network_layer (void);
PROTOCOL_API void disable_network_layer (void);
PROTOCOL_API int  get_packet (unsigned char *packet);
PROTOCOL_API void put_packet (unsigned char *packet, int len);

/* Physical Layer functions */

PROTOCOL_API int  recv_frame (unsigned char *buf, int size);
PROTOCOL_API void send_frame (unsigned char *frame, int len);

PROTOCOL_API int  phl_sq_len (void);

/* CRC-32 polynomium coding function */

PROTOCOL_API unsigned int crc32 (unsigned char *buf, int len);

/* Timer Management functions */

PROTOCOL_API unsigned int get_ms (void);
PROTOCOL_API void start_timer (unsigned int nr, unsigned int ms);
PROTOCOL_API void stop_timer (unsigned int nr);
PROTOCOL_API void start_ack_timer (unsigned int ms);
PROTOCOL_API void stop_ack_timer (void);

/* Protocol Debugger */

PROTOCOL_API void log_printf (char *fmt, ...);
PROTOCOL_API void lprintf (char *fmt, ...);
PROTOCOL_API char *station_name (void);

PROTOCOL_API void dbg_event (char *fmt, ...);
PROTOCOL_API void dbg_frame (char *fmt, ...);
PROTOCOL_API void dbg_warning (char *fmt, ...);

#define HERE log_printf("File \"%s\" (%d)\n", __FILE__, __LINE__)

#ifdef  __cplusplus
}
#endif

#endif

