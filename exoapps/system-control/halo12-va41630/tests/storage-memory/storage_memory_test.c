#include <stdio.h>
#include "update/update_command.h"
#include "storage/storage_memory.h"
#include "update_helper.h"
#include "utils/macro_tools.h"
#include "trace/trace.h"
#include "iacm/iacm.h"
#include "storage/mem_comp_map.h"

#define TEST_PAGE_SIZE 512
#define UPDATE_SIZE 0x40000
static uint32_t update_comp = 0;
static uint8_t update_buf[UPDATE_SIZE];
int main(void)
{
    sm_init();
    iacm_init();
    eh_init();
    uc_init();
    trcInit();
    TraceInfo(TrcMsgAlways,"Starting Storage Test\n",0,0,0,0,0,0);

#ifdef __x86_64__
    int fd = open("/opt/test-builds/update.bin", O_RDONLY);
    if(read(fd, update_buf, UPDATE_SIZE) == 0){
        printf("Error reading update file\n");
        return -1;
    }

    struct stat file_info;
    stat("/opt/test-builds/update.bin", &file_info);
    TraceInfo(TrcMsgAlways, "File size %d\n", file_info.st_size,0,0,0,0,0);
#endif


    uc_prepare(file_info.st_size);

    int total_size = file_info.st_size;
    int pages2wr = total_size / TEST_PAGE_SIZE;
    int remaining = total_size % TEST_PAGE_SIZE;

    for(int page = 0; page < pages2wr; page++) {
        uc_upload(&update_buf[page * TEST_PAGE_SIZE], TEST_PAGE_SIZE, &update_comp);
    }

    uc_upload(&update_buf[pages2wr * TEST_PAGE_SIZE], remaining, &update_comp);

    uh_update_software(UPDATE_MOVE_NONEXE,&update_comp);

    uh_install_app(0);

    return 0;
}