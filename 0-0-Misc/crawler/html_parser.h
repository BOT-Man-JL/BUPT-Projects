
// Parse urls from html code
//   by BOT Man & ZhangHan, 2018

#ifndef HTML_PARSER
#define HTML_PARSER

// sync multi callback
typedef void (*yeild_atag_urls_callback_fn)(const char* url, void* context);

void ParseAtagUrls(const char* html,
                   yeild_atag_urls_callback_fn callback,
                   void* context);

#endif  // HTML_PARSER
