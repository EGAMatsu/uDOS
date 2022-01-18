#include <css.h>
#include <cpu.h>
#include <mm.h>
#include <printf.h>
#include <panic.h>

static struct css_request_queue g_queue = {0};

/* Creates a new request to be placed on the css request queue */
struct css_request *CssNewRequest(struct css_device *dev, size_t n_ccws)
{
    struct css_request *req;
    req = MmAllocateZero(sizeof(struct css_request));
    if(req == NULL) {
        KePanic("Out of memory");
    }

    req->n_ccws = n_ccws;
    req->ccws = MmAllocatePhysical(req->n_ccws * sizeof(req->ccws[0]), 8);
    if(req->ccws == NULL) {
        KePanic("Out of memory");
    }
    KeSetMemory(req->ccws, 0, req->n_ccws * sizeof(req->ccws[0]));

    req->dev = dev;
    return req;
}

void CssDestroyRequest(struct css_request *req)
{
    MmFree(req->ccws);
    MmFree(req);
    return;
}

/* Sends a CSS request to the main queue. Note that this releases ownership of
 * the request object since the allocated memory is now freed */
void CssSendRequest(struct css_request *req)
{
    g_queue.requests = MmReallocateArray(g_queue.requests, g_queue.n_requests + 1, sizeof(struct css_request));
    if(g_queue.requests == NULL) {
        return;
    }

    KeDebugPrint("css:%i:%i: New channel program with %zu words\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num, (size_t)req->n_ccws);
    KeCopyMemory(&g_queue.requests[g_queue.n_requests], req, sizeof(struct css_request));
    g_queue.n_requests++;
    return;
}

/* The req parameter is a pointer to an empty request structure, which will then
 * be filled with an element from the queue
 *
 * If any part of the process fails the function will return and the queue
 * will be left unaffected, the caller can then retry the request or "drop"
 * the request from the queue */
int CssPerformRequest(struct css_request *req)
{
    /* Used for catching potential PC exceptions */
    PSW_DEFAULT_TYPE saved_psw;
    const PSW_DECL(catch_pc_psw, 0x444, PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_WAIT_STATE);
    
    int timeout, r;

    /* Set the PC handlers (and save the old one in saved_psw) */
#if (MACHINE > 390u)
    KeCopyMemory(&saved_psw, (void *)PSA_FLCEPNPSW, sizeof(saved_psw));
    KeCopyMemory((void *)PSA_FLCEPNPSW, &catch_pc_psw, sizeof(catch_pc_psw));
#else
    KeCopyMemory(&saved_psw, (void *)PSA_FLCPNPSW, sizeof(saved_psw));
    KeCopyMemory((void *)PSA_FLCPNPSW, &catch_pc_psw, sizeof(catch_pc_psw));
#endif

    if(g_queue.n_requests == 0) {
        return 1;
    }
    
    KeCopyMemory(req, &g_queue.requests[g_queue.n_requests - 1], sizeof(struct css_request));
    
    KeSetMemory(&req->dev->irb, 0, sizeof(struct css_irb));
    KeSetMemory(&req->dev->schib, 0, sizeof(struct css_schib));
    
    req->dev->orb.cpa_addr = (uint32_t)&req->ccws[0];
    *((volatile uint32_t *)PSA_FLCCAW) = (uint32_t)&req->ccws[0];

    /* Test that the device is actually online */
    if(req->flags & CSS_REQUEST_MODIFY != 0) {
        r = CssTestChannel(req->dev->schid, &req->dev->irb);
        if(r == CSS_STATUS_NOT_PRESENT && !(req->flags & CSS_REQUEST_IGNORE_CC)) {
            KeDebugPrint("css:%i:%i: Test channel (modify) failed\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num);
            return -1;
        }

        r = CssModifyChannel(req->dev->schid, &req->dev->orb);
        if(r == CSS_STATUS_NOT_PRESENT && !(req->flags & CSS_REQUEST_IGNORE_CC)) {
            KeDebugPrint("css:%i:%i: Modify channel failed\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num);
            return -1;
        }
    }

    r = CssTestChannel(req->dev->schid, &req->dev->irb);
    if(r == CSS_STATUS_NOT_PRESENT && !(req->flags & CSS_REQUEST_IGNORE_CC)) {
        KeDebugPrint("css:%i:%i: Test channel failed\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num);
        return -1;
    }

    /* Wait for attention */
    if(req->flags & CSS_REQUEST_WAIT_ATTENTION) {
        KeDebugPrint("css:%i:%i: Waiting for attention\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num);
        
        /* Loop until the attention bit is set or the device fails */
        while(!(req->dev->irb.scsw.device_status & CSS_SCSW_DS_ATTENTION)) {
            HwWaitIO();
            r = CssTestChannel(req->dev->schid, &req->dev->irb);
            if(r == CSS_STATUS_NOT_PRESENT && !(req->flags & CSS_REQUEST_IGNORE_CC)) {
                KeDebugPrint("css:%i:%i: Test channel failed\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num);
                return -1;
            }
        }
    }

    r = CssStartChannel(req->dev->schid, &req->dev->orb);
    if(r == CSS_STATUS_NOT_PRESENT && !(req->flags & CSS_REQUEST_IGNORE_CC)) {
        KeDebugPrint("css:%i:%i: Start channel failed\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num);
        return -1;
    }
    
    /* Block for IO here */
    HwWaitIO();
    
    r = CssTestChannel(req->dev->schid, &req->dev->irb);
    if(r == CSS_STATUS_NOT_PRESENT && !(req->flags & CSS_REQUEST_IGNORE_CC)) {
        KeDebugPrint("css:%i:%i: Test channel failed\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num);
        return -1;
    }

    if(req->dev->irb.scsw.cpa_addr != (uint32_t)&req->ccws[req->n_ccws]) {
        KeDebugPrint("css:%i:%i: Command chain not completed\r\n", (int)req->dev->schid.id, (int)req->dev->schid.num);
        return -1;
    }
    g_queue.n_requests--;
    return 0;
catch_exception:
    /* Restore back the old pc handler PSW */
#if (MACHINE > 390u)
    KeCopyMemory((void *)PSA_FLCEPNPSW, &saved_psw, sizeof(saved_psw));
#else
    KeCopyMemory((void *)PSA_FLCPNPSW, &saved_psw, sizeof(saved_psw));
#endif
    /* Return error code due to catched PC */
    return -1;
}

#include <x3390.h>
#include <terminal.h>
/* Probe for devices in the channel subsystem */
int ModProbeCss(void)
{
    struct css_device dev = {0};
    struct css_senseid sensebuf;

    for(dev.schid.id = 1; dev.schid.id < 2; dev.schid.id++) {
#if defined(DEBUG)
        KeDebugPrint("css: Checking subchannel %i\r\n", (int)dev.schid.id);
#endif
        for(dev.schid.num = 0; dev.schid.num < 80; dev.schid.num++) {
            struct css_request *req;
            int r;

            KeSetMemory(&sensebuf, 0, sizeof(sensebuf));
            req = CssNewRequest(&dev, 1);
            req->ccws[0].cmd = CSS_CMD_SENSE_ID;
            CSS_SET_ADDR(&req->ccws[0], &sensebuf);
            req->ccws[0].flags = 0;
            req->ccws[0].length = (uint16_t)sizeof(sensebuf);
            req->dev->orb.flags = 0x0080FF00;

            CssSendRequest(req);
            r = CssPerformRequest(req);
            CssDestroyRequest(req);

            if(r != CSS_STATUS_OK) {
                continue;
            }

            KeDebugPrint("css: Device present @ %i:%i\r\n", (int)dev.schid.id, (int)dev.schid.num);
            
            KeDebugPrint("Type: %x, Model: %x\n", (unsigned int)sensebuf.cu_type, (unsigned int)sensebuf.cu_model);

            switch(sensebuf.cu_type) {
            case 0x2305:
            case 0x2311:
            case 0x2314:
            case 0x3330:
            case 0x3340:
            case 0x3350:
            case 0x3375:
            case 0x3380:
            case 0x3990:
            case 0x9345:
                KeDebugPrint("Probed %x disk\r\n", (unsigned int)sensebuf.cu_type);
                ModAdd3390Device(dev.schid, &sensebuf);
                break;
            case 0x1052:
            case 0x2703:
                KeDebugPrint("Probed %x console\r\n", (unsigned int)sensebuf.cu_type);
                ModAdd2703Device(dev.schid, &sensebuf);
                break;
            case 0x3270:
            case 0x3287:
                KeDebugPrint("Probed %x terminal\r\n", (unsigned int)sensebuf.cu_type);
                ModAdd3270Device(dev.schid, &sensebuf);
                break;
            default:
                break;
            }
        }
    }
    return 0;
}
