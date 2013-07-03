/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/property.h>
#include <thing/paths.h>
#include <type.h>

object query_environment();
object *query_archetypes();

private mapping properties;
private string *removed_properties;

mixed query_property(string name);

static void create()
{
}

nomask void clean_properties()
{
	if (!map_sizeof(properties)) {
		properties = nil;
	}

	if (!removed_properties) {
		return;
	}

	if (!sizeof(removed_properties)) {
		removed_properties = nil;
	}
}

nomask string *list_local_properties()
{
	if (properties) {
		return map_indices(properties);
	} else {
		return ({ });
	}
}

nomask mapping query_local_properties()
{
	if (properties) {
		return SUBD->deep_copy(properties);
	} else {
		return ([ ]);
	}
}

nomask void set_local_properties(mapping prop)
{
	properties = SUBD->deep_copy(prop);

	clean_properties();
}

nomask void clear_local_properties()
{
	properties = nil;
}

nomask void set_removed_properties(string *remove)
{
	if (sizeof(remove & ({ nil }) )) {
		error("Invalid removed property");
	}

	if (sizeof(remove)) {
		removed_properties = remove[..];
	} else {
		removed_properties = nil;
	}
}

nomask string *query_removed_properties()
{
	if (removed_properties) {
		return removed_properties[..];
	} else {
		return ({ });
	}
}

nomask void clear_removed_properties()
{
	removed_properties = nil;
}

nomask void set_local_property(string name, mixed value, varargs int nosignal)
{
	if (!properties) {
		properties = ([ ]);
	}

	properties[name] = SUBD->deep_copy(value);

	clean_properties();
}

void set_property(string name, mixed value)
{
	mixed *info;
	mixed old;
	string basename;

	info = PROPERTYD->query_property(name);

	if (!info) {
		if (value != nil) {
			error("Undefined property \"" + name + "\"");
		}
	} else {
		if (info[0] != -1 && value != nil && info[0] !=
			typeof(value)) {
			error("Type mismatch, got " + typeof(value) +
				", expected " + info[0] + " on " + name);
		}
	}

	if (info) {
		switch(info[1]) {
		case PROP_SIMPLE:
			break;

		case PROP_COMBO:
		case PROP_MIXDOWN:
			error("Composite properties are read only");
		case PROP_MAGIC:
			{
				string func;
				string lib;
				string creator;

				func = info[2][1];	/* writer */
				lib = function_object(func, this_object());

				if (!lib) {
					error("No such function");
				}

				creator = DRIVER->creator(lib);

				call_other(this_object(), func, value);
			}
			return;
		}
	}

	set_local_property(name, value);
}

nomask mixed query_local_property(string name)
{
	if (properties) {
		return SUBD->deep_copy(properties[name]);
	} else {
		return nil;
	}
}

mixed query_property(string name)
{
	mixed *info;
	object propkey;
	string basename;

	int flags;

	info = PROPERTYD->query_property(name);

	if (!info) {
		error(name + ": Undefined property");
	}

	switch (info[1]) {
	case PROP_SIMPLE:
		{
			mixed value;

			if (properties) {
				value = properties[name];
			}

			if (value != nil) {
				return SUBD->deep_copy(value);
			}

			return info[2];
		}

	case PROP_INHERIT:
	case PROP_DROPDOWN:
		if (properties && properties[name] != nil)  {
			return SUBD->deep_copy(properties[name]);
		}

		if (removed_properties && sizeof(removed_properties & ({ name }) )) {
			return info[2];
		}

		{
			int index;
			mixed value;
			object *arch;

			if (info[1] == PROP_DROPDOWN) {
				arch = ({ query_environment() }) - ({ nil });
			} else {
				arch = query_archetypes();
			}

			for (index = 0; index < sizeof(arch); index++) {
				value = arch[index]->
					query_property(name);

				if (value != nil) {
					return value;
				}
			}

			return info[2];
		}

	case PROP_COMBO:
	case PROP_MIXDOWN:
		{
			int index;
			mixed local;
			mixed gather;

			mixed remove;

			string lname;
			string rname;

			string *extra;
			object *arch;

			extra = info[2];

			lname = extra[0];
			rname = extra[1];

			if (info[1] == PROP_MIXDOWN) {
				arch = ({ query_environment() }) - ({ nil });
			} else {
				arch = query_archetypes();
			}

			if (properties) {
				local = SUBD->deep_copy(properties[lname]);
			}

			switch(info[0]) {
			case T_ARRAY:
				gather = ({ });

				if (!local) {
					local = ({ });
				}

				break;

			case T_MAPPING:
				gather = ([ ]);

				if (!local) {
					local = ([ ]);
				}

				break;

			default:
				error("Illegal combo type");
			}

			for (index = 0; index < sizeof(arch);
				index++) {
				gather |=
					arch[index]->
					query_property(name);
			}

			if (rname) {
				remove = query_property(rname);
				gather -= remove;
			}

			if (typeof(gather) == T_MAPPING) {
				gather += local;
			} else {
				gather |= local;
			}

			return gather;
		}
		break;

	case PROP_MAGIC:
		{
			string func;
			string lib;
			string creator;

			func = info[2][0];	/* reader */
			lib = function_object(func, this_object());

			if (!lib) {
				error("No such function");
			}

			creator = DRIVER->creator(lib);

			return call_other(this_object(), func);
		}
	default:
		error("Invalid property code of " + info[1]);
	}
}
