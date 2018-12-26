
// Url Map Utils
//   by BOT Man & ZhangHan, 2018

#ifndef URL_MAP
#define URL_MAP

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// sync multi callback
typedef void (*yeild_url_connection_index_callback_fn)(const char* url,
                                                       size_t index,
                                                       void* context);
// sync multi callback
typedef void (*yeild_url_connection_pair_callback_fn)(size_t src,
                                                      size_t dst,
                                                      void* context);

void ConnectUrls(const char* src, const char* dst);

void YieldUrlConnectionIndex(yeild_url_connection_index_callback_fn callback,
                             void* context);
void YieldUrlConnectionPair(yeild_url_connection_pair_callback_fn callback,
                            void* context);

#ifdef __cplusplus
}
#endif

#endif  // URL_MAP
