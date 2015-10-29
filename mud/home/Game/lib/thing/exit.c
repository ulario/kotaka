/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2014  Raymond Jennings
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
#include <kotaka/paths/thing.h>

mixed query_local_property(string prop);
void set_local_property(string prop, mixed value);

static void create()
{
}

object query_exit_destination()
{
	object obj;

	obj = query_local_property("exit_return");

	if (obj) {
		return obj->query_environment();
	}

	return query_local_property("exit_destination");
}

atomic void set_exit_destination(object obj)
{
	object lobj;

	lobj = query_local_property("exit_return");

	if (lobj && obj != nil) {
		error("Exit has a return exit");
	}

	set_local_property("exit_destination", obj);
}

object query_exit_return()
{
	return query_local_property("exit_return");
}

atomic void set_exit_return(object obj)
{
	object lobj;

	if (query_local_property("exit_destination")) {
		error("Exit already has a destination");
	}

	if (obj->query_local_property("exit_destination")) {
		error("Return exit already has a destination");
	}

	if (obj->query_local_property("exit_return")) {
		error("Return exit already has a return exit");
	}

	lobj = query_local_property("exit_return");

	if (lobj) {
		object llobj;

		llobj = lobj->query_local_property("exit_return");

		if (llobj == this_object()) {
			llobj->set_local_property("exit_return", nil);
		}
	}

	lobj = obj->query_local_property("exit_return");

	if (lobj) {
		error("Return exit already has a return exit");
	}

	obj->set_local_property("exit_return", this_object());
	set_local_property("exit_return", obj);
}
