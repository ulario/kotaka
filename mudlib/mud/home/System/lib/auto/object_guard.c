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

static int free_objects()
{
	return status(ST_OTABSIZE) - status(ST_NOBJECTS);
}

static object compile_object(mixed args ...)
{
	object obj;

	obj = find_object(args[0]);

	if (!SYSTEM() &&
		DRIVER->creator(args[0]) != "System" &&
		!obj && free_objects() < status(ST_OTABSIZE) / 20) {
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
		!obj && free_objects() < status(ST_OTABSIZE) / 20) {
		error("Too many objects");
	}

	return ::compile_object(args ...);
}

static object clone_object(mixed args ...)
{
	if (!SYSTEM() &&
		query_owner() != "System" &&
		free_objects() < status(ST_OTABSIZE) / 20) {
		error("Too many objects");
	}

	return ::clone_object(args ...);
}
