/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/log.h>
#include <status.h>
#include <kernel/access.h>

private void check_node(object node)
{
	if (!node->query_root()) {
		LOGD->post_message("bigstruct", LOG_WARNING, "Found orphaned bigstruct node: " + object_name(node));
		node->self_destruct();
	}
}

private void find_orphaned_nodes_of(string program)
{
	/* a node is orphaned if its root object is missing */
	int i;
	int sz;

	sz = status(ST_OTABSIZE);

	for (i = 0; i < sz; i++) {
		object obj;

		obj = find_object(program + "#" + i);

		if (obj) {
			check_node(obj);
		}
	}
}

void find_orphaned_nodes()
{
	find_orphaned_nodes_of(USR_DIR + "/Bigstruct/obj/array/node");
	find_orphaned_nodes_of(USR_DIR + "/Bigstruct/obj/deque/node");
	find_orphaned_nodes_of(USR_DIR + "/Bigstruct/obj/map/node");
}

private void check_root(object root)
{
	mapping grants;
	int *access;

	grants = root->query_grants();

	access = map_values(grants);

	if (!sizeof(access & ({ FULL_ACCESS }))) {
		LOGD->post_message("bigstruct", LOG_WARNING, "Found orphaned bigstruct root: " + object_name(root));
		root->self_destruct();
	}
}

private void find_orphaned_roots_of(string program)
{
	/* a root is orphaned if no object has full access */
	int i;
	int sz;

	sz = status(ST_OTABSIZE);

	for (i = 0; i < sz; i++) {
		object obj;

		obj = find_object(program + "#" + i);

		if (obj) {
			check_root(obj);
		}
	}
}

void find_orphaned_roots()
{
	find_orphaned_roots_of(USR_DIR + "/Bigstruct/obj/array/root");
	find_orphaned_roots_of(USR_DIR + "/Bigstruct/obj/deque/root");
	find_orphaned_roots_of(USR_DIR + "/Bigstruct/obj/map/root");
}
