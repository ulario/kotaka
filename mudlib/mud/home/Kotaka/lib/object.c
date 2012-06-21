#include <type.h>

#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/property.h>

#define ID(x, y) (((y) > 1) ? ((x) + "#" + (y)) : (x))

/**********************/
/* Archetype handling */
/**********************/

private object *archetypes;

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

/*********************/
/* Identity handling */
/*********************/

object *_F_query_inventory();
object _F_query_environment();

private string id_base;
private int id_number;

static void validate_base_id(string new_base)
{
	if (!STRINGD->is_valid_base_id(new_base)) {
		error("Bad base ID");
	}

	switch(new_base) {
	case "ENV":
	case "THIS":
		error("Reserved base ID");
	default:
		break;
	}
}

nomask string _F_query_id()
{
	ACCESS_CHECK(KOTAKA());

	return ID(id_base, id_number);
}

nomask string _F_query_id_base()
{
	ACCESS_CHECK(KOTAKA());

	return id_base;
}

nomask int _F_query_id_number()
{
	ACCESS_CHECK(KOTAKA());
	
	return id_number;
}

nomask int _F_query_lowest_free(string trial, object exclude)
{
	mapping taken;
	object *inv;
	int i;
	int sz;

	ACCESS_CHECK(KOTAKA());

	taken = ([ ]);

	inv = map_values(inventory) - ({ exclude });

	for (i = 0; i < sizeof(inv); i++) {
		object check;

		check = inv[i];
		
		if (check->_F_query_id_base() != trial) {
			continue;
		}
		
		taken[check->_F_query_id_number()] = 1;
	}

	i = 1;

	while(taken[i]) {
		i++;
	}

	return i;
}

nomask object _F_find_by_id(string id)
{
	string base;
	int number;
	int index;
	object *inv;

	ACCESS_CHECK(KOTAKA());

	if (!sscanf(id, "%s#%d", base, number)) {
		base = id;
		number = 1;
	}

	ASSERT(number > 0);

	if (base == "ENV") {
		object env;

		env = this_object();

		for (index = 0; index < number; index++) {
			if (!env) {
				return nil;
			}

			env = env->_F_query_environment();
		}

		return env;
	}

	if (base == "THIS") {
		if (number != 1) {
			error("Bad ID");
		}

		return this_object();
	}

	inv = map_values(inventory);

	for(index = 0; index < sizeof(inv); index++) {
		if (inv[index]->_F_query_id() == id) {
			return inv[index];
		}
	}

	return nil;
}

nomask string _F_query_path(object root)
{
	string *parts;
	object cc;
	object cursor;
	int absolute;
	int upcount;

	ACCESS_CHECK(KOTAKA());

	parts = ({ });

	cc = SUBD->query_common_container(this_object(), root);

	if (!cc) {
		return nil;
	}

	if (cc == this_object()) {
		return "THIS";
	}

	cursor = this_object();

	while (cursor != cc) {
		parts = ({ cursor->_F_query_id() }) + parts;
		cursor = cursor->_F_query_environment();
	}

	cursor = root;

	while (cursor != cc) {
		upcount++;
		cursor = cursor->_F_query_environment();
	}

	if (upcount) {
		if (upcount > 1) {
			parts = ({ "ENV#" + upcount }) + parts;
		} else {
			parts = ({ "ENV" }) + parts;
		}
	}

	return implode(parts, ":");
}

nomask object _F_locate_object(string path)
{
	string *parts;
	object step;
	int index;

	ACCESS_CHECK(KOTAKA());

	parts = explode(path, ":") - ({ "" });

	step = this_object();

	for(index = 0; index < sizeof(parts); index++) {
		string part;

		if (!step) {
			return nil;
		}

		part = parts[index];

		step = step->_F_find_by_id(part);
	}

	return step;
}

nomask void _F_set_id(string new_id)
{
	object env;
	string new_base;
	int new_number;
	string old_id;

	ACCESS_CHECK(KOTAKA());

	if (!sscanf(new_id, "%s#%d", new_base, new_number)) {
		new_base = new_id;
		new_number = 1;
	}

	validate_base_id(new_base);

	new_id = ID(new_base, new_number);

	if (env = environment) {
		object test;

		test = env->_F_find_by_id(new_id);

		if (test && test != this_object()) {
			error("Duplicate ID");
		}
	}

	old_id = ID(base, number);

	base = new_base;
	number = new_number;
}

nomask void _F_set_id_base(string new_base)
{
	object env;
	int new_number;
	string old_id;
	string new_id;
	
	ACCESS_CHECK(KOTAKA());
	
	validate_base_id(new_base);
	
	env = environment;

	if (env) {
		new_number = env->_F_query_lowest_free(
			new_base, this_object()
		);
	} else {
		new_number = 1;
	}
	
	base = new_base;
	number = new_number;
}

nomask void _F_set_id_number(int new_number)
{
	object env;
	object other;
	
	ACCESS_CHECK(KOTAKA());

	env = environment;

	if (env) {
		other = env->_F_find_by_id(ID(base, new_number));

		if (other && other != this_object()) {
			error("Duplicate ID");
		}
	}

	number = new_number;
}

/* high */

string query_id()
{
	return ID(base, number);
}

string query_id_base()
{
	return base;
}

int query_id_number()
{
	return number;
}

void set_id(string new_id)
{
	_F_set_id(new_id);
}

void set_id_base(string new_base)
{
	_F_set_id_base(new_base);
}

void set_id_number(int new_number)
{
	_F_set_id_number(new_number);
}

object *filter_by_base(string str)
{
	int index;
	object *inv;

	inv = map_values(inventory);

	for(index = 0; index < sizeof(inv); index++) {
		if (inv[index]->_F_query_id_base() != str)
			inv[index] = nil;
	}
	
	return inv - ({ nil });
}

object find_by_id(string str)
{
	return _F_find_by_id(str);
}

string query_path(object root)
{
	return _F_query_path(root);
}

object locate_object(string path)
{
	return _F_locate_object(path);
}

/**********************/
/* Inventory handling */
/**********************/

private object environment;
private mapping inventory;

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

	return map_indices(inventory);
}

nomask void _F_add_inventory(object arriving)
{
	ACCESS_CHECK(KOTAKA());

	inventory[arriving] = 1;
}

nomask void _F_del_inventory(object departing)
{
	ACCESS_CHECK(KOTAKA());

	inventory[departing] = nil;
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
		if (query_owner() != new_env->query_owner()) {
			error("Owner mismatch");
		}

		if (_F_is_container_of(new_env)) {
			error("Cyclic containment attempted");
		}
	}

	if (new_env == environment) {
		return;
	}

	if (new_env) {
		new_number = new_env->_F_query_lowest_free(id_base, nil);
	} else {
		new_number = 1;
	}

	if (environment) {
		environment->_F_del_inventory(this_object());
	}
	environment = new_env;
	_F_set_id_number(new_number);
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
	return map_indices(inventory);
}

/*********************/
/* Property handling */
/*********************/

mixed query_property(string pname);

private mapping properties;
private string *removed_properties;

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
			switch(info[0]) {
			case T_INT:
				if (value == 0) {
					value = nil;
				}
				break;

			case T_FLOAT:
				if (value == 0.0) {
					value = nil;
				}
				break;
			}
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
			
			if (value) {
				return SUBD->deep_copy(value);
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

/*****************/
/* General stuff */
/*****************/

private void initialize()
{
	if (!archetypes) {
		archetypes = ({ });
	}

	if (!base) {
		base = "anonymous";
		number = 1;
	}

	if (!inventory) {
		inventory = ([ ]);
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
