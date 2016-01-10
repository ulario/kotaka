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
#include <kernel/user.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

private mapping prev;
private mapping next;

object query_prev_link(string domain, mixed key)
{
	ACCESS_CHECK(SYSTEM());

	return SYSTEM_SUBD->query_tiered_map(prev, domain, key);
}

object query_next_link(string domain, mixed key)
{
	ACCESS_CHECK(SYSTEM());

	return SYSTEM_SUBD->query_tiered_map(next, domain, key);
}

void set_prev_link(string domain, mixed key, object obj)
{
	ACCESS_CHECK(SYSTEM());

	prev = SYSTEM_SUBD->set_tiered_map(prev, domain, key, obj);
}

void set_next_link(string domain, mixed key, object obj)
{
	ACCESS_CHECK(SYSTEM());

	next = SYSTEM_SUBD->set_tiered_map(next, domain, key, obj);
}

private void clear_list_domain(string domain)
{
	mixed *keys;
	int sz;

	keys = map_indices(prev[domain]);

	for (sz = sizeof(keys) - 1; sz >= 0; --sz) {
		LISTD->unlink(domain, keys[sz], this_object());
	}
}

void clear_list()
{
	ACCESS_CHECK(SYSTEM());

	if (prev) {
		int sz;
		string *domains;

		domains = map_indices(prev);

		for (sz = sizeof(domains) - 1; sz >= 0; --sz) {
			clear_list_domain(domains[sz]);
		}
	}
}
