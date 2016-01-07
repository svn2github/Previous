/***************************************************************************

    i860dbg.cpp

    Debugger for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)
    Released for general non-commercial use under the MAME license
    with the additional requirement that you are free to use and
    redistribute this code in modified or unmodified form, provided
    you list me in the credits.
    Visit http://mamedev.org for licensing and usage restrictions.

    Changes for previous/NeXTdimension by Simon Schubiger (SC)
 
***************************************************************************/

extern int i860_disassembler(UINT32 pc, UINT32 insn, char* buffer);

/* A simple internal debugger.  */
void i860_cpu_device::debugger() {
    debugger("");
}

void i860_cpu_device::debugger(const char* format, ...) {
    char buf[256];
    UINT32 curr_disasm = m_pc;
    UINT32 curr_dumpdb = 0;
    int c = 0;
    
    if (m_single_stepping > 1 && m_single_stepping != m_pc)
        return;
    
    if(format) {
        va_list ap;
        va_start (ap, format);
        fprintf(stderr, "\n[i860]");
        if(format[0]) {
            fprintf(stderr, " (");
            vfprintf (stderr, format, ap);
        }
        va_end (ap);
        if(format[0])
            fprintf(stderr, ")");
        fprintf(stderr, " debugger started (? for help).\n");
        }
    
    disasm (m_pc, 1);
    if (has_delay_slot (2))
        disasm (m_pc + 4, 1);
    
    buf[0] = 0;
    fflush (stdin);
    
    m_single_stepping = 0;
    
    while(!m_single_stepping) {
        fprintf (stderr, ">");
        for(;;) {
            char it = 0;
            if (read(STDIN_FILENO, &it, 1) == 1) {
                if (it == '\n') {
                    buf[c] = 0;
                    c = 0;
                    break;
                }
                buf[c++] = it;
            }
        }
        if(buf[0] == 0) {
            buf[0] = m_lastcmd;
            buf[1] = 0;
        } else {
            m_lastcmd = buf[0];
        }
        switch(buf[0]) {
            case 'g':
                if (buf[1] == '0')
                    sscanf (buf + 1, "%x", &m_single_stepping);
                else {
                    m_single_stepping = 2;
                    break;
                }
                buf[1] = 0;
                fprintf (stderr, "go until pc = 0x%08x.\n", m_single_stepping);
                break;
            case 'h':
                i860_halt(true);
                m_single_stepping = 2;
                break;
            case 'c':
                i860_halt(false);
                m_single_stepping = 2;
                break;
            case 'r':
                dump_state();
                break;
            case 'd':
                if (buf[1] == '0')
                    sscanf (buf + 1, "%x", &curr_disasm);
                curr_disasm = disasm (curr_disasm, 10);
                buf[1] = 0;
                break;
            case 'p':
                if (buf[1] >= '0' && buf[1] <= '4')
                    dump_pipe (buf[1] - 0x30);
                buf[1] = 0;
                break;
            case 's':
                m_single_stepping = 1;
                break;
            case 'w':
                nd_dbg_cmd(buf);
                break;
            case 'm':
                if (buf[1] == '0')
                    sscanf (buf + 1, "%x", &curr_dumpdb);
                dbg_db (curr_dumpdb, 32);
                curr_dumpdb += 32;
                break;
            case 'x':
                if(buf[1] == '0') {
                    UINT32 v;
                    sscanf (buf + 1, "%x", &v);
                    if (GET_DIRBASE_ATE ())
                        fprintf (stderr, "vma 0x%08x ==> phys 0x%08x\n", v, get_address_translation (v, 1, 0));
                    else
                        fprintf (stderr, "not in virtual address mode.\n");
                    break;
                }
            case '?':
                fprintf (stderr,
                         "   m: dump bytes (m[0xaddress])\n"
                         "   r: dump registers\n"
                         "   s: single-step\n"
                         "   h: halt i860\n"
                         "   c: continue i860\n"
                         "   g: go back to emulator (g[0xaddress])\n"
                         "   d: disassemble (u[0xaddress])\n"
                         "   p: dump pipelines (p{0-4} for all, add, mul, load, graphics)\n"
                         "   x: give virt->phys translation (x{0xaddress})\n");
                nd_dbg_cmd(0);
                break;
            default:
                if(!(nd_dbg_cmd(buf)))
                    fprintf (stderr, "Bad command '%s'. Type '?' for help.\n", buf);
                break;
        }
    }
    
    /* Less noise when single-stepping.  */
    if (m_single_stepping != 1) {
        fprintf (stderr, "Debugger done, continuing emulation.\n");
        m_single_stepping = 0;
    }
}

/* Disassemble `len' instructions starting at `addr'.  */
UINT32 i860_cpu_device::disasm (UINT32 addr, int len)
{
	UINT32 insn;
	int j;
	for (j = 0; j < len; j++)
	{
		char buf[256];
		/* Note that we print the incoming (possibly virtual) address as the
		   PC rather than the translated address.  */
		fprintf (stderr, " [i860] %08X: ", addr);
		insn = ifetch_notrap(addr);
        i860_disassembler(addr, insn, buf);
        fprintf (stderr, "%s", buf);
		fprintf (stderr, "\n");
		addr += 4;
		if (m_single_stepping == 1 && has_delay_slot (insn))
			len += 1;
	}
    return addr;
}


