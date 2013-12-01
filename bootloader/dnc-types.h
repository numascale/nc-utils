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

	void ensure(void) {
		if (used >= max) {
			max += 8;
			elements = (T *)realloc((void *)elements, sizeof(T) * max);
		}
	}
public:
	int used;
	T *elements, *limit;
	Vector(void): max(0), used(0), elements(NULL), limit(NULL) {}
	~Vector(void) {
		free(elements);
	}

	void add(T elem) {
		ensure();
		elements[used++] = elem;
		limit = &elements[used];
	}

	void del(const int offset) {
		memmove(&elements[offset], &elements[offset + 1], (used - offset - 1) * sizeof(T));
		used--;
		limit = &elements[used];
	}

	void insert(T elem, const int pos) {
		ensure();

		/* Move any later elements down */
		if (used > pos)
			memmove(&elements[pos + 1], &elements[pos], (used - pos) * sizeof(T));

		elements[pos] = elem;
		used++;
		limit = &elements[used];
	}
};

#endif

