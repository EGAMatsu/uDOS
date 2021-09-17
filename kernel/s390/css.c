#include <s390/css.h>

int css_start_channel(struct css_schid schid, struct css_orb *schib) {
    register struct css_schid r1 __asm__("1") = schid;
    int cc                                    = -1;
    __asm__ __volatile__("ssch 0(%1)\n"
                         "ipm %0\n"
                         : "+d"(cc)
                         : "a"(schib), "d"(r1), "m"(schib)
                         : "cc", "memory");
    return cc >> 28;
}

int css_store_channel(struct css_schid schid, void *schib) {
    register struct css_schid r1 __asm__("1") = schid;
    int cc                                    = -1;
    __asm__ __volatile__("stsch 0(%2)\n"
                         "ipm %0"
                         : "+d"(cc), "=m"(*schib)
                         : "a"(schib), "d"(r1)
                         : "cc", "memory");
    return cc >> 28;
}

int css_modify_channel(struct css_schid schid, void *schib) {
    register struct css_schid r1 __asm__("1") = schid;
    int cc                                    = -1;
    __asm__ __volatile__("msch 0(%1)\n"
                         "ipm %0"
                         : "+d"(cc)
                         : "a"(schib), "d"(r1), "m"(schib)
                         : "cc", "memory");
    return cc >> 28;
}

int css_test_channel(struct css_schid schid, struct css_irb *schib) {
    register struct css_schid r1 __asm__("1") = schid;
    int cc                                    = -1;
    __asm__ __volatile__("tsch 0(%2)\n"
                         "ipm %0"
                         : "+d"(cc), "=m"(*schib)
                         : "a"(schib), "d"(r1)
                         : "cc", "memory");
    return cc >> 28;
}