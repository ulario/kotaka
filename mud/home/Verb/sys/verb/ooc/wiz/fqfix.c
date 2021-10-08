/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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
#include <kotaka/log.h>

inherit LIB_VERB;
inherit "~Text/lib/sub";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Fqfix";
}

string *query_help_contents()
{
	return ({ "Audits and reconciles filequota" });
}

void main(object actor, mapping roles)
{
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard or an administrator can perform a filequota update.\n");
		return;
	}

	if (roles["raw"]) {
		send_out("Usage: fqfix\n");
		return;
	}

	LOGD->post_message("system", LOG_NOTICE, user->query_name() + " is doing a filequota audit.");
	KERNELD->fix_filequota();
}
