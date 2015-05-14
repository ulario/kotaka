/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2006, 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/privilege.h>

private object *archetypes;

private object first_instance;

private mapping prev_instance;
private mapping next_instance;

static void create()
{
	archetypes = ({ });
	prev_instance = ([ ]);
	next_instance = ([ ]);
}

nomask void clean_archetypes()
{
	archetypes -= ({ nil });
}

nomask int is_archetype_of(object test)
{
	int index;
	int sz;
	object *arch;

	arch = test->query_archetypes();
	sz = sizeof(arch);

	if (sizeof(arch & ({ this_object() }))) {
		return 1;
	}

	for(index = 0; index < sz; index++) {
		if (is_archetype_of(arch[index])) {
			return 1;
		}
	}

	return 0;
}

nomask object *query_archetypes()
{
	clean_archetypes();

	return archetypes[..];
}

atomic nomask void set_archetypes(object *new_archs)
{
	int sz;

	object *removed, *added, *arch;
	object *check, this;

	CHECKARG(sizeof(new_archs & ({ nil })) == 0, 1, "set_archetypes");

	arch = query_archetypes();

	removed = arch - new_archs;
	added = new_archs - arch;

	removed -= ({ nil });
	added -= ({ nil });

	for (sz = sizeof(added); --sz >= 0; ) {
		object arch;

		arch = added[sz];

		if (is_archetype_of(arch)) {
			error("Circular reference");
		}
	}

	archetypes = new_archs;

	this = this_object();

	for (sz = sizeof(removed); --sz >= 0; ) {
		removed[sz]->thing_remove_instance(this);
	}

	for (sz = sizeof(added); --sz >= 0; ) {
		added[sz]->thing_add_instance(this);
	}
}

atomic nomask void clear_archetypes()
{
	int sz;
	object this;

	this = this_object();

	for (sz = sizeof(archetypes -= ({ nil }) ); sz >= 0; ) {
		archetypes[sz]->thing_remove_instance(this);
	}

	archetypes = ({ });
}

atomic nomask void add_archetype(object new_arch)
{
	CHECKARG(new_arch, 1, "add_archetype");

	if (is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	clean_archetypes();

	archetypes += ({ new_arch });

	new_arch->thing_add_instance(this_object());
}

atomic nomask void add_archetype_at(object new_arch, int position)
{
	CHECKARG(new_arch, 1, "add_archetype_at");
	CHECKARG(position >= 0, 2, "add_archetype_at");
	CHECKARG(position <= sizeof(archetypes), 2, "add_archetype_at");

	if (is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	clean_archetypes();

	if (position == -1) {
		archetypes += ({ new_arch });
	} else {
		archetypes = archetypes[0 .. position - 1]
			+ ({ new_arch }) + archetypes[position ..];
	}

	new_arch->thing_add_instance(this_object());
}

atomic nomask void del_archetype(object old_arch)
{
	archetypes -= ({ old_arch });

	clean_archetypes();

	old_arch->thing_del_instance(this_object());
}

nomask object query_first_instance()
{
	return first_instance;
}

nomask object query_prev_instance(object archetype)
{
	return prev_instance[archetype];
}

nomask object query_next_instance(object archetype)
{
	return next_instance[archetype];
}

nomask mapping query_prev_instances()
{
	return prev_instance[..];
}

nomask mapping query_next_instances()
{
	return next_instance[..];
}

nomask void thing_add_instance(object instance)
{
	object this;
	object prev;

	ACCESS_CHECK(THING());

	this = this_object();

	if (!first_instance) {
		first_instance = instance;
		instance->thing_set_prev_instance(this, instance);
		instance->thing_set_next_instance(this, instance);
	} else {
		prev = first_instance->query_prev_instance(this);

		first_instance->thing_set_prev_instance(this, instance);
		instance->thing_set_next_instance(this, first_instance);

		instance->thing_set_prev_instance(this, prev);
		prev->thing_set_next_instance(this, instance);
	}
}

nomask void thing_remove_instance(object instance)
{
	object prev, next, this;

	ACCESS_CHECK(THING());

	this = this_object();
	prev = instance->query_prev_instance(this);
	next = instance->query_next_instance(this);
	ASSERT(prev);
	ASSERT(next);

	if (instance == next) {
		first_instance = nil;
		instance->thing_set_prev_instance(this, nil);
		instance->thing_set_next_instance(this, nil);
	} else {
		if (first_instance == instance) {
			first_instance = next;
		}
		prev->thing_set_next_instance(this, next);
		next->thing_set_prev_instance(this, prev);
	}
}

nomask void thing_set_prev_instance(object archetype, object instance)
{
	ACCESS_CHECK(THING());

	if (instance) {
		prev_instance[archetype] = instance;
	} else {
		prev_instance[archetype] = nil;
	}
}

nomask void thing_set_next_instance(object archetype, object instance)
{
	ACCESS_CHECK(THING());

	if (instance) {
		next_instance[archetype] = instance;
	} else {
		next_instance[archetype] = nil;
	}
}

static void thing_wipe_archetype()
{
	archetypes = ({ });
	first_instance = nil;
	prev_instance = ([ ]);
	next_instance = ([ ]);
}
