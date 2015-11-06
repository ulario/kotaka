/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kotaka/privilege.h>
#include <status.h>

#define MIN_SPARE_OBJECTS 100

private int enough_free_objects(int clone)
{
	int used;
	int free;
	int total;

	used = status(ST_NOBJECTS);
	total = status(ST_OTABSIZE);
	free = total - used;

	if (clone) {
		if (free < 100) {
			return 0;
		}

		if (free * 100 / total <= 10) {
			return 0;
		}
	} else {
		if (free < 50) {
			return 0;
		}

		if (free * 100 / total <= 5) {
			return 0;
		}
	}

	return 1;
}

static object compile_object(mixed args ...)
{
	object obj;

	obj = find_object(args[0]);

	if (!SYSTEM() &&
		DRIVER->creator(args[0]) != "System" &&
		!obj && !enough_free_objects(0)) {
		error("Too many objects");
	}

	return ::compile_object(args ...);
}

static object load_object(mixed args ...)
{
	object obj;

	obj = find_object(args[0]);

	if (obj) {
		return obj;
	}

	if (!SYSTEM() &&
		DRIVER->creator(args[0]) != "System" &&
		!obj && !enough_free_objects(0)) {
		error("Too many objects");
	}

	return ::compile_object(args ...);
}

static object clone_object(mixed args ...)
{
	if (!SYSTEM() &&
		query_owner() != "System" &&
		!enough_free_objects(1)) {
		error("Too many objects");
	}

	return ::clone_object(args ...);
}
