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
#include <type.h>

#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/property.h>
#include <kotaka/log.h>

/*************/
/* Variables */
/*************/

private object *archetypes;

private object environment;
private object *inventory;

private mapping properties;
private string *removed_properties;

/*****************/
/* General stuff */
/*****************/

private void initialize()
{
	if (!archetypes) {
		archetypes = ({ });
	}

	if (!inventory) {
		inventory = ({ });
	}

	if (!properties) {
		properties = ([ ]);
		removed_properties = ({ });
	}
}

static void create()
{
	initialize();
}

nomask void kotaka_object_constructor()
{
	ACCESS_CHECK(previous_program() == SECOND_AUTO);

	initialize();
}

nomask void kotaka_object_destructor()
{
	ACCESS_CHECK(previous_program() == SECOND_AUTO);
}

/**********************/
/* Archetype handling */
/**********************/

nomask int _F_is_archetype_of(object test)
{
	int index;
	int sz;
	object *arch;

	ACCESS_CHECK(KOTAKA());

	arch = test->_F_query_archetypes();
	sz = sizeof(arch);

	if (sizeof(arch & ({ this_object() }))) {
		return 1;
	}

	for(index = 0; index < sz; index++) {
		if (_F_is_archetype_of(arch[index])) {
			return 1;
		}
	}

	return 0;
}

nomask object *_F_query_archetypes()
{
	ACCESS_CHECK(KOTAKA());

	return archetypes -= ({ nil });
}

nomask void _F_set_archetypes(object *new_archs)
{
	int i;
	int sz;

	object *old_archs;
	object *check;

	ACCESS_CHECK(KOTAKA());

	archetypes -= ({ nil });
	new_archs = new_archs -= ({ nil });
	check = new_archs - ({ archetypes });

	sz = sizeof(check);

	for (i = 0; i < sz; i++) {
		if (!check[i] <- LIB_OBJECT) {
			error("Bad argument 1 for function set_archetypes (found non LIB_OBJECT)");
		}
		if (_F_is_archetype_of(check[i])) {
			error("Circular reference");
		}
	}

	archetypes = new_archs;
}

nomask void _F_clear_archetypes()
{
	ACCESS_CHECK(KOTAKA());
	
	archetypes = ({ });
}

nomask void _F_add_archetype(object new_arch)
{
	ACCESS_CHECK(KOTAKA());

	CHECKARG(new_arch, 1, "add_archetype");
	CHECKARG(new_arch <- LIB_OBJECT, 1, "add_archetype");

	if (_F_is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	archetypes -= ({ nil });
	archetypes += ({ new_arch });
}

nomask void _F_add_archetype_at(object new_arch, int position)
{
	ACCESS_CHECK(KOTAKA());

	CHECKARG(new_arch, 1, "add_archetype_at");
	CHECKARG(new_arch <- LIB_OBJECT, 1, "add_archetype_at");

	CHECKARG(position >= -1, 2, "add_archetype_at");
	CHECKARG(position <= sizeof(archetypes), 2, "add_archetype_at");

	archetypes -= ({ nil });
	
	if (position == -1) {
		archetypes += ({ new_arch });
	} else {
		archetypes = archetypes[0 .. position - 1]
			+ ({ new_arch }) + archetypes[position ..];
	}
}

nomask void _F_del_archetype(object old_arch)
{
	ACCESS_CHECK(KOTAKA());

	archetypes -= ({ nil, old_arch });
}

/* high */

void add_archetype(object new_arch)
{
	_F_add_archetype(new_arch);
}

void add_archetype_at(object new_arch, int position)
{
	_F_add_archetype_at(new_arch, position);
}

int is_archetype_of(object test)
{
	CHECKARG(test, 1, "is_archetype_of");
	CHECKARG(test <- LIB_OBJECT, 1, "is_archetype_of");

	return _F_is_archetype_of(test);
}

void del_archetype(object old_arch)
{
	_F_del_archetype(old_arch);
}

void set_archetypes(object *new_archs)
{
	_F_set_archetypes(new_archs);
}

object *query_archetypes()
{
	return archetypes -= ({ nil });
}

void clear_archetypes()
{
	_F_clear_archetypes();
}

/**********************/
/* Inventory handling */
/**********************/

int forbid_move(object new_env)
{
	return 0;
}

int forbid_insert(object obj)
{
	return 0;
}

int forbid_remove(object obj)
{
	return 0;
}

void move_notify(object old_env)
{
}

void insert_notify(object obj)
{
}

void remove_notify(object obj)
{
}

nomask object _F_query_environment()
{
	ACCESS_CHECK(KOTAKA());

	return environment;
}

nomask object *_F_query_inventory()
{
	ACCESS_CHECK(KOTAKA());

	return inventory - ({ nil });
}

nomask void _F_add_inventory(object arriving)
{
	ACCESS_CHECK(KOTAKA());
	ASSERT(arriving);
	ASSERT(!sizeof( ({ arriving }) & inventory ));

	inventory = ({ arriving }) + (inventory - ({ nil }));
}

nomask void _F_del_inventory(object departing)
{
	ACCESS_CHECK(KOTAKA());
	ASSERT(departing);

	inventory -= ({ departing });
}

nomask int _F_is_container_of(object test)
{
	object env;
	object this;

	ACCESS_CHECK(KOTAKA());

	this = this_object();
	env = test->_F_query_environment();

	while (env) {
		if (env == this) {
			return 1;
		}

		env = env->_F_query_environment();
	}

	return 0;
}

atomic nomask void _F_move(object new_env)
{
	object this;
	string base;
	int new_number;

	ACCESS_CHECK(KOTAKA());

	CHECKARG(!new_env || new_env <- LIB_OBJECT, 1, "move");

	this = this_object();

	if (new_env) {
		if (_F_is_container_of(new_env)) {
			error("Recursive containment");
		}
	}

	if (new_env == environment) {
		return;
	}

	if (environment) {
		environment->_F_del_inventory(this_object());
	}
	environment = new_env;
	if (environment) {
		environment->_F_add_inventory(this_object());
	}
}

/* high */

void move(object new_env)
{
	object old_env;
	object this;

	old_env = environment;

	this = this_object();

	PERMISSION_CHECK(!old_env || !old_env->forbid_remove(this));
	PERMISSION_CHECK(!forbid_move(new_env));
	PERMISSION_CHECK(!new_env || !new_env->forbid_insert(this));

	_F_move(new_env);

	if (old_env) {
		old_env->remove_notify(this);
	}
	move_notify(old_env);
	if (new_env) {
		new_env->insert_notify(this);
	}
}

int is_container_of(object test)
{
	CHECKARG(test, 1, "is_container_of");
	CHECKARG(test <- LIB_OBJECT, 1, "is_container_of");

	return _F_is_container_of(test);
}

object query_environment()
{
	return environment;
}

object *query_inventory()
{
	return _F_query_inventory();
}

/*********************/
/* Property handling */
/*********************/

mixed query_property(string pname);

/* low */

nomask mapping _F_query_local_properties()
{
	ACCESS_CHECK(KOTAKA());

	return SUBD->deep_copy(properties);
}

nomask void _F_set_local_properties(mapping prop)
{
	ACCESS_CHECK(KOTAKA());

	properties = SUBD->deep_copy(prop);
}

nomask void _F_clear_local_properties()
{
	mapping old_props;

	ACCESS_CHECK(KOTAKA());

	properties = ([ ]);
}

nomask void _F_set_removed_properties(string *remove)
{
	ACCESS_CHECK(KOTAKA());

	removed_properties = remove - ({ nil });
}

nomask string *_F_query_removed_properties()
{
	ACCESS_CHECK(KOTAKA());

	return removed_properties[..];
}

nomask void _F_clear_removed_properties()
{
	ACCESS_CHECK(KOTAKA());

	removed_properties = ({ });
}

nomask void _F_set_local_property(string name, mixed value, varargs int nosignal)
{
	properties[name] = SUBD->deep_copy(value);
}

nomask void _F_set_property(string name, mixed value)
{
	mixed *info;
	mixed old;
	string basename;

	ACCESS_CHECK(KOTAKA());

	info = PROPERTYD->query_property(name);

	if (!info) {
		if (value != nil) {
			error("Undefined property \"" + name + "\"");
		}
	} else {
		if (info[0] != -1 && value != nil && info[0] !=
			typeof(value)) {
			error("Type mismatch, " + typeof(value) +
				" versus " + info[0] + " on " + name);
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

				ACCESS_CHECK(creator == "Game");

				call_other(this_object(), func, value);
			}
			return;
		}
	}

	_F_set_local_property(name, value);
}

