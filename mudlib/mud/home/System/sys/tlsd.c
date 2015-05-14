/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2008, 2010, 2012, 2013, 2014  Raymond Jennings
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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kernel/tls.h>
#include <kotaka/paths/system.h>

inherit SECOND_AUTO;

private inherit API_TLS;

mapping registry;	/*< ([ domain: ([ key: ([ user: access ]) ]) ]) */

static void create()
{
	::create();
	::set_tls_size(1);

	registry = ([ ]);
}

int query_tls_access(string domain, string key, string user)
{
	int access;
	string creator;
	mapping dmap;
	mapping kmap;

	if (user == "System") {
		return FULL_ACCESS;
	}

	if (domain == user) {
		return FULL_ACCESS;
	}

	dmap = registry[domain];

	if (!dmap) {
		return 0;
	}

	kmap = dmap[key];

	if (!kmap) {
		return 0;
	}

	if (!kmap[user]) {
		return 0;
	}

	return kmap[user];
}

mixed set_tls_access(string domain, string key, string user, int access)
{
	string creator;
	mapping dmap;
	mapping kmap;

	creator = DRIVER->creator(previous_program());

	if (query_tls_access(domain, key, creator) != FULL_ACCESS) {
		error("Insufficient access granting privileges");
	}

	if (domain == user) {
		error("Self access is fixed at full");
	}

	if (access == FULL_ACCESS && creator != "System") {
		error("Insufficient access granting privileges");
	}

	dmap = registry[domain];

	if (!dmap) {
		dmap = ([ ]);
	}

	kmap = dmap[key];

	if (!kmap) {
		kmap = ([ ]);
	}

	kmap[user] = access ? access : nil;

	if (!map_sizeof(kmap)) {
		kmap = nil;
	}

	dmap[key] = kmap;

	if (!map_sizeof(dmap)) {
		dmap = nil;
	}

	registry[domain] = dmap;
}

mixed query_tls_value(string domain, string key)
{
	string creator;
	mapping heap;
	mapping dmap;

	creator = DRIVER->creator(previous_program());

	if (query_tls_access(domain, key, creator) < READ_ACCESS) {
		error("Access denied");
	}

	heap = get_tlvar(0);

	if (!heap) {
		return nil;
	}

	dmap = heap[domain];

	if (!dmap) {
		return nil;
	}

	return dmap[key];
}

void set_tls_value(string domain, string key, mixed value)
{
	string creator;
	mapping heap;
	mapping dmap;

	creator = DRIVER->creator(previous_program());

	if (query_tls_access(domain, key, creator) < WRITE_ACCESS) {
		error("Access denied");
	}

	heap = get_tlvar(0);

	if (!heap) {
		if (value == nil) {
			return;
		} else {
			heap = ([ ]);
		}
	}

	dmap = heap[domain];

	if (!dmap) {
		if (value == nil) {
			return;
		} else {
			dmap = ([ ]);
		}
	}

	dmap[key] = value;

	if (map_sizeof(dmap) == 0) {
		dmap = nil;
	}

	heap[domain] = dmap;

	if (map_sizeof(heap) == 0) {
		heap = nil;
	}

	set_tlvar(0, heap);
}
