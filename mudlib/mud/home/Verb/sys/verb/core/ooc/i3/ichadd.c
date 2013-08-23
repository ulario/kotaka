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
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string chan;
	int type;
	string rest;

	if (query_user()->query_class() < 3) {
		send_out("You have insufficient access to create i3 channels.\n");
		return;
	}

	if (sscanf(roles["raw"], "%s %s", chan, rest)) {
		if (!sscanf(roles["raw"], "%s %d", chan, type)) {
			send_out("Channel type must be a number, idiot.\n");
			return;
		}
	} else {
		chan = roles["raw"];
	}

	"~Intermud/sys/intermudd"->add_channel(chan, type);
}
