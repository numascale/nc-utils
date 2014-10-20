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

#define _GNU_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "dnc-config.h"
#include "dnc-commonlib.h"
#include "dnc-route.h"

uint8_t dims[] = {0, 1, 2};

int cfg_nodes = 16;
struct fabric_info cfg_fabric;
struct node_info *cfg_nodelist;

int main(int argc, char **argv)
{
	cfg_fabric.size[0] = 4;
	cfg_fabric.size[1] = 4;
	cfg_fabric.size[2] = 0;

	cfg_nodelist = (struct node_info *) malloc(cfg_nodes * sizeof(struct node_info));

	cfg_nodelist[0].sci = 0x000;
	cfg_nodelist[1].sci = 0x001;
	cfg_nodelist[2].sci = 0x002;
	cfg_nodelist[3].sci = 0x003;
	cfg_nodelist[4].sci = 0x010;
	cfg_nodelist[5].sci = 0x011;
	cfg_nodelist[6].sci = 0x012;
	cfg_nodelist[7].sci = 0x013;
	cfg_nodelist[8].sci = 0x020;
	cfg_nodelist[9].sci = 0x021;
	cfg_nodelist[10].sci = 0x022;
	cfg_nodelist[11].sci = 0x023;
	cfg_nodelist[12].sci = 0x030;
	cfg_nodelist[13].sci = 0x031;
	cfg_nodelist[14].sci = 0x032;
	cfg_nodelist[15].sci = 0x033;

	const char names[][4] = {"SIU", "XA", "XB", "YA", "YB", "ZA", "ZB", "ZB"};

	for (int j = 0; j < cfg_nodes; j++) {
		sci_t sci = cfg_nodelist[j].sci;
		printf("-------------------------------\n");
		for (int i = 0; i < cfg_nodes; i++) {
			uint8_t out;
			if (sci == cfg_nodelist[i].sci)
				continue;

			out = router0(sci, i);
			printf("router 0 %03x routing to %03x via %s\n", sci, cfg_nodelist[i].sci, names[out]);
			out = router1(sci, i);
			printf("router 1 %03x routing to %03x via %s\n", sci, cfg_nodelist[i].sci, names[out]);
		}

	}

	return 0;
}
