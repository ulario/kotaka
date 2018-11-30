/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/paths/intermud.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object user;
	string target;
	string mud;
	string msg;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You have insufficient access to send i3 tells.\n");
		return;
	}

	if (sscanf(roles["raw"], "\"%s@%s\" %s", target, mud, msg) != 3) {
		if (sscanf(roles["raw"], "%s@%s %s", target, mud, msg) != 3) {
			send_out("Usage: itell user@mud message\n");
			send_out("If the mud has spaces in its name, put user@mud in quotation marks.\n");
			return;
		}
	}

	if (!find_object(INTERMUDD)) {
		send_out("IntermudD is offline.\n");
		return;
	}

	INTERMUDD->send_tell(user->query_name(), user->query_titled_name(), mud, target, msg);

	user->message("You tell " + target + "@" + mud + ": " + msg + "\n");
}
