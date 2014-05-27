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
#define lassert(cond) do { if (!(cond)) { \
        printf("Error: assertion '%s' failed in %s at %s:%d\n", \
            #cond, __FUNCTION__, __FILE__, __LINE__); while (1); \
    } } while (0)
#define IMPORT_RELOCATED(sym) extern volatile uint8_t sym ## _relocate
#define REL8(sym) ((uint8_t *)((volatile uint8_t *)asm_relocated + ((volatile uint8_t *)&sym ## _relocate - (volatile uint8_t *)&asm_relocate_start)))
#define REL16(sym) ((uint16_t *)((volatile uint8_t *)asm_relocated + ((volatile uint8_t *)&sym ## _relocate - (volatile uint8_t *)&asm_relocate_start)))
#define REL32(sym) ((uint32_t *)((volatile uint8_t *)asm_relocated + ((volatile uint8_t *)&sym ## _relocate - (volatile uint8_t *)&asm_relocate_start)))
#define REL64(sym) ((uint64_t *)((volatile uint8_t *)asm_relocated + ((volatile uint8_t *)&sym ## _relocate - (volatile uint8_t *)&asm_relocate_start)))

typedef uint8_t ht_t;
typedef uint16_t sci_t;

template<class T> class Vector {
	unsigned max;

	void ensure(void) {
		lassert(used <= max);
		if (used == max) {
			max += 8;
			elements = (T *)realloc((void *)elements, sizeof(T) * max);
		}
	}
public:
	unsigned used;
	T *elements, *limit;
	Vector(void): max(0), used(0), elements(NULL), limit(NULL) {}
	~Vector(void) {
		free(elements);
	}

	T operator[](const unsigned pos) {
		lassert(pos < used);
		return elements[pos];
	}

	void add(T elem) {
		ensure();
		elements[used++] = elem;
		limit = &elements[used];
	}

	void del(const unsigned offset) {
		lassert(offset < used);
		memmove(&elements[offset], &elements[offset + 1], (used - offset - 1) * sizeof(T));
		used--;
		limit = &elements[used];
	}

	void insert(T elem, const unsigned pos) {
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

