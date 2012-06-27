#include "dnc-types.h"

struct fabric_info {
    u32 x_size;
    u32 y_size;
    u32 z_size;
    u32 strict;
};

struct node_info {
    u32 uuid;
    u32 sciid;
    u32 partition;
    u32 osc;
    char desc[32];
    u32 sync_only;
};

struct part_info {
    u32 master;
    u32 builder;
};

int parse_config_file(const char *filename,
		      struct node_info **cfg_nodelist,
		      int *num_nodes);
