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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

void main(object actor, mapping roles)
{
	object first;
	object obj;
	object proxy;

	if (query_user()->query_class() < 3) {
		send_out("Only an administrator can nuke a subsystem\n");
		return;
	}

	first = KERNELD->first_link(roles["raw"]);

	if (!first) {
		send_out(roles["raw"] + " owns no objects.\n");
		return;
	}

	proxy = PROXYD->get_proxy(query_user()->query_name());

	do {
		proxy->destruct_object(first);
	} while (first = KERNELD->first_link(roles["raw"]));
}
