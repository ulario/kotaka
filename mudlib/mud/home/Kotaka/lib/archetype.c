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
private object *archetypes;

/**********************/
/* Archetype handling */
/**********************/

static void initialize()
{
	if (!archetypes) {
		archetypes = ({ });
	}
}

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