/* Dump `len' bytes starting at `addr'.  */
void i860_cpu_device::dbg_db (UINT32 addr, int len)
{
	UINT8 b[16];
	int i;
	/* This will always dump a multiple of 16 bytes, even if 'len' isn't.  */
	while (len > 0)
	{
		/* Note that we print the incoming (possibly virtual) address
		   rather than the translated address.  */
		fprintf (stderr, "0x%08x: ", addr);
		for (i = 0; i < 16; i++)
		{
			UINT32 phys_addr = addr;
			if (GET_DIRBASE_ATE ())
				phys_addr = get_address_translation (addr, 1  /* is_dataref */, 0 /* is_write */);

			b[i] = rd8(phys_addr);
			fprintf (stderr, "%02x ", b[i]);
			addr++;
		}
		fprintf (stderr, "| ");
		for (i = 0; i < 16; i++)
		{
			if (isprint (b[i]))
				fprintf (stderr, "%c", b[i]);
			else
				fprintf (stderr, ".");
		}
		fprintf (stderr, "\n");
		len -= 16;
	}
}

/* Do a pipeline dump.
 type: 0 (all), 1 (add), 2 (mul), 3 (load), 4 (graphics).  */
void i860_cpu_device::dump_pipe (int type)
{
    int i = 0;
    
    fprintf (stderr, "pipeline state:\n");
    /* Dump the adder pipeline, if requested.  */
    if (type == 0 || type == 1)
    {
        fprintf (stderr, "  A: ");
        for (i = 0; i < 3; i++)
        {
            if (m_A[i].stat.arp)
                fprintf (stderr, "[%dd] 0x%016llx ", i + 1,
                         *(UINT64 *)(&m_A[i].val.d));
            else
                fprintf (stderr, "[%ds] 0x%08x ", i + 1,
                         *(UINT32 *)(&m_A[i].val.s));
        }
        fprintf (stderr, "\n");
    }
    
    
    /* Dump the multiplier pipeline, if requested.  */
    if (type == 0 || type == 2)
    {
        fprintf (stderr, "  M: ");
        for (i = 0; i < 3; i++)
        {
            if (m_M[i].stat.mrp)
                fprintf (stderr, "[%dd] 0x%016llx ", i + 1,
                         *(UINT64 *)(&m_M[i].val.d));
            else
                fprintf (stderr, "[%ds] 0x%08x ", i + 1,
                         *(UINT32 *)(&m_M[i].val.s));
        }
        fprintf (stderr, "\n");
    }
    
    /* Dump the load pipeline, if requested.  */
    if (type == 0 || type == 3)
    {
        fprintf (stderr, "  L: ");
        for (i = 0; i < 3; i++)
        {
            if (m_L[i].stat.lrp)
                fprintf (stderr, "[%dd] 0x%016llx ", i + 1,
                         *(UINT64 *)(&m_L[i].val.d));
            else
                fprintf (stderr, "[%ds] 0x%08x ", i + 1,
                         *(UINT32 *)(&m_L[i].val.s));
        }
        fprintf (stderr, "\n");
    }
    
    /* Dump the graphics pipeline, if requested.  */
    if (type == 0 || type == 4)
    {
        fprintf (stderr, "  I: ");
        if (m_G.stat.irp)
            fprintf (stderr, "[1d] 0x%016llx\n",
                     *(UINT64 *)(&m_G.val.d));
        else
            fprintf (stderr, "[1s] 0x%08x\n",
                     *(UINT32 *)(&m_G.val.s));
    }
}


/* Do a register/state dump.  */
void i860_cpu_device::dump_state()
{
    int rn;
    
    /* GR's first, 4 per line.  */
    for (rn = 0; rn < 32; rn++)
    {
        if ((rn % 4) == 0)
            fprintf (stderr, "\n");
        fprintf (stderr, "%%r%-3d: 0x%08x  ", rn, get_iregval (rn));
    }
    fprintf (stderr, "\n");
    
    /* FR's (as 32-bits), 4 per line.  */
    for (rn = 0; rn < 32; rn++)
    {
        float ff = get_fregval_s (rn);
        if ((rn % 4) == 0)
            fprintf (stderr, "\n");
        fprintf (stderr, "%%f%-3d: 0x%08x  ", rn, *(UINT32 *)&ff);
    }
    fprintf (stderr, "\n");
    fprintf(stderr, "    psr: CC=%d LCC=%d SC=%d IM=%d U=%d IT=%d FT=%d IAT=%d DAT=%d IN=%d\n",
            GET_PSR_CC (), GET_PSR_LCC (), GET_PSR_SC (), GET_PSR_IM (), GET_PSR_U (),
            GET_PSR_IT (), GET_PSR_FT (), GET_PSR_IAT (), GET_PSR_DAT (), GET_PSR_IN ());
    fprintf(stderr, "   epsr: INT=%d, OF=%d BE=%d WP=%d\n",       GET_EPSR_INT (), GET_EPSR_OF (), GET_EPSR_BE (), GET_EPSR_WP());
    fprintf(stderr, "dirbase: CS8=%d ATE=%d 0x%08x",              GET_DIRBASE_CS8(), GET_DIRBASE_ATE(), m_cregs[CR_DIRBASE]);
    fprintf(stderr, "    fir: 0x%08x fsr: 0x%08x pc:0x%08x\n", m_cregs[CR_FIR], m_cregs[CR_FSR], m_pc);
}
