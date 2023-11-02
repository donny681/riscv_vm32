#include <stdio.h>
#include "include/cpu.h"
#include <elog.h>
// #define LOG_TAG    "main"
#include <limits.h>
#include "include/monitor.h"
#include "cpu.h"
int main(int argc, const char** argv)
{

    /* close printf buffer */
    setbuf(stdout, NULL);
    /* initialize EasyLogger */
    elog_init();
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    elog_set_text_color_enabled(true);
    elog_start();
    // test_elog();

    // uint64_t imm = 0xffa00113;
    cpu_init(&cpu);           // init cpu
    dram_init(&cpu.bus.dram); // init dram
    init_inst_funct();
    // udp_init();
    if(argc > 1){
       read_file(argv[1]);
       cmd_c();
       return 0;
    }
    while (1)
    {
        scanf("%s", opt);
        switch (opt[0])
        {
        case 'q':
            return 0;
        case 'c':
            log_d("run c");
            cmd_c();
            break;
        case 'h':
            cmd_h();
            break;
        case 'r':
            if (opt[1] == 'e')
                cmd_re();
            else
                cmd_r();
            break;
        case 's':
            cmd_s();
            break;
        default:
            puts("invalid command");
            puts("input \'h\' for help");
            break;
        }
    }

    return 0;
}
