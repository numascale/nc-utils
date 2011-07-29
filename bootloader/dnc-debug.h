#ifndef __DNC_DEBUG
#define __DNC_DEBUG 1

#include "../ht-trace/ht-trace.h"

void setup_ht_trace(u16 sciid, u8 cpu, u64 start, u64 end)
{
    printf("Starting trace @ %03x:HT#%d (%llx - %llx)(%x)\n",
	   sciid, cpu, start, end, (u32)((start >> 24) | (end >> 8)));

    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0x120, 0x0);
    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xc4, 0);
    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xc8, TRACE_STOP_NOW);
    
    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xb8, (start >> 24) | (end >> 8));
    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xbc, start >> 6);

    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xc0, 0);
    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xc0, TRACE_CTL_TRACE | TRACE_CTL_MP);

    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xc8, TRACE_STOP_BUF_FULL);

    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xcc,
		   TRACE_CAP_TSC_END |
		   TRACE_CAP_DAT_SRCDST |
		   TRACE_CAP_SRC_ALL |
		   TRACE_CAP_DST_ALL);

    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xc4, TRACE_START_NOW);
}

void start_ht_trace(u16 sciid, u8 cpu)
{
    dnc_write_conf(sciid, 0, 24+cpu, NB_FUNC_DRAM, 0xc4, TRACE_START_NOW);
}

void stop_ht_trace(u16 sciid, u8 cpu)
{
    u32 val;
    
    dnc_write_conf(sciid, 0, 24+cpu, 2, 0xc8, TRACE_STOP_NOW);
    val = dnc_read_conf(sciid, 0, 24+cpu, 2, 0xbc);
    printf("Stopped trace @ %03x:HT#%d (%llx)\n", sciid, cpu,  (u64)val<<6ULL);
    dnc_write_conf(sciid, 0, 24+cpu, 2, 0xc0, 0);
}

#if defined(ENABLE_HT_TRACING)

void start_remote_trace(int num)
{
    u64 start;
    u64 end;
    u16 sciid = num > 0 ? nc_node[num].sci_id : 0xfff0;
    int i;

    for (i = 0; i < nc_node[num].nc_ht_id; i++) {
	printf("SCI %03x HT#%d: %lx, %lx\n", 
	       sciid, i, nc_node[num].ht[i].base, nc_node[num].ht[i].size);
	if (!nc_node[num].ht[i].size)
	    continue;

	start = ((u64)nc_node[num].ht[i].base << DRAM_MAP_SHIFT) +
	    ((u64)nc_node[num].ht[i].size << DRAM_MAP_SHIFT) - TRACE_BUF_SIZE;
	end   = ((u64)nc_node[num].ht[i].base << DRAM_MAP_SHIFT) +
	    ((u64)nc_node[num].ht[i].size << DRAM_MAP_SHIFT) - (1<<DRAM_MAP_SHIFT);

	setup_ht_trace(sciid, i, start, end);
    }

    for (i = 0; i < nc_node[num].nc_ht_id; i++) {
	start_ht_trace(sciid, i);
    }
}

void start_local_trace(void)
{
    start_remote_trace(0);
}

#endif

#endif
