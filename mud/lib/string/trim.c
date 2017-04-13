/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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

inherit "char";

string trim_whitespace(string str)
{
	int start, end;

	if (!str || str == "")
		return str;

	start = 0;
	end = strlen(str) - 1;

	while ((start <= end) && char_is_whitespace(str[start])) {
		start++;
	}

	while ((start <= end) && char_is_whitespace(str[end])) {
		end--;
	}

	return str[start .. end];
}

string rtrim_null(string str)
{
	int end;

	if (!str || str == "")
		return str;

	end = strlen(str) - 1;

	while (end >= 0 && str[end] == 0)
		end--;

	return str[.. end];
}

string rtrim_whitespace(string str)
{
	int end;

	if (!str || str == "")
		return str;

	end = strlen(str) - 1;

	while ((end >= 0) && char_is_whitespace(str[end])) {
		end--;
	}

	return str[.. end];
}
