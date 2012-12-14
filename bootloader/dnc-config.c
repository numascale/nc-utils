/*
 * Copyright (C) 2008-2012 Numascale AS, support@numascale.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dnc-config.h"
#include "dnc-commonlib.h"
#include "json-1.5/src/json.h"

struct fabric_info cfg_fabric;
struct node_info *cfg_nodelist;
struct part_info *cfg_partlist;
int cfg_nodes, cfg_partitions;
int name_matching = 0;
char *hostname;

static int parse_json_bool(json_t *obj, const char *label, uint32_t *val, int opt)
{
    json_t *item;
    *val = -1;
    item = json_find_first_label(obj, label);
    if (!(item && item->child)) {
	if (!opt)
	    printf("Warning: Label <%s> not found in fabric configuration file\n", label);
        return 0;
    }
    if (item->child->type == JSON_TRUE) {
        *val = 1;
    }
    else if (item->child->type == JSON_FALSE) {
        *val = 0;
    }
    else {
        printf("Warning: Label <%s> has bad type %d in fabric configuration file\n",
               label, item->child->type);
        return 0;
    }

    return 1;
}

static int parse_json_num(json_t *obj, const char *label, uint32_t *val, int opt)
{
    json_t *item;
    char *end;
    *val = -1;
    item = json_find_first_label(obj, label);
    if (!(item && item->child)) {
	if (!opt)
	    printf("Warning: Label <%s> not found in fabric configuration file\n", label);
        return 0;
    }
    if (item->child->type == JSON_NUMBER) {
        *val = strtol(item->child->text, &end, 10);
    }
    else if (item->child->type == JSON_STRING) {
        *val = strtol(item->child->text, &end, 16);
    }
    else {
        printf("Warning: Label <%s> has bad type %d in fabric configuration file\n",
               label, item->child->type);
        return 0;
    }

    if (end[0] != '\0') {
        printf("Warning: Label <%s> value bad format in fabric configuration file\n", label);
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
	    printf("Warning: Label <%s> not found in fabric configuration file\n", label);
        return 0;
    }
    if (item->child->type == JSON_STRING) {
        strncpy(val, item->child->text, len);
        val[len-1] = '\0';
    }
    else {
        printf("Warning: Label <%s> has bad type %d in fabric configuration file\n",
               label, item->child->type);
        return 0;
    }
    return 1;
}

static int parse_json(json_t *root)
{
    json_t *fab, *list, *obj;
    int i;
    uint32_t val;

    fab = json_find_first_label(root, "fabric");
    if (!fab) {
        printf("Error: Label <fabric> not found in fabric configuration file\n");
        goto out1;
    }
    if (!parse_json_num(fab->child, "x-size", &cfg_fabric.x_size, 0)) {
	printf("Error: Label <x-size> not found in fabric configuration file\n");
	goto out1;
    }

    if (!parse_json_num(fab->child, "y-size", &cfg_fabric.y_size, 0)) {
	printf("Error: Label <y-size> not found in fabric configuration file\n");
	goto out1;
    }

    if (!parse_json_num(fab->child, "z-size", &cfg_fabric.z_size, 0)) {
	printf("Error: Label <z-size> not found in fabric configuration file\n");
        goto out1;
    }

    if (!parse_json_bool(fab->child, "strict", &cfg_fabric.strict, 0))
	cfg_fabric.strict = 0;

    list = json_find_first_label(fab->child, "nodes");
    if (!(list && list->child && list->child->type == JSON_ARRAY)) {
        printf("Error: Label <nodes> not found in fabric configuration file\n");
        goto out1;
    }

    for (cfg_nodes = 0, obj = list->child->child; obj; obj = obj->next)
        cfg_nodes++;

    cfg_nodelist = malloc(cfg_nodes * sizeof(*cfg_nodelist));
    assert(cfg_nodelist);

    for (i = 0, obj = list->child->child; obj; obj = obj->next, i++) {
	/* UUID is optional */
	if (!parse_json_num(obj, "uuid", &cfg_nodelist[i].uuid, 1)) {
	    cfg_nodelist[i].uuid = 0;
	    name_matching = 1;
	}

	if (!parse_json_num(obj, "sciid", &cfg_nodelist[i].sciid, 0)) {
	    printf("Error: Label <sciid> not found in fabric configuration file\n");
	    goto out2;
	}

        if (!parse_json_num(obj, "partition", &cfg_nodelist[i].partition, 0)) {
	    printf("Error: Label <partition> not found in fabric configuration file\n");
	    goto out2;
	}

	/* OSC is optional */
	if (!parse_json_num(obj, "osc", &cfg_nodelist[i].osc, 1))
	    cfg_nodelist[i].osc = 0;

	if (!parse_json_str(obj, "desc", cfg_nodelist[i].desc, 32, 0)) {
	    printf("Error: Label <desc> not found in fabric configuration file\n");
	    goto out2;
        }

	if (parse_json_num(obj, "sync-only", &val, 1))
	    cfg_nodelist[i].sync_only = val;
	else
	    cfg_nodelist[i].sync_only = 0;
    }

    if (name_matching)
	printf("UUIDs omitted - matching hostname to <desc> label\n");

    list = json_find_first_label(fab->child, "partitions");
    if (!(list && list->child && list->child->type == JSON_ARRAY)) {
        printf("Error: Label <partitions> not found in fabric configuration file\n");
        free(cfg_nodelist);
        return 0;
    }

    for (cfg_partitions = 0, obj = list->child->child; obj; obj = obj->next)
        cfg_partitions++;

    cfg_partlist = malloc(cfg_partitions * sizeof(*cfg_partlist));
    assert(cfg_partlist);

    for (i = 0, obj = list->child->child; obj; obj = obj->next, i++) {
	if (!parse_json_num(obj, "master", &cfg_partlist[i].master, 0)) {
	    printf("Error: Label <master> not found in fabric configuration file\n");
	    goto out3;
	}

	if (!parse_json_num(obj, "builder", &cfg_partlist[i].builder, 0)) {
	    printf("Error: Label <builder> not found in fabric configuration file\n");
	    goto out3;
	}
    }

    return 1;

