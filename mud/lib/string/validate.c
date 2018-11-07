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
#include <kotaka/paths/string.h>

inherit "case";
inherit "char";

int is_whitespace(string str)
{
	int len;
	int iter;

	len = strlen(str);

	for (iter = 0; iter < len; iter++) {
		if (!char_is_whitespace(str[iter]))
			return 0;
	}

	return 1;
}

int is_capitalized(string test)
{
	int char;

	char = test[0];

	return (char >= 'A' && char <= 'Z');
}

int string_has_char(int char, string str)
{
	int len, iter;

	len = strlen(str);
	for (iter = 0; iter < len; iter++) {
		if (str[iter] == char)
			return 1;
	}
	return 0;
}

int is_valid_base_id(string str)
{
	return STRINGD->regex_match(str, "[A-Za-z][-+A-Za-z0-9_]*");
}

int is_valid_id(string str)
{
	return STRINGD->regex_match(str, "[A-Za-z][-+A-Za-z0-9_]*(#[1-9][0-9]*)?");
}

int is_alpha(string str)
{
	return STRINGD->regex_match(str, "[a-zA-Z]+");
}

int is_alphanum(string str)
{
	return STRINGD->regex_match(str, "[a-zA-Z0-9]+");
}

int is_valid_ident(string str)
{
	return STRINGD->regex_match(str, "[A-Za-z][_a-zA-Z0-9]*");
}

int is_valid_username(string str)
{
	return str && strlen(str) >= 1 && strlen(str) <= 32 &&
		STRINGD->regex_match(str, "[A-Za-z][_A-Za-z0-9]*");
}
