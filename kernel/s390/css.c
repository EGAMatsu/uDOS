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
    struct css_schib *schib)
{
    register struct css_schid r1 __asm__("1") = schid;
    int cc = -1;
    __asm__ __volatile__(
        "msch 0(%2)\n"
        "ipm %0"
        : "+d"(cc), "=m"(*schib)
        : "a"(schib), "d"(r1)
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

/* Creates a new request to be placed on the css request queue */
struct css_request *css_new_request(
    struct css_device *dev,
    size_t n_ccws)
{
    struct css_request *req;
    req = kzalloc(sizeof(struct css_request));
    if(req == NULL) {
        kpanic("Out of memory");
    }

    req->n_ccws = n_ccws;
    req->ccws = kzalloc_array(req->n_ccws, sizeof(req->ccws[0]));
    if(req->ccws == NULL) {
        kpanic("Out of memory");
    }

    req->dev = dev;
    return req;
}

void css_destroy_request(
    struct css_request *req)
{
    kfree(req->ccws);
    kfree(req);
    return;
}

/* Sends a CSS request to the main queue. Note that this releases ownership of
 * the request object since the allocated memory is now freed */
void css_send_request(
    struct css_request *req)
{
    g_queue.requests = krealloc_array(g_queue.requests, g_queue.n_requests + 1,
        sizeof(struct css_request));
    if(g_queue.requests == NULL) {
        return;
    }

    kprintf("css:%i:%i: New channel program with %zu words\n",
        (int)req->dev->schid.id, (int)req->dev->schid.num, (size_t)req->n_ccws);
    memcpy(&g_queue.requests[g_queue.n_requests], req,
        sizeof(struct css_request));
    g_queue.n_requests++;
    return;
}

/* The req parameter is a pointer to an empty request structure, which will then
 * be filled with an element from the queue
 *
 * If any part of the process fails the function will return and the queue
 * will be left unaffected, the caller can then retry the request or "drop"
 * the request from the queue */
int css_do_request(
    struct css_request *req)
{
    int r;

    if(g_queue.n_requests == 0) {
        return 1;
    }
    
    memcpy(req, &g_queue.requests[g_queue.n_requests - 1],
        sizeof(struct css_request));
    
    memset(&req->dev->irb, 0, sizeof(struct css_irb));
    
    req->dev->orb.cpa_addr = (uint32_t)&req->ccws[0];
    *((volatile uint32_t *)S390_FLCCAW) = (uint32_t)&req->ccws[0];

    /* Do the actual subchannel process part */
    if(req->flags & CSS_REQUEST_MODIFY != 0) {
        kprintf("css:%i:%i: Modify channel\n", (int)req->dev->schid.id,
            (int)req->dev->schid.num);
        r = css_modify_channel(req->dev->schid, &req->dev->schib);
        if(r == CSS_STATUS_NOT_PRESENT) {
            return -1;
        }
    }

    kprintf("css:%i:%i: Test channel\n", (int)req->dev->schid.id,
        (int)req->dev->schid.num);
    r = css_test_channel(req->dev->schid, &req->dev->irb);
    if(r == CSS_STATUS_NOT_PRESENT) {
        return -1;
    }

    kprintf("css:%i:%i: Start channel\n", (int)req->dev->schid.id,
        (int)req->dev->schid.num);
    r = css_start_channel(req->dev->schid, &req->dev->orb);
    if(r == CSS_STATUS_NOT_PRESENT) {
        return -1;
    }

    kprintf("css:%i:%i: Wait for I/O\n", (int)req->dev->schid.id,
        (int)req->dev->schid.num);
    s390_wait_io();

    kprintf("css:%i:%i: Test channel\n", (int)req->dev->schid.id,
        (int)req->dev->schid.num);
    r = css_test_channel(req->dev->schid, &req->dev->irb);
    if(r == CSS_STATUS_NOT_PRESENT) {
        return -1;
    }

    if(req->dev->irb.scsw.cpa_addr != (uint32_t)&req->ccws[req->n_ccws]) {
        kprintf("Command chain not completed\n");
        return -1;
    }

    g_queue.n_requests--;
    return 0;
}

/* Probe for devices in the channel subsystem */
int css_probe(
    void)
{
    struct css_schid schid;
    struct css_senseid sensebuf;

    /*
    for(schid.num = 1; schid.num < 255; schid.num++) {
        kprintf("css: Checking subchannel %i\n", (int)schid.num);
        for(schid.id = 1; schid.id < 255; schid.id++) {
            struct css_device dev = {0};
            struct css_request *req;
            int r;

            dev.schid = schid;

            req = css_new_request(&dev, 1);

            req->ccws[0].cmd = CSS_CMD_SENSE_ID;
            req->ccws[0].addr = (uint32_t)&sensebuf;
            req->ccws[0].flags = CSS_CCW_SLI | CSS_CCW_CC;
            req->ccws[0].length = (uint16_t)sizeof(sensebuf);
            req->dev->orb.flags = 0x0080FF00;

            css_send_request(req);
            r = css_do_request(req);
            css_destroy_request(req);

            if(r == CSS_STATUS_NOT_PRESENT) {
                kprintf("css: Device present @ %i:%i\n", (int)dev.schid.id,
                    (int)dev.schid.num);
            }
        }
    }
    */
}