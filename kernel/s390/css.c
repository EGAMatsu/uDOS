#include <s390/css.h>
#include <alloc.h>

static struct css_request_queue g_queue = {0};

int css_start_channel(
    struct css_schid schid,
    struct css_orb *schib)
{
    register struct css_schid r1 __asm__("1") = schid;
    int cc = -1;
    __asm__ __volatile__(
        "ssch 0(%1)\n"
        "ipm %0\n"
        : "+d"(cc)
        : "a"(schib), "d"(r1), "m"(schib)
        : "cc", "memory");
    return cc >> 28;
}

int css_store_channel(
    struct css_schid schid,
    void *schib)
{
    register struct css_schid r1 __asm__("1") = schid;
    int cc = -1;
    __asm__ __volatile__(
        "stsch 0(%2)\n"
        "ipm %0"
        : "+d"(cc), "=m"(*schib)
        : "a"(schib), "d"(r1)
        : "cc", "memory");
    return cc >> 28;
}

int css_modify_channel(
    struct css_schid schid,
    struct css_orb *schib)
{
    register struct css_schid r1 __asm__("1") = schid;
    int cc = -1;
    __asm__ __volatile__(
        "msch 0(%1)\n"
        "ipm %0"
        : "+d"(cc)
        : "a"(schib), "d"(r1), "m"(schib)
        : "cc", "memory");
    return cc >> 28;
}

int css_test_channel(
    struct css_schid schid,
    struct css_irb *schib)
{
    register struct css_schid r1 __asm__("1") = schid;
    int cc = -1;
    __asm__ __volatile__(
        "tsch 0(%2)\n"
        "ipm %0"
        : "+d"(cc), "=m"(*schib)
        : "a"(schib), "d"(r1)
        : "cc", "memory");
    return cc >> 28;
}

/* Creates a new request to ble placed on the css request queue */
struct css_request *css_new_request(
    struct css_schid schid)
{
    struct css_request *req;
    req = kzalloc(sizeof(struct css_request));
    if(req == NULL) {
        kpanic("Out of memory");
    }
    return req;
}

/* Sends a CSS request to the main queue. Note that this releases ownership of
 * the request object since the allocated memory is now freed */
void css_send_request(
    struct css_request *req)
{
    g_queue.requests = krealloc_array(g_queue.requests, g_queue.n_requests + 1, sizeof(struct css_request));
    if(g_queue.requests == NULL) {
        return;
    }
    memcpy(&g_queue.requests[g_queue.n_requests], req,
        sizeof(struct css_request));
    g_queue.n_requests++;

    kfree(req);
    return;
}