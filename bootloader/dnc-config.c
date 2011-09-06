// $Id:$
// This source code including any derived information including but
// not limited by net-lists, fpga bit-streams, and object files is the
// confidential and proprietary property of
// 
// Numascale AS
// Enebakkveien 302A
// NO-1188 Oslo
// Norway
// 
// Any unauthorized use, reproduction or transfer of the information
// provided herein is strictly prohibited.
// 
// Copyright © 2008-2011
// Numascale AS Oslo, Norway. 
// All Rights Reserved.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dnc-config.h"
#include "json-1.3/src/json.h"

struct fabric_info cfg_fabric;
struct node_info *cfg_nodelist;
struct part_info *cfg_partlist;
int cfg_nodes, cfg_partitions;

static int parse_json_bool(json_t *obj, const char *label, u32 *val, int opt)
{
    json_t *item;
    *val = -1;
    item = json_find_first_label(obj, label);
    if (!(item && item->child)) {
	if (!opt)
	    printf("** parse error: label <%s> not found!\n", label);
        return 0;
    }
    if (item->child->type == JSON_TRUE) {
        *val = 1;
    }
    else if (item->child->type == JSON_FALSE) {
        *val = 0;
    }
    else {
        printf("** parse error: label <%s> has bad type (%d)!\n",
               label, item->child->type);
        return 0;
    }

    return 1;
}

static int parse_json_num(json_t *obj, const char *label, u32 *val, int opt)
{
    json_t *item;
    char *end;
    *val = -1;
    item = json_find_first_label(obj, label);
    if (!(item && item->child)) {
	if (!opt)
	    printf("** parse error: label <%s> not found!\n", label);
        return 0;
    }
    if (item->child->type == JSON_NUMBER) {
        *val = strtol(item->child->text, &end, 10);
    }
    else if (item->child->type == JSON_STRING) {
        *val = strtol(item->child->text, &end, 0);
    }
    else {
        printf("** parse error: label <%s> has bad type (%d)!\n",
               label, item->child->type);
        return 0;
    }

    if (end[0] != '\0') {
        printf("** parse error: label <%s> value bad format!\n", label);
        *val = -1;
        return 0;
    }
    return 1;
}

static int parse_json_str(json_t *obj, const char *label, char *val, int len, int opt)
{
    json_t *item;
    val[0] = '\0';
    item = json_find_first_label(obj, label);
    if (!(item && item->child)) {
	if (!opt)
	    printf("** parse error: label <%s> not found!\n", label);
        return 0;
    }
    if (item->child->type == JSON_STRING) {
        strncpy(val, item->child->text, len);
        val[len-1] = '\0';
    }
    else {
        printf("** parse error: label <%s> has bad type (%d)!\n",
               label, item->child->type);
        return 0;
    }
    return 1;
}

static int parse_json(json_t *root)
{
    json_t *fab, *list, *obj;
    int i;
    u32 val;

    fab = json_find_first_label(root, "fabric");
    if (!fab) {
        printf("** parse error: object <fabric> not found!\n");
        return 0;
    }
    if (!(parse_json_num(fab->child, "x-size", &cfg_fabric.x_size, 0) &&
          parse_json_num(fab->child, "y-size", &cfg_fabric.y_size, 0) &&
          parse_json_num(fab->child, "z-size", &cfg_fabric.z_size, 0)))
        return 0;

    if (!parse_json_bool(fab->child, "strict", &cfg_fabric.strict, 0))
	cfg_fabric.strict = 0;

    list = json_find_first_label(fab->child, "nodes");
    if (!(list && list->child && list->child->type == JSON_ARRAY)) {
        printf("** parse error: array <nodes> not found!\n");
        return 0;
    }

    for (cfg_nodes = 0, obj = list->child->child; obj; obj = obj->next)
        cfg_nodes++;
    cfg_nodelist = malloc(cfg_nodes * sizeof(*cfg_nodelist));
    for (i = 0, obj = list->child->child; obj; obj = obj->next, i++) {
        if (!(parse_json_num(obj, "uuid", &cfg_nodelist[i].uuid, 0) &&
              parse_json_num(obj, "sciid", &cfg_nodelist[i].sciid, 0) &&
              parse_json_num(obj, "partition", &cfg_nodelist[i].partition, 0) &&
              parse_json_num(obj, "osc", &cfg_nodelist[i].osc, 0) &&
              parse_json_str(obj, "desc", cfg_nodelist[i].desc, 32, 0)))
        {
            free(cfg_nodelist);
            return 0;
        }
	if (parse_json_num(obj, "sync-only", &val, 1))
	    cfg_nodelist[i].sync_only = val;
	else 
	    cfg_nodelist[i].sync_only = 0;
    }

    list = json_find_first_label(fab->child, "partitions");
    if (!(list && list->child && list->child->type == JSON_ARRAY)) {
        printf("** parse error: array <paritions> not found!\n");
        free(cfg_nodelist);
        return 0;
    }

    for (cfg_partitions = 0, obj = list->child->child; obj; obj = obj->next)
        cfg_partitions++;
    cfg_partlist = malloc(cfg_partitions * sizeof(*cfg_partlist));
    for (i = 0, obj = list->child->child; obj; obj = obj->next, i++) {
	if (!(parse_json_num(obj, "master", &cfg_partlist[i].master, 0) &&
	      parse_json_num(obj, "builder", &cfg_partlist[i].builder, 0)))
        {
            free(cfg_nodelist);
            free(cfg_partlist);
            return 0;
        }
    }
    return 1;
}

int parse_config_file(char *data)
{
    json_t *root = NULL;
    enum json_error err;
    int ret = 0;

#ifdef DEBUG_CONFIG
    printf("+++++++ START JSON +++++++++\n");
    printf("%s", data);
    printf("+++++++ END   JSON +++++++++\n");
#endif

    err = json_parse_document(&root, data);
#ifdef DEBUG_CONFIG
    if (!root) printf("No json root????\n");
#endif
    if (root) {
        if (parse_json(root)) {
#ifdef DEBUG_CONFIG            
            int i;
            printf("Fabric dimensions: x: %d, y: %x, z: %d\n",
                   cfg_fabric.x_size, cfg_fabric.y_size, cfg_fabric.z_size);

            for (i = 0; i < cfg_nodes; i++) {
                printf("Node %d: <%s> uuid: %08x, sciid: 0x%03x, partition: %d, osc: %d, sync-only: %d\n",
                       i,
                       cfg_nodelist[i].desc,
                       cfg_nodelist[i].uuid,
                       cfg_nodelist[i].sciid,
                       cfg_nodelist[i].partition,
                       cfg_nodelist[i].osc,
                       cfg_nodelist[i].sync_only);
            }

            for (i = 0; i < cfg_partitions; i++) {
                printf("Partition %d: master: 0x%03x, builder: 0x%03x\n",
                       i, cfg_partlist[i].master, cfg_partlist[i].builder);
            }
#endif
            ret = 1;
        }
#ifdef DEBUG_CONFIG
        else printf("Error parsing json root????\n");
#endif

        json_free_value(&root);
    }

    return ret;
}

struct node_info* get_node_config(u32 uuid)
{
    int i;
    for (i = 0; i < cfg_nodes; i++) {
        if (cfg_nodelist[i].uuid == uuid)
            return &cfg_nodelist[i];
    }
    return NULL;
}

struct part_info* get_partition_config(int idx)
{
    if (idx < cfg_partitions)
        return &cfg_partlist[idx];
    else
        return NULL;
}
