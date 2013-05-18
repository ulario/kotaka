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

int query_raw()
{
	return 1;
}

void main(mixed *tree)
{
}

static string fetch_raw(mixed *tree)
{
	mixed *iclause;
	mixed *raw;

	if (tree[0] != "S") {
		error("Expected: statement");
	}

	iclause = tree[1];

	if (iclause[0] != "V") {
		error("Expected: verb");
	}

	raw = iclause[2];

	if (raw[0] != "R") {
		error("Expected: raw role");
	}

	return raw[1];
}
