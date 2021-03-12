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

void reset_properties();

static void create()
{
	reset_properties();
}

void set_property(string name, int type, int code, varargs mixed extra)
{
	ACCESS_CHECK(THING() || GAME());

	if (sscanf(name, "base:")) {
		error("Reserved property");
	}

	if ((code == PROP_COMBO || code == PROP_MIXDOWN)
		&& type < T_ARRAY) {
		error("Combo properties must be of conglomerate type");
	}

	pinfo[name] = ({ type, code, extra });
}

void add_property(string name, int type, int code, varargs mixed extra)
{
	ACCESS_CHECK(THING() || GAME());

	if (sscanf(name, "base:")) {
		error("Reserved property");
	}

	if (pinfo[name]) {
		error("Duplicate property");
	}

	if ((code == PROP_COMBO || code == PROP_MIXDOWN)
		&& type < T_ARRAY) {
		error("Combo properties must be of conglomerate type");
	}

	pinfo[name] = ({ type, code, extra });
}

void clear_property(string name)
{
	ACCESS_CHECK(THING() || GAME());

	pinfo[name] = nil;
}

void remove_property(string name)
{
	ACCESS_CHECK(THING() || GAME());

	if (!pinfo[name]) {
		error("No such property");
	}

	pinfo[name] = nil;
}

/* stub */
void del_property(string name)
{
	remove_property(name);
}

atomic void reset_properties()
{
	ACCESS_CHECK(THING() || GAME());

	pinfo = ([ ]);

	/* identification */
	set_property("id", T_STRING, PROP_MAGIC, ({ "query_id", "set_id" }) );
	set_property("name", T_STRING, PROP_MAGIC, ({ "query_object_name", "set_object_name" }) );

	/* inheritance */
	set_property("archetype", T_OBJECT, PROP_MAGIC, ({ "query_archetype", "set_archetype" }) );

	/* inventory */
	set_property("environment", T_OBJECT, PROP_MAGIC, ({ "query_environment", "move" }) );
	set_property("inventory", T_ARRAY, PROP_MAGIC, ({ "query_inventory", nil }) );

	/* bulk */
	set_property("mass", T_FLOAT, PROP_MAGIC, ({ "query_mass", "set_mass" }) );
	set_property("density", T_FLOAT, PROP_MAGIC, ({ "query_density", "set_density" }) );
	set_property("capacity", T_FLOAT, PROP_MAGIC, ({ "query_capacity", "set_capacity" }) );
	set_property("max_mass", T_FLOAT, PROP_MAGIC, ({ "query_max_mass", "set_max_mass" }) );

	set_property("local_mass", T_FLOAT, PROP_MAGIC, ({ "query_local_mass", "set_local_mass" }) );
	set_property("local_density", T_FLOAT, PROP_MAGIC, ({ "query_local_density", "set_local_density" }) );
	set_property("local_capacity", T_FLOAT, PROP_MAGIC, ({ "query_local_capacity", "set_local_capacity" }) );
	set_property("local_max_mass", T_FLOAT, PROP_MAGIC, ({ "query_local_max_mass", "set_local_max_mass" }) );

	set_property("absolute", T_INT, PROP_MAGIC, ({ "query_absolute", "set_absolute" }) );

	set_property("is_flexible", T_INT, PROP_MAGIC, ({ "query_flexible", "set_flexible" }) );
	set_property("is_virtual", T_INT, PROP_MAGIC, ({ "query_virtual", "set_virtual" }) );

	set_property("volume", T_FLOAT, PROP_MAGIC, ({ "query_volume", nil }) );
	set_property("total_volume", T_FLOAT, PROP_MAGIC, ({ "query_total_volume", nil }) );
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
