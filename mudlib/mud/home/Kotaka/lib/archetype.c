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
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

private object *archetypes;

nomask static void initialize_archetype()
{
	ACCESS_CHECK(previous_program() == LIB_OBJECT);

	if (!archetypes) {
		archetypes = ({ });
	}
}

nomask int kotaka_is_archetype_of(object test)
{
	int index;
	int sz;
	object *arch;

	ACCESS_CHECK(KOTAKA());

	arch = test->query_archetypes();
	sz = sizeof(arch);

	if (sizeof(arch & ({ this_object() }))) {
		return 1;
	}

	for(index = 0; index < sz; index++) {
		if (kotaka_is_archetype_of(arch[index])) {
			return 1;
		}
	}

	return 0;
}

nomask object *kotaka_query_archetypes()
{
	ACCESS_CHECK(KOTAKA());

	archetypes -= ({ nil });

	return archetypes[..];
}

nomask void kotaka_set_archetypes(object *new_archs)
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
		if (kotaka_is_archetype_of(check[i])) {
			error("Circular reference");
		}
	}

	archetypes = new_archs;
}

nomask void kotaka_clear_archetypes()
{
	ACCESS_CHECK(KOTAKA());

	archetypes = ({ });
}

nomask void kotaka_add_archetype(object new_arch)
{
	CHECKARG(new_arch, 1, "add_archetype");
	CHECKARG(new_arch <- LIB_OBJECT, 1, "add_archetype");

	ACCESS_CHECK(KOTAKA());

	if (kotaka_is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	archetypes -= ({ nil });
	archetypes += ({ new_arch });
}

nomask void kotaka_add_archetype_at(object new_arch, int position)
{
	CHECKARG(new_arch, 1, "add_archetype_at");
	CHECKARG(new_arch <- LIB_OBJECT, 1, "add_archetype_at");

	CHECKARG(position >= -1, 2, "add_archetype_at");
	CHECKARG(position <= sizeof(archetypes), 2, "add_archetype_at");

	ACCESS_CHECK(KOTAKA());

	archetypes -= ({ nil });

	if (position == -1) {
		archetypes += ({ new_arch });
	} else {
		archetypes = archetypes[0 .. position - 1]
			+ ({ new_arch }) + archetypes[position ..];
	}
}

nomask void kotaka_del_archetype(object old_arch)
{
	ACCESS_CHECK(KOTAKA());

	archetypes -= ({ nil, old_arch });
}

/* untrusted */

int is_archetype_of(object test)
{
	return kotaka_is_archetype_of(test);
}

object *query_archetypes()
{
	archetypes -= ({ nil });

	return archetypes[..];
}

void set_archetypes(object *new_archs)
{
	int i;
	int sz;

	object *old_archs;
	object *check;

	archetypes -= ({ nil });
	new_archs = new_archs -= ({ nil });
	check = new_archs - ({ archetypes });

	sz = sizeof(check);

	for (i = 0; i < sz; i++) {
		if (!check[i] <- LIB_OBJECT) {
			error("Bad argument 1 for function set_archetypes (found non LIB_OBJECT)");
		}

		if (kotaka_is_archetype_of(check[i])) {
			error("Circular reference");
		}
	}

	archetypes = new_archs;
}

void clear_archetypes()
{
	archetypes = ({ });
}

void add_archetype(object new_arch)
{
	CHECKARG(new_arch, 1, "add_archetype");
	CHECKARG(new_arch <- LIB_OBJECT, 1, "add_archetype");

	if (kotaka_is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	archetypes -= ({ nil });
	archetypes += ({ new_arch });
}

void add_archetype_at(object new_arch, int position)
{
	CHECKARG(new_arch, 1, "add_archetype_at");
	CHECKARG(new_arch <- LIB_OBJECT, 1, "add_archetype_at");

	CHECKARG(position >= -1, 2, "add_archetype_at");
	CHECKARG(position <= sizeof(archetypes), 2, "add_archetype_at");

	if (kotaka_is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	archetypes -= ({ nil });

	if (position == -1) {
		archetypes += ({ new_arch });
	} else {
		archetypes = archetypes[0 .. position - 1]
			+ ({ new_arch }) + archetypes[position ..];
	}
}

void del_archetype(object old_arch)
{
	archetypes -= ({ nil, old_arch });
}