out3:
    free(cfg_nodelist);
out2:
    free(cfg_partlist);
out1:
    return 0;
}

int parse_config_file(char *data)
{
    json_t *root = NULL;
    enum json_error err;
    int i;

    if (verbose >= 2)
	printf("Fabric configuration file:\n%s", data);

    err = json_parse_document(&root, data);
    if (err != JSON_OK) {
	printf("Error: Failed to parse fabric configuration (reason %d)\n", err);
	return 0;
    }

    if (!parse_json(root)) {
	printf("Error: Parsing fabric configuration root failed\n");
	json_free_value(&root);
	return 0;
    }

    printf("Fabric dimensions: x: %d, y: %x, z: %d\n",
	cfg_fabric.x_size, cfg_fabric.y_size, cfg_fabric.z_size);

    for (i = 0; i < cfg_nodes; i++)
	printf("Node %d: <%s> uuid: %08X, sciid: 0x%03x, partition: %d, osc: %d, sync-only: %d\n",
	    i, cfg_nodelist[i].desc, cfg_nodelist[i].uuid,
	    cfg_nodelist[i].sciid, cfg_nodelist[i].partition,
	    cfg_nodelist[i].osc, cfg_nodelist[i].sync_only);

    for (i = 0; i < cfg_partitions; i++)
	printf("Partition %d: master: 0x%03x, builder: 0x%03x\n",
	    i, cfg_partlist[i].master, cfg_partlist[i].builder);

    return 1;
}

void make_singleton_config(uint32_t uuid)
{
    cfg_fabric.x_size = 1;
    cfg_fabric.y_size = 0;
    cfg_fabric.z_size = 0;

    cfg_nodes = 1;
    cfg_nodelist = malloc(sizeof(*cfg_nodelist));
    assert(cfg_nodelist);

    cfg_nodelist[0].uuid = uuid;
    cfg_nodelist[0].sciid = 0;
    cfg_nodelist[0].osc = 0;
    cfg_nodelist[0].partition = 0;
    memcpy(cfg_nodelist[0].desc, "self", 5);
    cfg_nodelist[0].sync_only = 1;

    cfg_partitions = 1;
    cfg_partlist = malloc(sizeof(*cfg_partlist));
    assert(cfg_partlist);

    cfg_partlist[0].master = 0;
    cfg_partlist[0].builder = 0;
}

/* Must only be used to get local config */
struct node_info* get_node_config(uint32_t uuid)
{
    int i;

    for (i = 0; i < cfg_nodes; i++)
	if (config_local(&cfg_nodelist[i], uuid))
		return &cfg_nodelist[i];

    printf("Error: Failed to find node config (hostname %s)\n", hostname ? hostname : "<none>");
    return NULL;
}

struct part_info* get_partition_config(int idx)
{
    if (idx < cfg_partitions)
        return &cfg_partlist[idx];
    else
        return NULL;
}

char *get_master_name(uint32_t sciid)
{
    int i;
    
    for (i = 0; i < cfg_nodes; i++)
	if (cfg_nodelist[i].sciid == sciid)
	    return cfg_nodelist[i].desc;
    return "<not found>";
}
