/*
 * Copyright (C) 2008-2014 Numascale AS, support@numascale.com
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

#pragma once

#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define roundup(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))

#define assertf(cond, format, args...) do { if (!(cond)) { \
  fprintf(stderr, "Error: "); \
  fprintf(stderr, format, ## args); \
  fprintf(stderr, "\n"); \
  _exit(1); \
} } while (0)

#define sysassertf(cond, format, args...) do { if (!(cond)) { \
  fprintf(stderr, "Error: "); \
  fprintf(stderr, format, ## args); \
  fprintf(stderr, " with '%s'\n", strerror(errno)); \
  _exit(1); \
} } while (0)

#define error(format, args...) do { \
  fprintf(stderr, "Error: "); \
  fprintf(stderr, format, ## args); \
  fprintf(stderr, "\n"); \
  exit(1); \
} while (0)

#define warn(format, args...) do { \
  fprintf(stderr, "Warning: "); \
  fprintf(stderr, format, ## args); \
  fprintf(stderr, "\n"); \
} while (0)

#define warn_once(format, args...) do { \
  static bool done = 0; \
  if (!done) { \
    fprintf(stderr, "Warning: "); \
    fprintf(stderr, format, ## args); \
    fprintf(stderr, "\n"); \
    done = 1; \
  } \
} while (0)

#define syserror(format, args...) do { \
  fprintf(stderr, "Error: "); \
  fprintf(stderr, format, ## args); \
  fprintf(stderr, " with '%s'\n", strerror(errno)); \
  exit(1); \
} while (0)

#define FLAGS_VERBOSE 1
#define FLAGS_DEBUG 2

static inline uint64_t roundup_nextpow2(const uint64_t val)
{
	const uint64_t next = 1ULL << (64 - __builtin_clzll(val));
	if (val & ((next >> 1) - 1))
		return next;
	return val;
}
