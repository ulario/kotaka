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
#include <kotaka/log.h>

inherit "/lib/string/case";

static string pinkfish2ansi(string input)
{
	string *slices;
	int sz, i;

	slices = explode("%^" + input + "%^", "%^");
	sz = sizeof(slices);

	if (sz == 1) {
		return input;
	}

	for (i = 1; i < sz - 1; i++) {
		string fish;

		fish = slices[i];

		switch(fish) {
		case "":
			break;
		case "BOLD":
			fish = "\033[1m";
			break;
		case "RESET":
			fish = "\033[0m";
			break;
		case "RED":
			fish = "\033[31m";
			break;
		case "GREEN":
			fish = "\033[32m";
			break;
		case "ORANGE":
		case "YELLOW":
			fish = "\033[33m";
			break;
		case "BLUE":
			fish = "\033[34m";
			break;
		case "CYAN":
			fish = "\033[35m";
			break;
		case "MAGENTA":
			fish = "\033[36m";
			break;
		case "B_RED":
			fish = "\033[1;31m";
			break;
		case "B_GREEN":
			fish = "\033[1;32m";
			break;
		case "B_YELLOW":
			fish = "\033[1;33m";
			break;
		case "B_BLUE":
			fish = "\033[1;34m";
			break;
		case "B_CYAN":
			fish = "\033[1;35m";
			break;
		case "B_MAGENTA":
			fish = "\033[1;36m";
			break;
		case "WHITE":
			fish = "\033[37m";
			break;
		default:
			/* if all caps, it's probably a pinkfish code */
			if (fish == to_upper(fish)) {
				LOGD->post_message("system", LOG_NOTICE, "Potential untranslated pinkfish code " + fish);
			}
		}

		slices[i] = fish;
	}

	return implode(slices, "");
}
