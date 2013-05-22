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

static object query_ustate()
{
	return TLSD->query_tls_value("Text", "ustate");
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

nomask int query_raw()
{
	error("Deprecated function");
}

nomask mixed fetch_raw(mixed junk ...)
{
	error("Deprecated function");
}

nomask mixed fetch_evoke(mixed junk ...)
{
	error("Deprecated function");
}

static string generate_brief_definite(object thing)
{
	string brief;

	brief = thing->query_property("brief");

	if (brief) {
		return "the " + brief;
	}

	brief = thing->query_property("id");

	if (brief) {
		return "the " + brief;
	}
}

static string generate_brief_indefinite(object thing)
{
	string brief;

	brief = thing->query_property("brief");

	if (brief) {
		return "a " + brief;
	}

	brief = thing->query_property("id");

	if (brief) {
		return "a " + brief;
	}
}
