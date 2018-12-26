
// Simple web crawler
//   by BOT Man & ZhangHan, 2018

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "bloom_filter.h"
#include "html_parser.h"
#include "http_client.h"
#include "string_helper.h"
#include "third_party/HTParse.h"
#include "url_map.h"

#define URL_HTTP_SCHEME "http://"
#define HANDLED_URL_SET_SIZE (16000000 * 100)
#define PAGE_URL_SET_SIZE (1000 * 100)

void RequestCallback(const char* url,
                     RequestStatus status,
                     const char* html,
                     void* context);

struct PendingRequest {
  const char* url;
  TAILQ_ENTRY(PendingRequest) _entries;
};

unsigned char g_is_fd_reach_limits;
size_t g_pending_request_count;
TAILQ_HEAD(, PendingRequest) g_pending_request_queue;

typedef struct {
  // referred source url
  const char* src_url;

  // local url-set for current page to avoid dup |ConnectUrls|
  BloomFilter* page_url_set;
} ProcessUrlContext;

// global url-set for crawling pages to avoid dup |Request|
BloomFilter* g_handled_url_set;

void ProcessUrl(const char* raw_url, void* context) {
  assert(raw_url);
  const ProcessUrlContext* page_context = (const ProcessUrlContext*)context;

  // fix |raw_url| by |src_url| to canonical |url| without fragment
  char* url = HTParse(raw_url, page_context ? page_context->src_url : NULL,
                      PARSE_ALL & ~PARSE_VIEW);
  HTSimplify(&url);

  // ignore non-http url
  if (strstr(url, URL_HTTP_SCHEME) != url) {
    free((void*)url);
    return;
  }

  // handle page connections (test page_url_set, handle by ConnectUrls)
  if (page_context) {
    assert(page_context->src_url);
    assert(page_context->page_url_set);

    // ensure |src_url| in |g_handled_url_set| already
    assert(BloomFilterTest(g_handled_url_set, page_context->src_url));

    if (!BloomFilterTest(page_context->page_url_set, url)) {
      BloomFilterAdd(page_context->page_url_set, url);

      // connect |src_url| to |url|
      ConnectUrls(page_context->src_url, url);
    }
  }

  // handle crawl tasks (test g_handled_url_set, handle by Request)
  if (!BloomFilterTest(g_handled_url_set, url)) {
    BloomFilterAdd(g_handled_url_set, url);

    // use |url| as start node to crawl pages
    // async once call |RequestCallback|
    Request(url, RequestCallback, NULL);
  }

  free((void*)url);
}

void RequestCallback(const char* url,
                     RequestStatus status,
                     const char* html,
                     void* context) {
  assert(url);
  (void)(context);

  // set flag |g_is_fd_reach_limits|
  g_is_fd_reach_limits = (status == Request_Fd_Limit);

  if (g_is_fd_reach_limits) {
    struct PendingRequest* pending_request =
        (struct PendingRequest*)malloc(sizeof(struct PendingRequest));
    pending_request->url = CopyString(url);

    TAILQ_INSERT_TAIL(&g_pending_request_queue, pending_request, _entries);
    ++g_pending_request_count;
    return;
  }

  if (status != Request_Succ || !html) {
    fprintf(stderr, "failed to fetch %s (%d)\n", url, status);
    return;
  }

  BloomFilter* page_url_set = CreateBloomFilter(PAGE_URL_SET_SIZE);
  ProcessUrlContext page_context = {url, page_url_set};

  // sync multi call |ProcessUrl|
  ParseAtagUrls(html, ProcessUrl, &page_context);

  FreeBloomFilter(page_url_set);
}

void YieldUrlConnectionIndexCallback(const char* url,
                                     size_t index,
                                     void* context) {
  assert(url);
  assert(index);
  assert(context);
  FILE* output_file = (FILE*)context;

  fprintf(output_file, "%-6lu %s\n", index, url);
}

void YieldUrlConnectionPairCallback(size_t src, size_t dst, void* context) {
  assert(src);
  assert(dst);
  assert(context);
  FILE* output_file = (FILE*)context;

  fprintf(output_file, "%-6lu %lu\n", src, dst);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: ./crawler URL [OUTPUT_FILE]\n");
    return 1;
  }

  TAILQ_INIT(&g_pending_request_queue);

  g_handled_url_set = CreateBloomFilter(HANDLED_URL_SET_SIZE);
  assert(g_handled_url_set);

  // use |argv[1]| to start crawl tasks
  ProcessUrl(argv[1], NULL);

  // record how-many pending request in previous round
  static size_t previous_pending_request_count = 0;

  while (1) {
    // dispatch crawl tasks
    DispatchLibEvent();

    // break if
    // - there is no pending request
    // - remaining pending requests are the same as in previous round
    if (TAILQ_EMPTY(&g_pending_request_queue) ||
        previous_pending_request_count == g_pending_request_count)
      break;

    // record |previous_pending_request_count|
    previous_pending_request_count = g_pending_request_count;

    // reset flag |g_is_fd_reach_limits|
    g_is_fd_reach_limits = 0;

    // try start pending requests if not |g_is_fd_reach_limits|
    while (!TAILQ_EMPTY(&g_pending_request_queue) && !g_is_fd_reach_limits) {
      struct PendingRequest* request = TAILQ_FIRST(&g_pending_request_queue);
      TAILQ_REMOVE(&g_pending_request_queue, request, _entries);
      --g_pending_request_count;

      // update flag |g_is_fd_reach_limits| here
      Request(request->url, RequestCallback, NULL);

      free((void*)request->url);
      free((void*)request);
    }
  }

  FreeLibEvent();
  FreeBloomFilter(g_handled_url_set);
  AssertBloomFilterNoLeak();

  // discard remaining requests in |g_pending_request_queue|

  // use output_file if exists
  FILE* output_file = argc >= 3 ? fopen(argv[2], "w") : stdout;

  // output results
  YieldUrlConnectionIndex(YieldUrlConnectionIndexCallback, output_file);
  fprintf(output_file, "\n");
  YieldUrlConnectionPair(YieldUrlConnectionPairCallback, output_file);

  if (output_file != stdout)
    fclose(output_file);
  return 0;
}
