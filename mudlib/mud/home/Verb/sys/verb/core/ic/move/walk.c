/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/string.h>
#include <kotaka/paths/verb.h>

inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	int number;
	int px, py;
	object env;
	string stepqueue;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	env = actor->query_environment();

	if (!env) {
		send_out("You cannot walk without an environment.\n");
		return;
	}

	roles["raw"] = STRINGD->to_lower(roles["raw"]);
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
				stepqueue += STRINGD->chars('n', number);
			} else {
				stepqueue += "n";
			}
			number = 0;
			break;

		case 'e':
			if (number) {
				stepqueue += STRINGD->chars('e', number);
			} else {
				stepqueue += "e";
			}
			number = 0;
			break;

		case 's':
			if (number) {
				stepqueue += STRINGD->chars('s', number);
			} else {
				stepqueue += "s";
			}
			number = 0;
			break;

		case 'w':
			if (number) {
				stepqueue += STRINGD->chars('w', number);
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
			send_out("You could not complete your walk.\n");
			return;
		}

		/* We cannot go out of bounds if we're inside a hard object */
		/* We cannot enter a hard object */

		actor->set_x_position(px);
		actor->set_y_position(py);

		/* todo: check to see if we entered or exited a soft object */
	}

	/* check and make sure that the target position is allowed */
}
