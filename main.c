#include <kernel.h>
#include <loadfile.h>
#include <fileio.h>
#include <sifrpc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <debug.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sbv_patches.h>
#include <elf-loader.h>
#include <libpad.h>
#include <libmc.h>
#include <iopcontrol_special.h>
#include "modelname.h"
#include <libcdvd-common.h>

extern unsigned char ps2dev9_irx[];
extern unsigned int size_ps2dev9_irx;
extern unsigned char udptty_standalone_irx[];
extern unsigned int size_udptty_standalone_irx;
extern unsigned char ioprp[];
extern unsigned int size_ioprp;
char romver_buf[16] = { 0 };
char* ConsoleROMVER = romver_buf;

void reload_modules_withiopreset() {
    SifIopReset("", 0);
    SifIopSync();
    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:MCMAN", 0, NULL);
    sbv_patch_disable_prefix_check();
    sbv_patch_enable_lmb();
    scr_printf("Loading required modules\n");
    printf("Loading required modules\n");
}

void loadextramodules() {
    scr_printf("Loading extra modules\n");
    printf("Loading extra modules\n");
    int ret;
    ret = SifExecModuleBuffer(ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
    if (ret < 0) {
        printf("Failed to load ps2dev9.irx: %d\n", ret);
    }


    ret = SifExecModuleBuffer(udptty_standalone_irx, size_udptty_standalone_irx, 0, NULL, NULL);
    if (ret < 0) {
        printf("Failed to load udptty_standalone.irx (err %d)\n", ret);
    }
}

int main() {
    SifInitRpc(0);
    //patch for OSDMenu
    reload_modules_withiopreset();

    //patch end
    //patch for Arcade
#ifdef ARCADE
#if defined(__cplusplus)
    extern "C" {
#endif
        void _ps2sdk_memory_init() {
            while (!SifIopRebootBuffer(ioprp, size_ioprp)) {};
            while (!SifIopSync()) {};
            SifLoadStartModule("rom0:CDVDFSV", 0, NULL, NULL);
        }
#if defined(__cplusplus)
    }
#endif
    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:MCMAN", 0, NULL);
    SifLoadModule("rom0:MCSERV", 0, NULL);
    printf("setting MC type to MC_TYPE_XMC\n");
    mcInit(MC_TYPE_XMC);
    SifLoadModule("rom0:DAEMON", 0, NULL);
    int ret = SifLoadModule("mc0:/ACJVLOAD.irx", 0, NULL);
    if (ret < 0) {
        printf("Failed to load ACJVLOAD (err %d)\n", ret);
    }
#endif
    // patch end
    init_scr();
    scr_setCursor(0);
    loadextramodules();

    int romfd = open("rom0:ROMVER", O_RDONLY);
    if (romfd < 0) {
        printf("Failed to open rom0:ROMVER\n");
        scr_printf("Failed to open rom0:ROMVER\n");
        while (1) asm("nop");
    }

    int bytes = read(romfd, romver_buf, sizeof(romver_buf) - 1);
    close(romfd);

    if (bytes <= 0) {
        printf("Failed to read rom0:ROMVER\n");
        scr_printf("Failed to read rom0:ROMVER\n");
        while (1) asm("nop");
    }
    scr_clear();
    // patch for screens with overscan
    scr_printf("\n");
#ifdef R4D
    printf("ROM Version Checker for R4D\n");
    scr_printf("ROM Version Checker for R4D\n");
#else
    printf("ROM Version Checker\n");
    scr_printf("ROM Version Checker\n");
#endif
    printf("by slimpuggamer");
    scr_printf("by slimpuggamer");
    scr_printf("\n");
    scr_printf("\n");
    printf("\n");
    printf("ROMVER: %s\n", romver_buf);
    scr_printf("ROMVER: %s\n", romver_buf);
    char version_str[5] = { 0 };
    memcpy(version_str, romver_buf, 4);
    int is_dex = (romver_buf[5] == 'D' || (romver_buf[4] == 'T' && romver_buf[5] == 'Z'));
    int version_num = atoi(version_str);
    int bootelffound = open("mc0:/BOOT/BOOT.ELF", O_RDONLY);
    int fd;
    int is_protkrnl = open("rom0:OSBROWS", O_RDONLY);

    if (romver_buf[4] == 'T') {
        if (romver_buf[5] == 'Z') {
            printf("Console Type: Arcade\n");
            scr_printf("Console Type: Arcade\n");
        }
        else {
            printf("Console Type: DEX (TOOL)\n");
            scr_printf("Console Type: DEX (TOOL)\n");
        }
    }
    else if (romver_buf[5] == 'D') {
        printf("Console Type: DEX (TEST)\n");
        scr_printf("Console Type: DEX (TEST)\n");
    }
    else if (version_num == 250) {
        printf("Console Type: PS2 TV\n");
        scr_printf("Console Type: PS2 TV\n");
    }
    else if (romver_buf[5] == 'C') {
        printf("Console Type: Retail (CEX)\n");
        scr_printf("Console Type: Retail (CEX)\n");
    }
    else {
        printf("Console Type: Unknown\n");
        scr_printf("Console Type: Unknown\n");
    }
    char* r = strchr("AECJT", romver_buf[4]);
    printf("Region: %s\n", r ? (r[0] == 'A' ? "America" : r[0] == 'E' ? "Europe" : r[0] == 'C' ? "China" : r[0] == 'J' ? "Japan" : "TOOL") : "Unknown");
    scr_printf("Region: %s\n", r ? (r[0] == 'A' ? "America" : r[0] == 'E' ? "Europe" : r[0] == 'C' ? "China" : r[0] == 'J' ? "Japan" : "TOOL") : "Unknown");
    
#if ARCADE
#else
    ModelNameInit();
    const char* model = ModelNameGet();
    scr_printf("Model: %s\n", model);
    printf("Model: %s\n", model);
#endif

    fd = open("mc0:/SYS-CONF/PS2BBL.INI", O_RDONLY);
    if (fd < 0) {
        fd = open("mc1:/SYS-CONF/PS2BBL.INI", O_RDONLY);
    }

    if (fd >= 0) {
        printf("PS2BBL installed: yes\n");
        scr_printf("PS2BBL installed: yes\n");
        close(fd);
    }
    else {
        fd = open("mc0:/SYS-CONF/FREEMCB.CNF", O_RDONLY);
        if (fd < 0) {
            fd = open("mc1:/SYS-CONF/FREEMCB.CNF", O_RDONLY);
        }

        if (fd < 0) {
            printf("FMCB installed: no\n");
            scr_printf("FMCB installed: no\n");
        }
        else {
            printf("FMCB installed: yes\n");
            scr_printf("FMCB installed: yes\n");
            close(fd);
        }
    }
    if (is_dex) {
        printf("MechaPwnable: no\n");
        scr_printf("MechaPwnable: no\n");
    }
    else if (version_num >= 170) {
        printf("MechaPwnable: yes\n");
        scr_printf("MechaPwnable: yes\n");
    }
    else {
        printf("MechaPwnable: no\n");
        scr_printf("MechaPwnable: no\n");
    }
    if (is_protkrnl >= 0) {
        printf("ProtoPwnable: yes\n");
        scr_printf("ProtoPwnable: yes\n");
        scr_printf("\n");
        close(is_protkrnl);
    }
    else {
        printf("ProtoPwnable: no\n");
        scr_printf("ProtoPwnable: no\n");
        scr_printf("\n");
    }
    // DESR check start //
    int psxver = open("rom0:PSXVER", O_RDONLY);
    if (psxver >= 0) {
        printf("DESR PS2: it is recommended to set LK_Auto_E1 to something like wLaunchELF or OPL\n");
        scr_printf("DESR PS2: it is recommended to set LK_Auto_E1 to something like wLaunchELF\n");
        scr_printf("or OPL")
        close(psxver);
    }
    // DESR check end //
    else {
        // Arcade check start //
        if (romver_buf[4] == 'T' && romver_buf[5] == 'Z') {
            printf("Arcade PS2: Use wLaunchELF ISR COH\n");
            scr_printf("Arcade PS2: Use wLaunchELF ISR COH\n");
        }
        // Arcade check end //
        else {
            // DEX check start //
            if (is_dex) {
                printf("DEX PS2: FMCB may not work use PS2BBL\n");
                scr_printf("DEX PS2: FMCB may not work use PS2BBL\n");
            }

            // DEX check end //
            // bootrom patch check start //
            else {
                if (version_num >= 230) {
                    printf("FMCB/PS2BBL patched. Use OpenTuna if you want to start FMCB/PS2BBL.\n");
                    scr_printf("FMCB/PS2BBL patched. Use OpenTuna if you want to start FMCB/PS2BBL.\n");
                }
                else {
                    printf("FMCB/PS2BBL not patched. You can install it normally.\n");
                    scr_printf("FMCB/PS2BBL not patched. You can install it normally.\n");
                }
            }
            // bootrom patch check end //
        }
    }

    //scr_printf("exiting to OSDSYS in 2 minutes\n");
    //sleep(120);
    //LoadELFFromFile("rom0:OSDSYS", 0, NULL);
    //return 0;
    SifLoadModule("rom0:PADMAN", 0, NULL);
    padInit(0);
    static char padBuf[2][256] __attribute__((aligned(64)));
    for (int port = 0; port < 2; port++) {
        if (padPortOpen(port, 0, padBuf[port]) == 0) {
        }
    }
    struct padButtonStatus padinfo;
    printf("Press X to exit\n");
    scr_printf("Press X to exit\n");


    for (;;) {
        for (int port = 0; port < 2; port++) {
            if (padGetState(port, 0) == PAD_STATE_STABLE) {
                if (padRead(port, 0, &padinfo) > 0) {
                    if (!(padinfo.btns & PAD_CROSS)) {
                        printf("Exiting to OSDSYS\n");
                        scr_printf("Exiting to OSDSYS\n");
                        // bit of a hack for OpenTuna users so they don't need to re-exploit after exit
                        if (bootelffound >= 0) {
                            close(bootelffound);
                            LoadELFFromFile("mc0:/BOOT/BOOT.ELF", 0, NULL);
                        }
                        else {
                            bootelffound = open("mc1:/BOOT/BOOT.ELF", O_RDONLY);
                            if (bootelffound >= 0) {
                                close(bootelffound);
                                LoadELFFromFile("mc1:/BOOT/BOOT.ELF", 0, NULL);
                            }
                            else {
                                LoadELFFromFile("rom0:OSDSYS", 0, NULL);
                            }
                        }

                        return 0;
                    }
                }
            }
        }
        usleep(16000);
    }

    return 0;

}




