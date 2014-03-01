/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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

mapping aliases;

private void save();
private void restore();

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

private void save()
{
	string buf;

	buf = STRINGD->hybrid_sprint(aliases);

	SECRETD->remove_file("aliases-tmp");
	SECRETD->write_file("aliases-tmp", buf + "\n");
	SECRETD->remove_file("aliases");
	SECRETD->rename_file("aliases-tmp", "aliases");
}

private void restore()
{
	string buf;

	buf = SECRETD->read_file("aliases");

	if (buf) {
		aliases = PARSER_VALUE->parse(buf);
	}
}
