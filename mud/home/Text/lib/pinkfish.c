/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021, 2022  Raymond Jennings
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

private string ansi_of_fish(string fish)
{
	switch(fish) {
	case "BOLD":
	case "BLINK":
		return "\033[1m";

	case "RESET":
		return "\033[0m";

	case "RED":
		return "\033[31m";

	case "GREEN":
		return "\033[32m";

	case "ORANGE":
	case "YELLOW":
		return "\033[33m";

	case "BLUE":
		return "\033[34m";

	case "CYAN":
		return "\033[35m";

	case "MAGENTA":
		return "\033[36m";

	case "B_RED":
		return "\033[1;31m";

	case "B_GREEN":
		return "\033[1;32m";

	case "B_YELLOW":
		return "\033[1;33m";

	case "B_BLUE":
		return "\033[1;34m";

	case "B_CYAN":
		return "\033[1;35m";

	case "B_MAGENTA":
		return "\033[1;36m";

	case "WHITE":
		return "\033[37m";
	}
}

static string pinkfish2ansi(string input)
{
	string output, prefix, fish;
	output = "";

	rlimits (0; 25000) {
		while (sscanf(input, "%s%%^%s%%^%s", prefix, fish, input) == 3) {
			string ansi;

			output += prefix;

			ansi = ansi_of_fish(fish);

			if (!ansi) {
				ansi = fish;

				/* if all caps, it's probably a pinkfish code */
				if (fish == to_upper(fish)) {
					LOGD->post_message("system", LOG_NOTICE, "Potential untranslated pinkfish code " + fish);
				}
			}

			output += ansi;
		}

		output += input;
	}

	return output;
}
