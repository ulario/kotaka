/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

private void usage()
{
	send_out("gadd      Add global alias\n");
	send_out("gdel      Delete global alias\n");
	send_out("glist     List global aliases\n");
	send_out("gsave     Save global aliases\n");
	send_out("grestore  Restore global aliases\n");
}

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string alias;
	string first;
	string args;

	if (query_user()->query_class() < 3) {
		send_out("Permission denied.\n");
		return;
	}

	args = roles["raw"];

	if (!args) {
		send_out("Usage:\n");
		usage();
		return;
	}

	if (!sscanf(args, "%s %s", first, args)) {
		first = args;
		args = nil;
	}

	switch(first) {
	case "gadd":
		if (!sscanf(args, "%s %s", alias, args)) {
			send_out("Please specify the alias and the expansion.\n");
			break;
		}

		if (alias == "alias") {
			send_out("The alias command cannot be aliased.\n");
			return;
		}

		ALIASD->set_alias(alias, args);
		send_out("Done.\n");
		break;

	case "gdel":
		if (sscanf(args, "%*s %*s")) {
			send_out("Don't specify an expansion when removing an alias\n");
			break;
		}

		ALIASD->set_alias(args, nil);
		send_out("Done.\n");
		break;

	case "glist":
		if (args && sscanf(args, "%s %s", alias, args)) {
			send_out("Don't specify an expansion when listing aliases\n");
			break;
		}

		send_out("Aliases\n");
		send_out("-------\n");

		{
			string *aliases;
			int i, sz;

			aliases = ALIASD->query_aliases();

			sz = sizeof(aliases);

			for (i = 0; i < sz; i++) {
				alias = aliases[i];
				send_out(alias + " - " + ALIASD->query_alias(alias) + "\n");
			}
		}
		break;

	case "gsave":
		if (args && sscanf(args, "%s %s", alias, args)) {
			send_out("Don't specify an expansion when saving aliases\n");
			break;
		}

		ALIASD->save();
		send_out("Saved aliases.\n");
		break;

	case "grestore":
		if (args && sscanf(args, "%s %s", alias, args)) {
			send_out("Don't specify an expansion when loading aliases\n");
			break;
		}

		ALIASD->restore();
		send_out("Restored aliases.\n");
		break;

	default:
		send_out("Invalid alias command.\n");
		send_out("Choices:\n");
		usage();
	}
}
