/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
#include <kernel/user.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Kqreset";
}

string *query_help_contents()
{
	return ({ "Removes all quotas for a specific user." });
}

void main(object actor, mapping roles)
{
	int sz;
	object proxy;
	string args, *resources;

	if (query_user()->query_class() < 3) {
		send_out("Permission denied.\n");
		return;
	}

	args = roles["raw"];

	if (!args) {
		send_out("Usage: kqreset <owner>\n");
		return;
	}

	if (args == "Ecru") {
		args = nil;
	}

	resources = KERNELD->query_resources();
	proxy = PROXYD->get_proxy(query_user()->query_name());

	for (sz = sizeof(resources), --sz; sz >= 0; --sz) {
		proxy->rsrc_set_limit(args, resources[sz], -1);
	}

	send_out("Resource limits for " + (args ? args : "Ecru") + " removed.\n");
}
