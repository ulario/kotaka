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
#include <kotaka/paths.h>

inherit SECOND_AUTO;

string replace(string input, string from, string to)
{
	string *arr;

	arr = explode(from + input + from, from);

	return implode(arr, to);
}

string quote_escape(string input)
{
	input = replace(input, "\\", "\\\\");
	input = replace(input, "\"", "\\\"");
	input = replace(input, "\n", "\\n");
	input = replace(input, "\r", "\\r");
	input = replace(input, "\t", "\\t");
	
	return input;
}

string quote_unescape(string input)
{
	input = replace(input, "\\t", "\t");
	input = replace(input, "\\r", "\r");
	input = replace(input, "\\n", "\n");
	input = replace(input, "\\\"", "\"");
	input = replace(input, "\\\\", "\\");

	return input;
}
