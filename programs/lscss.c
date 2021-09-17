/*
 * List channel subsystem devices
 */

#include <s390/css.h>
#include <printf.h>

int main(int argc, char **argv) {
    struct css_schid schid = {1, 0x0000};
    struct css_schib schib = {0};
    int r;

    if (argc <= 1) {
        kprintf("Usage: lscss [id]\n");
        return -1;
    }

    schid.num = (unsigned)atoi(argv[1]);

    r = css_store_channel(schid, &schib);
    if (r != 0) {
        kprintf("Invalid channel\n");
        return -2;
    }

    schib.pmcw.flags |= CSS_PMCW_ENABLED(1);
    r = css_modify_channel(schid, &schib);
    if (r != 0) {
        kprintf("Modify channel error\n");
        return -1;
    }

    unsigned char data[255] = {0};
    struct css_ccw1 sense   = {0};
    sense.cmd               = 0x04;
    sense.length            = sizeof(data);
    sense.addr              = (uint32_t)&data;

    struct css_orb orb = {0};
    orb.flags |= CSS_ORB_FORMAT_2_IDAW_CTRL(1);
    orb.flags |= CSS_ORB_PREFETCH_CTRL(1);
    // orb.flags |= CSS_ORB_LPM_CTRL(0xff);
    orb.flags |= CSS_ORB_FORMAT_CTRL(1);
    orb.prog_addr = (uint32_t)&sense;
    r             = css_start_channel(schid, &orb);
    if (r != 0) {
        kprintf("Start channel error\n");
        return -1;
    }

    struct css_irb irb = {0};
    irb.scsw.cpa_addr  = (uint32_t)&sense;
    css_test_channel(schid, &irb);
    if (r != 0) {
        kprintf("Test channel error\n");
        return -1;
    }

    kprintf("*** Subchannel %x ***\n", (unsigned)atoi(argv[1]));
    kprintf("pmcw.int_param         %x\n", schib.pmcw.int_param);
    kprintf("pmcw.dev_num           %x\n", schib.pmcw.dev_num);
    kprintf("pmcw.lpm               %x\n", schib.pmcw.lpm);
    kprintf("pmcw.pnom              %x\n", schib.pmcw.pnom);
    kprintf("pmcw.lpum              %x\n", schib.pmcw.lpum);
    kprintf("pmcw.pim               %x\n", schib.pmcw.pim);
    kprintf("pmcw.mbi               %x\n", schib.pmcw.mbi);
    kprintf("pmcw.pom               %x\n", schib.pmcw.pom);
    kprintf("pmcw.pam               %x\n", schib.pmcw.pam);
    kprintf("pmcw.flags             %s, %s, ISC: %x, %s, %s, %s, %s, %x\n",
        schib.pmcw.flags & CSS_PMCW_DNV(1) ? "DeviceNumberValid"
                                           : "InvalidDeviceNumber",
        schib.pmcw.flags & CSS_PMCW_ENABLED(1) ? "Enabled" : "Disabled",
        schib.pmcw.flags & CSS_PMCW_ISC(2),
        schib.pmcw.flags & CSS_PMCW_LIMIT(1) ? "Limited" : "Unlimited",
        schib.pmcw.flags & CSS_PMCW_MM_ENABLE(1) ? "MeasureModeEnable"
                                                 : "MeasureModeDisable",
        schib.pmcw.flags & CSS_PMCW_MULTIPATH_MODE(1) ? "MultiPath"
                                                      : "NoMultiPath",
        schib.pmcw.flags & CSS_PMCW_TIMING(1) ? "TimingFacility"
                                              : "NoTimingFacility",
        (unsigned)schib.pmcw.flags);
    kprintf("pmcw.chpid             %x %x %x %x %x %x %x\n",
        (unsigned)schib.pmcw.chpid[0], (unsigned)schib.pmcw.chpid[1],
        (unsigned)schib.pmcw.chpid[2], (unsigned)schib.pmcw.chpid[3],
        (unsigned)schib.pmcw.chpid[4], (unsigned)schib.pmcw.chpid[5],
        (unsigned)schib.pmcw.chpid[6], (unsigned)schib.pmcw.chpid[7]);
    kprintf("pmcw.zero              %x %x %x %x\n",
        (unsigned)schib.pmcw.zero[0], (unsigned)schib.pmcw.zero[1],
        (unsigned)schib.pmcw.zero[2], (unsigned)schib.pmcw.last_flags);
    kprintf("scsw.flags             %x\n", schib.scsw.flags);
    kprintf("scsw.ccw_addr          %x\n", schib.scsw.cpa_addr);
    kprintf("scsw.device_status     %x\n", schib.scsw.device_status);
    kprintf("scsw.subchannel_status %x\n", schib.scsw.subchannel_status);
    kprintf("scsw.count             %x\n", schib.scsw.count);
    kprintf("mb_addr                %x\n", (unsigned)schib.mb_addr);
    kprintf("md_data                %x %x %x\n", (unsigned)schib.md_data[0],
        (unsigned)schib.md_data[1], (unsigned)schib.md_data[2]);

    for (size_t i = 0; i < sizeof(data); i++) {
        kprintf("%zu ", (size_t)data[i]);
    }
    kprintf("\n");
    return 0;
}