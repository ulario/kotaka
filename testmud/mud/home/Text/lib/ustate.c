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
#include <kotaka/paths.h>
#include <text/paths.h>

/* overrides LIB_USTATE to provide auditing */

inherit LIB_USTATE;

static void terminate_account_state()
{
	object parent;

	parent = query_parent();

	if (parent <- "~/obj/ustate/start") {
		parent->swap_state(clone_object("shell"));
	} else {
		pop_state();
	}
}

int execute_command(string command, string args)
{
	if (BIND->execute_command("adm/" + first, input))
		return 1;
	if (BIND->execute_command("wiz/tool/" + first, input))
		return 1;
	if (BIND->execute_command("wiz/debug/" + first, input))
		return 1;
	if (BIND->execute_command("wiz/" + first, input))
		return 1;
	if (BIND->execute_command("test/" + first, input))
		return 1;
	if (BIND->execute_command("movie/" + first, input))
		return 1;
	if (BIND->execute_command(first, input))
		return 1;
	return 0;
}
