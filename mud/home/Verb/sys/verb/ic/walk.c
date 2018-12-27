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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_EMIT;
inherit LIB_VERB;
inherit "/lib/string/char";
inherit "/lib/string/case";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Walk";
}

string *query_help_contents()
{
	return ({
		"Changes your position.",
		"To walk, specify the path you wish to take.  The path is just a string of" +
		" n's, e's, s's, or w's, possibly with numbers in front, that say where to" +
		" go and how far.",
		"All units are in meters."
	});
}

void main(object actor, mapping roles)
{
	int number;
	int px, py;
	object env;
	string stepqueue;
	int boundflag;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	env = actor->query_environment();

	if (!env) {
		send_out("You cannot walk without an environment.\n");
		return;
	}

	roles["raw"] = to_lower(roles["raw"]);
	px = actor->query_property("pos_x");
	py = actor->query_property("pos_y");

	stepqueue = "";

	while (strlen(roles["raw"])) {
		switch(roles["raw"][0]) {
		case '0' .. '9':
			number *= 10;
			number += roles["raw"][0] - '0';
			break;

		case 'n':
			if (number) {
				stepqueue += chars('n', number);
			} else {
				stepqueue += "n";
			}
			number = 0;
			break;

		case 'e':
			if (number) {
				stepqueue += chars('e', number);
			} else {
				stepqueue += "e";
			}
			number = 0;
			break;

		case 's':
			if (number) {
				stepqueue += chars('s', number);
			} else {
				stepqueue += "s";
			}
			number = 0;
			break;

		case 'w':
			if (number) {
				stepqueue += chars('w', number);
			} else {
				stepqueue += "w";
			}
			number = 0;
			break;

		default:
			send_out("Invalid walk string: " + roles["raw"] + "\n");
			roles["raw"] = "";
			continue;
		}

		roles["raw"] = roles["raw"][1 ..];
	}

	px = actor->query_x_position();
	py = actor->query_y_position();

	while (strlen(stepqueue)) {
		int step;
		object env;

		step = stepqueue[0];
		stepqueue = stepqueue[1 ..];

		switch(step) {
		case 'n':
			py--;
			break;

		case 'e':
			px++;
			break;

		case 's':
			py++;
			break;

		case 'w':
			px--;
			break;

		default:
			send_out("Invalid step.\n");
			break;
		}

		/* todo: validate (px, py) */

		env = actor->query_environment();

		if (!env) {
			actor->set_x_position(px);
			actor->set_y_position(py);

			continue;
		}

		if (px < 0 || py < 0 || px >= env->query_x_size() || py >= env->query_y_size()) {
			if (env->query_property("boundary_type") == 1) {
				send_out("You could not complete your walk.\n");
				return;
			} else {
				boundflag = 1;
			}
		}

		/* We cannot go out of bounds if we're inside a hard object */
		/* We cannot enter a hard object */

		actor->set_x_position(px);
		actor->set_y_position(py);

		/* todo: check to see if we entered or exited a soft object */
	}

	if (boundflag) {
		send_out("You notice that you're out of bounds.\n");
	}

	/* check and make sure that the target position is allowed */
}