nomask mixed _F_query_property(string name)
{
	mixed *info;
	object propkey;
	string basename;

	int flags;

	ACCESS_CHECK(KOTAKA());

	info = PROPERTYD->query_property(name);

	if (!info) {
		error(name + ": Undefined property");
	}

	switch (info[1]) {
	case PROP_SIMPLE:
		{
			mixed value;
			
			value = properties[name];
			
			if (value != nil) {
				return SUBD->deep_copy(value);
			}

			switch(info[0]) {
			case T_INT:
				return 0;

			case T_FLOAT:
				return 0.0;
			
			case T_ARRAY:
				return ({ });

			case T_MAPPING:
				return ([ ]);

			case T_STRING:
				return "";

			default:
				return nil;
			}
		}

	case PROP_INHERIT:
	case PROP_DROPDOWN:
		if (properties[name] != nil)  {
			return SUBD->deep_copy(properties[name]);
		}
		if (sizeof(removed_properties & ({ name }) )) {
			return nil;
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
			
			switch(info[0]) {
			case T_INT:
				return 0;
			
			case T_FLOAT:
				return 0.0;
			
			default:
				return nil;
			}
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

			local = SUBD->deep_copy(properties[lname]);

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
				
				if (!remove) {
					remove = ({ });
				}
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

			ACCESS_CHECK(creator == "Game");

			return call_other(this_object(), func);
		}
	default:
		error("Invalid property code of " + info[1]);
	}
}

nomask void _F_setprop_pre(string propname, mixed old, mixed new)
{
/*	_F_emit_signal("setprop:" + propname, old, new);*/
}

nomask void _F_setprop_post(string propname, mixed old, mixed new)
{
/*	_F_emit_signal("setprop-post:" + propname, old, new);*/
}

/* high */

void set_local_property(string name, mixed value)
{
	_F_set_local_property(name, value);
}

void set_property(string name, mixed value)
{
	_F_set_property(name, value);
}

mixed query_property(string name)
{
	return _F_query_property(name);
}

mixed query_local_property(string name)
{
	return SUBD->deep_copy(properties[name]);
}

string *list_local_properties()
{
	return map_indices(properties);
}

mapping query_local_properties()
{
	return SUBD->deep_copy(properties);
}

void set_local_properties(mapping prop)
{
	CHECKARG(prop, 1, "set_local_properties");

	_F_set_local_properties(prop);
}

void clear_local_properties()
{
	_F_clear_local_properties();
}

void set_removed_properties(string *remove)
{
	CHECKARG(remove, 1, "set_removed_properties");

	_F_set_removed_properties(remove);
}

string *query_removed_properties()
{
	return removed_properties[..];
}

void clear_removed_properties()
{
	_F_clear_removed_properties();
}
