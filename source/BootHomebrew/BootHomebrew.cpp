#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wpad.h>
#include <vector>
#include <string>
#include <ogc/es.h>

#include "FileOperations/fileops.h"
#include "DiskOperations/di2.h"
#include "Language/gettext.h"
#include "network/networkops.h"
#include "main.h"
#include "sys.h"

#define EXECUTE_ADDR	((u8 *) 0x92000000)
#define BOOTER_ADDR		((u8 *) 0x93000000)
#define ARGS_ADDR		((u8 *) 0x93200000)

#define GC_DOL_MAGIC    "gchomebrew dol"
#define GC_MAGIC_BUF    (char *) 0x807FFFE0
#define GC_DOL_BUF      (u8 *)0x80800000
#define BC              0x0000000100000100ULL

extern const u8 app_booter_bin[];
extern const u32 app_booter_bin_size;

typedef void (*entrypoint) (void);
extern "C" { void __exception_closeall(); }

static u8 *homebrewbuffer = EXECUTE_ADDR;
static u32 homebrewsize = 0;
static std::vector<std::string> Arguments;

void AddBootArgument(const char * argv)
{
    std::string arg(argv);
    Arguments.push_back(arg);
}

int CopyHomebrewMemory(u8 *temp, u32 pos, u32 len)
{
    homebrewsize += len;
    memcpy((homebrewbuffer)+pos, temp, len);

    return 1;
}

void FreeHomebrewBuffer()
{
    homebrewbuffer = EXECUTE_ADDR;
    homebrewsize = 0;
    Arguments.clear();
}

int LoadHomebrew(const char * filepath)
{
    u8 *buffer = NULL;
    u64 filesize = 0;
    int ret = LoadFileToMemWithProgress(tr("Loading file:"), filepath, &buffer, &filesize);
    if(ret < 0)
     return ret;

    FreeHomebrewBuffer();

    ret = CopyHomebrewMemory(buffer, 0, filesize);
    if(buffer) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

static int SetupARGV(struct __argv * args)
{
    if(!args)
        return -1;

    bzero(args, sizeof(struct __argv));
    args->argvMagic = ARGV_MAGIC;

    u32 stringlength = 1;

    /** Append Arguments **/
    for(u32 i = 0; i < Arguments.size(); i++)
    {
        stringlength += Arguments[i].size()+1;
    }

    args->length = stringlength;
    args->commandLine = (char*) malloc(args->length);
    if (!args->commandLine)
        return -1;

    u32 argc = 0;
    u32 position = 0;

    /** Append Arguments **/
    for(u32 i = 0; i < Arguments.size(); i++)
    {
        strcpy(&args->commandLine[position], Arguments[i].c_str());
        position += Arguments[i].size()+1;
        argc++;
    }

    args->argc = argc;

    args->commandLine[args->length - 1] = '\0';
    args->argv = &args->commandLine;
    args->endARGV = args->argv + 1;

    Arguments.clear();

    return 0;
}

int BootGameCubeHomebrew()
{
    if(homebrewsize == 0)
        return -1;

    ExitApp();

    static tikview view ATTRIBUTE_ALIGN(32);

    DI2_Init();
    DI2_Reset();
    DI2_ReadDiscID((u64 *) 0x80000000);
    DI2_Mount();

	strcpy(GC_MAGIC_BUF, GC_DOL_MAGIC);
	DCFlushRange(GC_MAGIC_BUF, 32);
    memcpy(GC_DOL_BUF, homebrewbuffer, homebrewsize);
	DCFlushRange(GC_DOL_BUF, homebrewsize);
	*(vu32 *) 0xCC003024 |= 0x07;

	ES_GetTicketViews(BC, &view, 1);
    int ret = ES_LaunchTitle(BC, &view);
    if(ret < 0)
        Sys_BackToLoader();

    return ret;
}

int BootHomebrew()
{
    if(homebrewsize == 0)
        return -1;

    ExitApp();

    if(IOS_GetVersion() != Settings.EntraceIOS)
        IOS_ReloadIOS(Settings.EntraceIOS);

    struct __argv args;
    SetupARGV(&args);

    u32 cpu_isr;

	memcpy(BOOTER_ADDR, app_booter_bin, app_booter_bin_size);
	DCFlushRange(BOOTER_ADDR, app_booter_bin_size);

    entrypoint entry = (entrypoint) BOOTER_ADDR;

	if (args.argvMagic == ARGV_MAGIC)
	{
		memmove(ARGS_ADDR, &args, sizeof(args));
		DCFlushRange(ARGS_ADDR, sizeof(args));
	}

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable (cpu_isr);
    __exception_closeall();
    entry();
    _CPU_ISR_Restore (cpu_isr);

    Sys_BackToLoader();

    return 0;
}
