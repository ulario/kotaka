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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit "/lib/string/sprint";

mapping aliases;

void save();
void restore();

static void create()
{
	aliases = ([ ]);

	restore();
}

string *query_aliases()
{
	return map_indices(aliases);
}

string query_alias(string name)
{
	return aliases[name];
}

void set_alias(string name, string value)
{
	ACCESS_CHECK(INTERFACE());

	aliases[name] = value;

	save();
}

void save()
{
	string buf;

	ACCESS_CHECK(INTERFACE() || VERB());

	buf = hybrid_sprint(aliases);

	CONFIGD->make_dir(".");
	CONFIGD->remove_file("aliases-tmp");
	CONFIGD->write_file("aliases-tmp", buf + "\n");
	CONFIGD->remove_file("aliases");
	CONFIGD->rename_file("aliases-tmp", "aliases");
}

void restore()
{
	string buf;

	ACCESS_CHECK(INTERFACE() || VERB());

	buf = CONFIGD->read_file("aliases");

	if (buf) {
		aliases = PARSER_VALUE->parse(buf);
	}
}
