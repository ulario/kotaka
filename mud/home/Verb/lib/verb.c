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
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

static object query_ustate()
{
	return ENGLISHD->query_current_ustate();
}

static object query_user()
{
	return query_ustate()->query_user();
}

static void send_in(string str)
{
	query_ustate()->send_in(str);
}

static void send_out(string str)
{
	query_ustate()->send_out(str);
}

string *query_parse_methods();

string query_help_title()
{
	return "Default help";
}

string *query_help_contents()
{
	return ({ "This verb has no help content." });
}
