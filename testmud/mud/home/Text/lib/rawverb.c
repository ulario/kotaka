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
#include <text/paths.h>
#include <text/parse.h>

inherit LIB_VERB;

nomask int query_parse_method()
{
	return PARSE_RAW;
}

nomask void do_action(mapping roles)
{
	error("This function does not belong in a raw verb");
}

nomask mixed **query_roles()
{
	error("This function does not belong in a raw verb");
}

void main(object actor, string args);
