#include "main.h"
#include "configuration.h"
#include "m68000.h"
#include "sysdeps.h"
#include "tmc.h"


/* NeXT TMC emulation */

#define TMC_ADB_ADDR_MASK	0x02208000

#define TMC_REGS_MASK		0x0000FFFF

/* TMC registers */

struct {
	Uint32 scr1;
} tmc;

/* Additional System Control Register for Turbo systems:
 * -------- -------- -------- -----xxx  bits 0:2   --> cpu speed
 * -------- -------- -------- --xx----  bits 4:5   --> main memory speed
 * -------- -------- -------- xx------  bits 6:7   --> video memory speed
 * -------- -------- ----xxxx --------  bits 8:11  --> cpu revision
 * -------- -------- xxxx---- --------  bits 12:15 --> cpu type
 * xxxx---- -------- -------- --------  bits 28:31 --> slot id
 * ----xxxx xxxxxxxx -------- ----x---  all other bits: 1
 *
 * cpu speed:       7 = 33MHz?
 * main mem speed:  0 = 60ns, 1 = 70ns, 2 = 80ns, 3 = 100ns
 * video mem speed: 3 on all Turbo systems (100ns)?
 * cpu revision:    0xF = rev 0
 *                  0xE = rev 1
 *                  0xD = rev 2
 *                  0xC - 0x0: rev 3 - 15
 * cpu type:        4 = NeXTstation turbo monochrome
 *                  5 = NeXTstation turbo color
 *                  8 = NeXTcube turbo
 */

#define TURBOSCR_FMASK   0x0FFF0F08

void TurboSCR1_Reset(void) {
	Uint8 memory_speed;
	Uint8 cpu_speed = 0x07; // 33 MHz
	
	if (ConfigureParams.System.nCpuFreq<20) {
		cpu_speed = 4;
	} else if (ConfigureParams.System.nCpuFreq<25) {
		cpu_speed = 5;
	} else if (ConfigureParams.System.nCpuFreq<33) {
		cpu_speed = 6;
	} else {
		cpu_speed = 7;
	}
	
	switch (ConfigureParams.Memory.nMemorySpeed) {
		case MEMORY_120NS: memory_speed = 0xF0; break;
		case MEMORY_100NS: memory_speed = 0xF0; break;
		case MEMORY_80NS: memory_speed = 0xA0; break;
		case MEMORY_70NS: memory_speed = 0x50; break;
		case MEMORY_60NS: memory_speed = 0x00; break;
		default: Log_Printf(LOG_WARN, "Turbo SCR1 error: unknown memory speed\n"); break;
	}
	tmc.scr1 = ((memory_speed&0xF0)|(cpu_speed&0x07));
	if (ConfigureParams.System.nMachineType == NEXT_CUBE040)
		tmc.scr1 |= 0x8000;
	else if (ConfigureParams.System.bColor) {
		tmc.scr1 |= 0x5000;
	} else {
		tmc.scr1 |= 0x4000;
	}
	tmc.scr1 |= TURBOSCR_FMASK;
}



/* Register read/write functions */

Uint8 tmc_ill_read(void) {
	Log_Printf(LOG_WARN, "[TMC] Illegal read!\n");
	return 0;
}

void tmc_ill_write(Uint8 val) {
	Log_Printf(LOG_WARN, "[TMC] Illegal write!\n");
}

Uint8 tmc_unimpl_read(void) {
	Log_Printf(LOG_WARN, "[TMC] Unimplemented read!\n");
	return 0;
}

void tmc_unimpl_write(Uint8 val) {
	Log_Printf(LOG_WARN, "[TMC] Unimplemented write!\n");
}

/* SCR1 */
Uint8 tmc_scr1_read0(void) {
	Log_Printf(LOG_WARN,"[TMC] SCR1 read at $0x2200000 PC=$%08x\n",m68k_getpc());
	return (tmc.scr1>>24);
}
Uint8 tmc_scr1_read1(void) {
	Log_Printf(LOG_WARN,"[TMC] SCR1 read at $0x2200001 PC=$%08x\n",m68k_getpc());
	return (tmc.scr1>>16);
}
Uint8 tmc_scr1_read2(void) {
	Log_Printf(LOG_WARN,"[TMC] SCR1 read at $0x2200002 PC=$%08x\n",m68k_getpc());
	return (tmc.scr1>>8);
}
Uint8 tmc_scr1_read3(void) {
	Log_Printf(LOG_WARN,"[TMC] SCR1 read at $0x2200003 PC=$%08x\n",m68k_getpc());
	return tmc.scr1;
}



/* Read register functions */
static Uint8 (*tmc_read_reg[36])(void) = {
	tmc_scr1_read0, tmc_scr1_read1, tmc_scr1_read2, tmc_scr1_read3,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read
};

static Uint8 (*tmc_read_vid_reg[16])(void) = {
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read,
	tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read, tmc_unimpl_read
};

/* Write register functions */
static void (*tmc_write_reg[36])(Uint8) = {
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write
};

static void (*tmc_write_vid_reg[16])(Uint8) = {
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write,
	tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write, tmc_unimpl_write
};


