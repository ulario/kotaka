/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kotaka/paths/thing.h>
#include <kotaka/paths/system.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

static void create()
{
	KERNELD->set_global_access("Thing", 1);

	load_dir("obj", 1);
	load_dir("sys", 1);
}

int forbid_inherit(string from, string path, int priv)
{
	switch(path) {
	case USR_DIR + "/Thing/lib/thing":
		/* no private inheritance allowed */
		return priv;
	default:
		/* no partial inheritance allowed */
		return !(DRIVER->creator(from) == "Thing");
	}
}

string query_destructor(string path)
{
	switch(path) {
	case LIB_THING:
		return "thing_destruct";
	}
}

string query_toucher(string path)
{
	switch(path) {
	case USR_DIR + "/Thing/lib/archetype":
		return "touch_archetype";
	case USR_DIR + "/Thing/lib/inventory":
		return "touch_inventory";
	case USR_DIR + "/Thing/lib/property":
		return "touch_properties";
	}
}
