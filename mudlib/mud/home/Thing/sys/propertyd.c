/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2008, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/thing.h>
#include <kotaka/privilege.h>
#include <kotaka/property.h>
#include <type.h>

private mapping pinfo;
/* ({ type, code, extra }) */

/*
For combo/mixdown properties, extra is ({ local, remove })
For magic properties, extra is ({ reader, writer })
For other inheritance codes, extra is the default value
*/

static void create()
{
	pinfo = ([ ]);
}

void add_property(string name, int type, int code, varargs mixed extra)
{
	ACCESS_CHECK(PRIVILEGED());

	if (sscanf(name, "base:")) {
		error("Reserved property");
	}

	if (pinfo[name]) {
		error(name + ": duplicate property");
	}

	if ((code == PROP_COMBO || code == PROP_MIXDOWN)
		&& type < T_ARRAY) {
		error("Combo properties must be of conglomerate type");
	}

	pinfo[name] = ({ type, code, extra });
}

void del_property(string name)
{
	ACCESS_CHECK(PRIVILEGED());

	if (pinfo[name]) {
		pinfo[name] = nil;
	} else {
		error(name + ": no such property");
	}
}

void clear_properties()
{
	ACCESS_CHECK(PRIVILEGED());

	pinfo = ([ ]);
}

mixed *query_property(string name)
{
	if (!pinfo[name]) {
		return nil;
	}

	return pinfo[name][..];
}

string *list_properties()
{
	return map_indices(pinfo);
}
