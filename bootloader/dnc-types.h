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

#ifndef __DNC_TYPES
#define __DNC_TYPES 1

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define checked __attribute__ ((warn_unused_result))

typedef uint8_t ht_t;
typedef uint16_t sci_t;

template<class T> class Vector {
	int max;
public:
	int used;
	T *elements;
	Vector(void): max(0), used(0), elements(NULL) {}
	~Vector(void) {
		while (used) {
			delete elements[used - 1];
			used--;
		}

		free(elements);
	}

	void add(T elem) {
		if (used >= max) {
			max += 8;
			elements = (T *)realloc((void *)elements, sizeof(T) * max);
		}

		elements[used++] = elem;
	}
};

#endif

