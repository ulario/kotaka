/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <kotaka/log.h>

mapping touches;

static void call_touch(object obj)
{
	string cpath;
	string opath;

	if (previous_program() == OBJECTD) {
		::call_touch(obj);
		return;
	}

	cpath = object_name(this_object());
	opath = object_name(obj);

	ACCESS_CHECK(DRIVER->creator(cpath) == DRIVER->creator(opath));

	OBJECTD->call_touch(obj);
}

static int touch(string function)
{
	if (sscanf(object_name(this_object()), USR_DIR + "/System/%*s")) {
		return 1;
	}
}

nomask int _F_call_touch(string function)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return touch(function);
}
