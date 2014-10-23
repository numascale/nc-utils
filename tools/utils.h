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
#include <string.h>
#include <errno.h>
#include <unistd.h>

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
  fprintf(stderr, " with %s\n", strerror(errno)); \
  _exit(1); \
} } while (0)

#define error(format, args...) do { \
  fprintf(stderr, "Error: "); \
  fprintf(stderr, format, ## args); \
  fprintf(stderr, "\n"); \
  exit(1); \
} while (0)

#define syserror(format, args...) do { \
  fprintf(stderr, "Error: "); \
  fprintf(stderr, format, ## args); \
  fprintf(stderr, " with %s\n", strerror(errno)); \
  exit(1); \
} while (0)

#define FLAGS_VERBOSE 1
#define FLAGS_DEBUG 3
