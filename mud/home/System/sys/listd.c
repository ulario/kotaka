/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kernel/tls.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/utility.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

mapping links;		/* ([ domain: ([ key: object ]) ]) */

/* private functions */

static void create()
{
	links = ([ ]);
}

private void validate_key(mixed key)
{
	switch(typeof(key)) {
	case T_NIL:
	case T_MAPPING:
	case T_ARRAY:
		error("Invalid key type");

	case T_OBJECT:
		if (sscanf(object_name(key), "%*s#-1")) {
			error("Invalid key type");
		}
	}
}


/* list */

object first_link(string domain, mixed key)
{
	mapping kmap;
	string creator;

	validate_key(key);

	creator = DRIVER->creator(object_name(previous_object()));

	if (domain != creator) {
		error("Access denied");
	}

	return SUBD->query_tiered_map(links, domain, key);
}

object prev_link(string domain, mixed key, object obj)
{
	mapping kmap;
	string creator;

	validate_key(key);

	creator = DRIVER->creator(object_name(previous_object()));

	if (domain != creator) {
		error("Access denied");
	}

	return obj->query_prev_link(domain, key);
}

object next_link(string domain, mixed key, object obj)
{
	string creator;

	validate_key(key);

	creator = DRIVER->creator(object_name(previous_object()));

	if (domain != creator) {
		error("Access denied");
	}

	return obj->query_next_link(domain, key);
}

void link(string domain, mixed key, object obj)
{
	object first;

	first = SUBD->query_tiered_map(links, domain, key);

	validate_key(key);

	if (!SYSTEM()) {
		string creator;

		creator = DRIVER->creator(object_name(previous_object()));

		if (domain != creator) {
			error("Access denied");
		}
	}

	if (first) {
		object prev;

		prev = first->query_next_link(domain, key);

		prev->set_next_link(domain, key, obj);
		obj->set_prev_link(domain, key, prev);
		obj->set_next_link(domain, key, first);
		first->set_prev_link(domain, key, obj);
	} else {
		first = obj;

		obj->set_next_link(domain, key, obj);
		obj->set_prev_link(domain, key, obj);

		links = SUBD->set_tiered_map(links, domain, key, obj);
	}
}

void unlink(string domain, mixed key, object obj)
{
	object first;
	object prev;
	object next;

	validate_key(key);

	if (!SYSTEM()) {
		string creator;

		creator = DRIVER->creator(object_name(previous_object()));

		if (domain != creator) {
			error("Access denied");
		}
	}

	first = SUBD->query_tiered_map(links, domain, key);

	if (!first) {
		error("Object not in list");
	}

	prev = obj->query_prev_link(domain, key);
	next = obj->query_next_link(domain, key);

	if (prev == obj) {
		ASSERT(first == obj);

		obj->set_prev_link(domain, key, nil);
		obj->set_next_link(domain, key, nil);

		links = SUBD->set_tiered_map(links, domain, key, nil);
	} else {
		ASSERT(prev);
		ASSERT(next);

		prev->set_next_link(domain, key, next);
		next->set_prev_link(domain, key, prev);

		obj->set_prev_link(domain, key, nil);
		obj->set_next_link(domain, key, nil);

		if (first == obj) {
			links = SUBD->set_tiered_map(links, domain, key, next);
		}
	}
}
