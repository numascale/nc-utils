


#define LC3_CSR_STATE_CLEAR 0x0000
#define LC3_CSR_NODE_IDS 0x0008
#define LC3_CSR_ERROR_COUNT 0x0180
#define LC3_CSR_SYNC_INTERVAL 0x0200
#define LC3_CSR_SAVE_ID 0x0208
#define LC3_CSR_ROUT_LCTBL00 0x0280
#define LC3_CSR_ROUT_LCTBL15 0x02BC
#define LC3_CSR_ROUT_BXTBLL00 0x02C0
#define LC3_CSR_ROUT_BXTBLL15 0x02FC
#define LC3_CSR_ROUT_BLTBL00 0x0300
#define LC3_CSR_ROUT_BLTBL15 0x033C
#define LC3_CSR_ROUT_BXTBLH00 0x0340
#define LC3_CSR_ROUT_BXTBLH15 0x037C
#define LC3_CSR_ROUT_CTRL 0x0380
#define LC3_CSR_ROUT_MASK 0x0384
#define LC3_CSR_UID1 0x0394
#define LC3_CSR_UID2 0x0398
#define LC3_CSR_ELOG0 0x03A0
#define LC3_CSR_ELOG1 0x03D8
#define LC3_CSR_INIT_STATE 0x03A8
#define LC3_CSR_CONFIG1 0x0388
#define LC3_CSR_CONFIG2 0x038C
#define LC3_CSR_CONFIG3 0x0390
#define LC3_CSR_CONFIG4 0x03D4
#define LC3_CSR_PCSEL 0x03BC
#define LC3_CSR_PCCNT 0x03C0
#define LC3_CSR_SW_INFO 0x03C4
#define LC3_CSR_SW_INFO2 0x03E0
#define LC3_CSR_SW_INFO3 0x03E4
#define H2S_CSR_G0_STATE_CLEAR (0x000 +(0<<12))
#define H2S_CSR_G0_STATE_SET (0x004 +(0<<12))
#define H2S_CSR_G0_NODE_IDS (0x008 +(0<<12))
#define H2S_CSR_G0_RESET_START (0x00C +(0<<12))
#define H2S_CSR_G0_INDIRECT_ADDRESS (0x010 +(0<<12))
#define H2S_CSR_G0_INDIRECT_DATA (0x014 +(0<<12))
#define H2S_CSR_G0_INTERRUPT_TARGET (0x080 +(0<<12))
#define H2S_CSR_G0_INTERRUPT_MASK (0x084 +(0<<12))
#define H2S_CSR_G0_CHIP_ID (0x3B0 +(0<<12))
#define H2S_CSR_G0_WATCH_SEL (0x3B8 +(0<<12))
#define H2S_CSR_G0_ROUT_TABLE_CHUNK (0x3C0 +(0<<12))
#define H2S_CSR_G0_ROUT_LCTBL00 (0x480 +(0<<12))
#define H2S_CSR_G0_ROUT_LCTBL15 (0x4BC +(0<<12))
#define H2S_CSR_G0_ROUT_BXTBLL00 (0x4C0 +(0<<12))
#define H2S_CSR_G0_ROUT_BXTBLL15 (0x4FC +(0<<12))
#define H2S_CSR_G0_ROUT_BLTBL00 (0x500 +(0<<12))
#define H2S_CSR_G0_ROUT_BLTBL15 (0x53C +(0<<12))
#define H2S_CSR_G0_ROUT_BXTBLH00 (0x540 +(0<<12))
#define H2S_CSR_G0_ROUT_BXTBLH15 (0x57C +(0<<12))
#define H2S_CSR_G0_MIB_IBX (0xC00 +(0<<12))
#define H2S_CSR_G0_MIB_IBC (0xC04 +(0<<12))
#define H2S_CSR_G0_MIB_IBD_HI (0xC08 +(0<<12))
#define H2S_CSR_G0_MIB_IBD_LO (0xC0C +(0<<12))
#define H2S_CSR_G0_SEQ_INDEX (0xC20 +(0<<12))
#define H2S_CSR_G0_WCS_ENTRY (0xC24 +(0<<12))
#define H2S_CSR_G0_JUMP_ENTRY (0xC28 +(0<<12))
#define H2S_CSR_G0_STACK_ENTRY (0xC2C +(0<<12))
#define H2S_CSR_G0_CNTXT_ENTRY (0xC30 +(0<<12))
#define H2S_CSR_G0_SEQ_INFO (0xC34 +(0<<12))
#define H2S_CSR_G0_MIU_NGCM0_LIMIT (0xC40 +(0<<12))
#define H2S_CSR_G0_MIU_NGCM1_LIMIT (0xC44 +(0<<12))
#define H2S_CSR_G0_MIU_CACHE_SIZE (0xC48 +(0<<12))
#define H2S_CSR_G0_RAW_CONTROL (0xC50 +(0<<12))
#define H2S_CSR_G0_RAW_INDEX (0xC54 +(0<<12))
#define H2S_CSR_G0_RAW_ENTRY_LO (0xC58 +(0<<12))
#define H2S_CSR_G0_RAW_ENTRY_HI (0xC5C +(0<<12))
#define H2S_CSR_G0_CHIP_RESET (0xD00 +(0<<12))
#define H2S_CSR_G0_RESET_DELAY (0xD04 +(0<<12))
#define H2S_CSR_G0_REG_PROTECT (0xD08 +(0<<12))
#define H2S_CSR_G0_ERROR_FSTAT (0xD10 +(0<<12))
#define H2S_CSR_G0_ERROR_FCLR (0xD14 +(0<<12))
#define H2S_CSR_G0_ERROR_FMASK (0xD18 +(0<<12))
#define H2S_CSR_G0_ERROR_NFSTAT (0xD20 +(0<<12))
#define H2S_CSR_G0_ERROR_NFCLR (0xD24 +(0<<12))
#define H2S_CSR_G0_ERROR_NFMASK (0xD28 +(0<<12))
#define H2S_CSR_G0_PERF_COUNT1 (0xD30 +(0<<12))
#define H2S_CSR_G0_PERF_LIMIT1 (0xD34 +(0<<12))
#define H2S_CSR_G0_PERF_COMPARE1 (0xD38 +(0<<12))
#define H2S_CSR_G0_PERF_MASK1 (0xD3C +(0<<12))
#define H2S_CSR_G0_PERF_COUNT2 (0xD40 +(0<<12))
#define H2S_CSR_G0_PERF_LIMIT2 (0xD44 +(0<<12))
#define H2S_CSR_G0_PERF_COMPARE2 (0xD48 +(0<<12))
#define H2S_CSR_G0_PERF_MASK2 (0xD4C +(0<<12))
#define H2S_CSR_G0_TRANS_COUNT (0xD50 +(0<<12))
#define H2S_CSR_G0_TRANS_COMPARE (0xD54 +(0<<12))
#define H2S_CSR_G0_TRANS_MASK (0xD58 +(0<<12))
#define H2S_CSR_G0_PERF_MCODE0 (0xD60 +(0<<12))
#define H2S_CSR_G0_PERF_MCODE1 (0xD64 +(0<<12))
#define H2S_CSR_G0_PERF_MCODE2 (0xD68 +(0<<12))
#define H2S_CSR_G0_PERF_MCODE3 (0xD6C +(0<<12))
#define H2S_CSR_G0_AP_INDEX (0xD80 +(0<<12))
#define H2S_CSR_G0_AP_ENTRY (0xD84 +(0<<12))
#define H2S_CSR_G0_BIU_CONTROL (0xD88 +(0<<12))
#define H2S_CSR_G0_AP_START (0xD8C +(0<<12))
#define H2S_CSR_G0_AP_END (0xD90 +(0<<12))
#define H2S_CSR_G0_ATT_INDEX (0xDC0 +(0<<12))
#define H2S_CSR_G0_ATT_ENTRY (0xDC4 +(0<<12))
#define H2S_CSR_G1_PIC_INDIRECT_READ (0x400 +(1<<12))
#define H2S_CSR_G1_PIC_RESET_CTRL (0x404 +(1<<12))
#define H2S_CSR_G4_MCTAG_DENALI_CTL_00 (0x000 +(4<<12))
#define H2S_CSR_G4_CDATA_DENALI_CTL_00 (0x800 +(4<<12))
#define NUMACHIP_STR_DECLARE(cfg, csr, lc3) \
const char *cfg[2][256]; \
const char *csr[6][1024]; \
const char *lc3[256]
#define NUMACHIP_STR_INIT_OVERRIDE(h2s_csr, lc3_csr) \
lc3_csr[0x0000/4] = "STATE_CLEAR"; \
lc3_csr[0x0008/4] = "NODE_IDS"; \
lc3_csr[0x0180/4] = "ERROR_COUNT"; \
lc3_csr[0x0200/4] = "SYNC_INTERVAL"; \
lc3_csr[0x0208/4] = "SAVE_ID"; \
lc3_csr[0x0280/4] = "ROUT_LCTBL00"; \
lc3_csr[0x02BC/4] = "ROUT_LCTBL15"; \
lc3_csr[0x02C0/4] = "ROUT_BXTBLL00"; \
lc3_csr[0x02FC/4] = "ROUT_BXTBLL15"; \
lc3_csr[0x0300/4] = "ROUT_BLTBL00"; \
lc3_csr[0x033C/4] = "ROUT_BLTBL15"; \
lc3_csr[0x0340/4] = "ROUT_BXTBLH00"; \
lc3_csr[0x037C/4] = "ROUT_BXTBLH15"; \
lc3_csr[0x0380/4] = "ROUT_CTRL"; \
lc3_csr[0x0384/4] = "ROUT_MASK"; \
lc3_csr[0x0394/4] = "UID1"; \
lc3_csr[0x0398/4] = "UID2"; \
lc3_csr[0x03A0/4] = "ELOG0"; \
lc3_csr[0x03D8/4] = "ELOG1"; \
lc3_csr[0x03A8/4] = "INIT_STATE"; \
lc3_csr[0x0388/4] = "CONFIG1"; \
lc3_csr[0x038C/4] = "CONFIG2"; \
lc3_csr[0x0390/4] = "CONFIG3"; \
lc3_csr[0x03D4/4] = "CONFIG4"; \
lc3_csr[0x03C4/4] = "SW_INFO"; \
lc3_csr[0x03E0/4] = "SW_INFO2"; \
lc3_csr[0x03E4/4] = "SW_INFO3"; \
h2s_csr[0][0x000/4] = "STATE_CLEAR"; \
h2s_csr[0][0x004/4] = "STATE_SET"; \
h2s_csr[0][0x008/4] = "NODE_IDS"; \
h2s_csr[0][0x00C/4] = "RESET_START"; \
h2s_csr[0][0x010/4] = "INDIRECT_ADDRESS"; \
h2s_csr[0][0x014/4] = "INDIRECT_DATA"; \
h2s_csr[0][0x080/4] = "INTERRUPT_TARGET"; \
h2s_csr[0][0x084/4] = "INTERRUPT_MASK"; \
h2s_csr[0][0x3B0/4] = "CHIP_ID"; \
h2s_csr[0][0x3B8/4] = "WATCH_SEL"; \
h2s_csr[0][0x3C0/4] = "ROUT_TABLE_CHUNK"; \
h2s_csr[0][0x480/4] = "ROUT_LCTBL00"; \
h2s_csr[0][0x4BC/4] = "ROUT_LCTBL15"; \
h2s_csr[0][0x4C0/4] = "ROUT_BXTBLL00"; \
h2s_csr[0][0x4FC/4] = "ROUT_BXTBLL15"; \
h2s_csr[0][0x500/4] = "ROUT_BLTBL00"; \
h2s_csr[0][0x53C/4] = "ROUT_BLTBL15"; \
h2s_csr[0][0x540/4] = "ROUT_BXTBLH00"; \
h2s_csr[0][0x57C/4] = "ROUT_BXTBLH15"; \
h2s_csr[0][0xC00/4] = "MIB_IBX"; \
h2s_csr[0][0xC04/4] = "MIB_IBC"; \
h2s_csr[0][0xC08/4] = "MIB_IBD_HI"; \
h2s_csr[0][0xC0C/4] = "MIB_IBD_LO"; \
h2s_csr[0][0xC20/4] = "SEQ_INDEX"; \
h2s_csr[0][0xC24/4] = "WCS_ENTRY"; \
h2s_csr[0][0xC28/4] = "JUMP_ENTRY"; \
h2s_csr[0][0xC2C/4] = "STACK_ENTRY"; \
h2s_csr[0][0xC30/4] = "CNTXT_ENTRY"; \
h2s_csr[0][0xC34/4] = "SEQ_INFO"; \
h2s_csr[0][0xC40/4] = "MIU_NGCM0_LIMIT"; \
h2s_csr[0][0xC44/4] = "MIU_NGCM1_LIMIT"; \
h2s_csr[0][0xC48/4] = "MIU_CACHE_SIZE"; \
h2s_csr[0][0xC50/4] = "RAW_CONTROL"; \
h2s_csr[0][0xC54/4] = "RAW_INDEX"; \
h2s_csr[0][0xC58/4] = "RAW_ENTRY_LO"; \
h2s_csr[0][0xC5C/4] = "RAW_ENTRY_HI"; \
h2s_csr[0][0xD00/4] = "CHIP_RESET"; \
h2s_csr[0][0xD04/4] = "RESET_DELAY"; \
h2s_csr[0][0xD08/4] = "REG_PROTECT"; \
h2s_csr[0][0xD10/4] = "ERROR_FSTAT"; \
h2s_csr[0][0xD14/4] = "ERROR_FCLR"; \
h2s_csr[0][0xD18/4] = "ERROR_FMASK"; \
h2s_csr[0][0xD20/4] = "ERROR_NFSTAT"; \
h2s_csr[0][0xD24/4] = "ERROR_NFCLR"; \
h2s_csr[0][0xD28/4] = "ERROR_NFMASK"; \
h2s_csr[0][0xD30/4] = "PERF_COUNT1"; \
h2s_csr[0][0xD34/4] = "PERF_LIMIT1"; \
h2s_csr[0][0xD38/4] = "PERF_COMPARE1"; \
h2s_csr[0][0xD3C/4] = "PERF_MASK1"; \
h2s_csr[0][0xD40/4] = "PERF_COUNT2"; \
h2s_csr[0][0xD44/4] = "PERF_LIMIT2"; \
h2s_csr[0][0xD48/4] = "PERF_COMPARE2"; \
h2s_csr[0][0xD4C/4] = "PERF_MASK2"; \
h2s_csr[0][0xD50/4] = "TRANS_COUNT"; \
h2s_csr[0][0xD54/4] = "TRANS_COMPARE"; \
h2s_csr[0][0xD58/4] = "TRANS_MASK"; \
h2s_csr[0][0xD60/4] = "PERF_MCODE0"; \
h2s_csr[0][0xD64/4] = "PERF_MCODE1"; \
h2s_csr[0][0xD68/4] = "PERF_MCODE2"; \
h2s_csr[0][0xD6C/4] = "PERF_MCODE3"; \
h2s_csr[0][0xD80/4] = "AP_INDEX"; \
h2s_csr[0][0xD84/4] = "AP_ENTRY"; \
h2s_csr[0][0xD88/4] = "BIU_CONTROL"; \
h2s_csr[0][0xD8C/4] = "AP_START"; \
h2s_csr[0][0xD90/4] = "AP_END"; \
h2s_csr[0][0xDC0/4] = "ATT_INDEX"; \
h2s_csr[0][0xDC4/4] = "ATT_ENTRY"; \
h2s_csr[1][0x400/4] = "PIC_INDIRECT_READ"; \
h2s_csr[1][0x404/4] = "PIC_RESET_CTRL"; \
h2s_csr[4][0x000/4] = "MCTAG_DENALI_CTL_00"; \
h2s_csr[4][0x800/4] = "CDATA_DENALI_CTL_00"; \
h2s_csr;
#include "numachip-autodefs.h"
#define NUMACHIP_STR_INIT(cfg, csr, lc3) NUMACHIP_STR_INIT_OVERRIDE(csr, lc3) NUMACHIP_STR_INIT_AUTO(cfg, csr)