Uint32 tmc_lget(uaecptr addr) {
	Uint32 val = 0;
	
    if (addr%4) {
        Log_Printf(LOG_WARN, "[TMC] Unaligned access.");
        abort();
    }
	
	if (addr==0x02210000) {
		Log_Printf(LOG_WARN, "[TMC] Nitro register lget from $%08X",addr);
		return 0;
	}

	if ((addr&0xFFFFF00)==TMC_ADB_ADDR_MASK) {
		Log_Printf(LOG_WARN, "[ADB] lget at $%08X",addr);
		if ((addr&0xFF) == 0) {
			return 0x7FFFFFFF;
		}
		return 0;
	}
	
	Log_Printf(LOG_WARN, "[TMC] lget from %08X",addr);
	
	addr &= TMC_REGS_MASK;

	if (addr<36) {
		val = tmc_read_reg[addr]()<<24;
		val |= tmc_read_reg[addr+1]()<<16;
		val |= tmc_read_reg[addr+2]()<<8;
		val |= tmc_read_reg[addr+3]();
	} else if (addr>=128 && addr<144) {
		val = tmc_read_vid_reg[addr&0xF]()<<24;
		val |= tmc_read_vid_reg[(addr+1)&0xF]()<<16;
		val |= tmc_read_vid_reg[(addr+1)&0xF]()<<8;
		val |= tmc_read_vid_reg[(addr+1)&0xF]();
	}

	return val;
}

Uint32 tmc_wget(uaecptr addr) {
    Uint32 val = 0;
    
	if (addr%2) {
		Log_Printf(LOG_WARN, "[TMC] Unaligned access.");
		abort();
	}
	
	if ((addr&0xFFFFF00)==TMC_ADB_ADDR_MASK) {
		Log_Printf(LOG_WARN, "[ADB] wget at $%08X",addr);
		return 0xFFFF;
	}
	
	Log_Printf(LOG_WARN, "[TMC] wget from %08X",addr);

	addr &= TMC_REGS_MASK;
	
	if (addr<36) {
		val = tmc_read_reg[addr]()<<8;
		val |= tmc_read_reg[addr+1]();
	} else if (addr>=128 && addr<144) {
		val = tmc_read_vid_reg[addr&0xF]()<<8;
		val |= tmc_read_vid_reg[(addr+1)&0xF]();
	}
	
	return val;
}

Uint32 tmc_bget(uaecptr addr) {
	if ((addr&0xFFFFF00)==TMC_ADB_ADDR_MASK) {
		Log_Printf(LOG_WARN, "[ADB] bget at $%08X",addr);
		return 0xFF;
	}
	
	Log_Printf(LOG_WARN, "[TMC] bget from %08X",addr);

	addr &= TMC_REGS_MASK;

	if (addr<36) {
		return tmc_read_reg[addr]();
	} else if (addr>=128 && addr<144) {
		return tmc_read_vid_reg[addr&0xF]();
	}

    return 0;
}

void tmc_lput(uaecptr addr, Uint32 l) {
	if (addr%4) {
		Log_Printf(LOG_WARN, "[TMC] Unaligned access.");
		abort();
	}
	
	if (addr==0x02210000) {
		Log_Printf(LOG_WARN, "[TMC] Nitro register lput %08X at $%08X",l,addr);
		return;
	}
	
	if ((addr&0xFFFFF00)==TMC_ADB_ADDR_MASK) {
		Log_Printf(LOG_WARN, "[ADB] lput %08X at $%08X",l,addr);
		return;
	}
	
	Log_Printf(LOG_WARN, "[TMC] lput %08X to %08X",l,addr);

	addr &= TMC_REGS_MASK;
	
	if (addr<36) {
		tmc_write_reg[addr](l>>24);
		tmc_write_reg[addr+1](l>>16);
		tmc_write_reg[addr+2](l>>8);
		tmc_write_reg[addr+3](l);
	} else if (addr>=128 && addr<144) {
		tmc_write_vid_reg[addr&0xF](l>>24);
		tmc_write_vid_reg[(addr+1)&0xF](l>>16);
		tmc_write_vid_reg[(addr+2)&0xF](l>>8);
		tmc_write_vid_reg[(addr+3)&0xF](l);
	}
}

void tmc_wput(uaecptr addr, Uint32 w) {
	if (addr%2) {
		Log_Printf(LOG_WARN, "[TMC] Unaligned access.");
		abort();
	}
	
	if ((addr&0xFFFFF00)==TMC_ADB_ADDR_MASK) {
		Log_Printf(LOG_WARN, "[ADB] wput %04X at $%08X",w,addr);
		return;
	}
	
	Log_Printf(LOG_WARN, "[TMC] wput %04X to %08X",w,addr);

	addr &= TMC_REGS_MASK;
	
	if (addr<36) {
		tmc_write_reg[addr](w>>8);
		tmc_write_reg[addr+1](w);
	} else if (addr>=128 && addr<144) {
		tmc_write_vid_reg[addr&0xF](w>>8);
		tmc_write_vid_reg[(addr+1)&0xF](w);
	}
}

void tmc_bput(uaecptr addr, Uint32 b) {
	if ((addr&0xFFFFF00)==TMC_ADB_ADDR_MASK) {
		Log_Printf(LOG_WARN, "[ADB] bput %02X at $%08X",b,addr);
		return;
	}
	
	Log_Printf(LOG_WARN, "[TMC] bput %02X to %08X",b,addr);

	addr &= TMC_REGS_MASK;
	
	if (addr<36) {
		tmc_write_reg[addr](b);
	} else if (addr>=128 && addr<144) {
		tmc_write_vid_reg[addr&0xF](b);
	}
}


/* TMC Reset */

void TMC_Reset(void) {
	TurboSCR1_Reset();
}