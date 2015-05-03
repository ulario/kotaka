/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

string grammar;

static void create()
{
	::create();

	grammar = read_file("~/data/parse/value.dpd");
}

void upgrading()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	::create();

	grammar = read_file("~/data/parse/value.dpd");
}

mixed parse(string input)
{
	mixed *ret;

	ret = parse_string(grammar, input);

	if (!ret) {
		error("Parse failure");
	}

	return ret[0];
}

static string *parse_str(string *input)
{
	return ({ STRINGD->quote_unescape(input[0][1 .. strlen(input[0]) - 2]) });
}

static int *parse_int(string *input)
{
	return ({ (int)input[0] });
}

static float *parse_flt(string *input)
{
	return ({ (float)input[0] });
}

static object *parse_obj(string *input)
{
	string oname, osubname;
	string *parts;
	object root, obj;

	oname = input[0];
	oname = oname[1 .. strlen(oname) - 2];	/* strip off angle brackets */

	obj = find_object(oname);

	if (!obj) {
		obj = CATALOGD->lookup_object(oname);
	}

	if (obj) {
		return ({ obj });
	} else {
		error("No such object");
	}
}

static mixed *parse_nil(string *input)
{
	return ({ nil });
}

static mixed *pick_middle(mixed *input)
{
	if (sizeof(input) % 2 != 1)
		error("Illegal input");
	return input[sizeof(input) / 2 .. sizeof(input) / 2];
}

static mixed **make_arr(mixed *input)
{
	return ({ input });
}

static mixed **make_empty_arr(string *input)
{
	return ({ ({ }) });
}

static mixed *build_arr(mixed *input)
{
	return ({ input[0] + ({ input[2] }) });
}

static mapping *make_map(mixed *input)
{
	return ({ ([ input[0] : input[1] ]) });
}

static mapping *make_empty_map(string *input)
{
	return ({ ([ ]) });
}

static mapping *build_map(mixed *input)
{
	return ({ input[0] + ([ input[2] : input[3] ]) });
}

static mixed *strike_middle(mixed *input)
{
	return ({ input[0], input[sizeof(input) - 1] });
}

static object *make_lwo(mixed *input)
{
	string name;
	mixed value;
	object obj;

	name = input[1];
	value = input[3];

	obj = new_object(name[1 .. strlen(name) - 2]);
	obj->sprint_restore(value);

	return ({ obj });
}
