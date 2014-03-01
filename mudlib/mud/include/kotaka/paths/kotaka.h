/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>

/* daemons */

#define CHANNELD	(USR_DIR + "/Kotaka/sys/channeld")
#define SUBD		(USR_DIR + "/Kotaka/sys/subd")
#define DUMPD		(USR_DIR + "/Kotaka/sys/dumpd")
#define PARSE_DUMP	(USR_DIR + "/Kotaka/sys/parse/dump")

/* libs */

#define LIB_KOTAKA_USER		(USR_DIR + "/Kotaka/lib/user")
#define LIB_USTATE		(USR_DIR + "/Kotaka/lib/ustate")
#define LIB_DRIVER		(USR_DIR + "/Kotaka/lib/driver")
#define LIB_DEEP_COPY		(USR_DIR + "/Kotaka/lib/deep_copy")
