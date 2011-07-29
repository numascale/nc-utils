#define HASH() HASH_I
#define HASH_I #
#define BACKSLASH_I \\

#define BACKSLASH() BACKSLASH_I

#define NC_OVERRIDE_PREAM
#define H2S_CSR(a, b, c) HASH()define H2S_CSR_G##a##_##b (c+(a<<12))
#define LC3_CSR(a, b) HASH()define LC3_CSR_##a b
#include "numachip-overrides.def"
#undef NC_OVERRIDE_PREAM
#undef H2S_CSR
#undef LC3_CSR

HASH()define NUMACHIP_STR_DECLARE(cfg, csr, lc3) BACKSLASH()
const char *cfg[2][256]; BACKSLASH()
const char *csr[6][1024]; BACKSLASH()
const char *lc3[256]

#define NC_OVERRIDE_PREAM \
    HASH()define NUMACHIP_STR_INIT_OVERRIDE(h2s_csr, lc3_csr) BACKSLASH()
#define H2S_CSR(a, b, c) h2s_csr[a][c/4] = #b; BACKSLASH()
#define LC3_CSR(a, b) lc3_csr[b/4] = #a; BACKSLASH()
#include "numachip-overrides.def"
h2s_csr;
#undef NC_OVERRIDE_PREAM
#undef H2S_CSR
#undef LC3_CSR

HASH()include "numachip-autodefs.h"

HASH()define NUMACHIP_STR_INIT(cfg, csr, lc3) \
	NUMACHIP_STR_INIT_OVERRIDE(csr, lc3) NUMACHIP_STR_INIT_AUTO(cfg, csr)

