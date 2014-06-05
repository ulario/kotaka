/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2014  Raymond Jennings
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
#include <config.h>
#include <kernel/access.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <status.h>

private void check_node(object node)
{
	if (!node->query_root()) {
		LOGD->post_message("bigstruct", LOG_WARNING, "Found orphaned bigstruct node: " + object_name(node));
		node->self_destruct();
	}
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

void root_check_tick(int sz)
{
	int quota;

	for (; quota < 100 && sz >= 0; --sz, quota++) {
		object obj;

		obj = find_object(USR_DIR + "/Bigstruct/obj/array/root#" + sz);

		if (obj) {
			check_root(obj);
		}

		obj = find_object(USR_DIR + "/Bigstruct/obj/deque/root#" + sz);

		if (obj) {
			check_root(obj);
		}

		obj = find_object(USR_DIR + "/Bigstruct/obj/map/root#" + sz);

		if (obj) {
			check_root(obj);
		}
	}

	if (sz >= 0) {
		call_out("root_check_tick", 0, sz);
	} else {
		call_out("node_check_tick", 0, status(ST_OTABSIZE) - 1);
	}
}

void node_check_tick(int sz)
{
	int quota;

	for (; quota < 100 && sz >= 0; --sz, quota++) {
		object obj;

		obj = find_object(USR_DIR + "/Bigstruct/obj/array/node#" + sz);

		if (obj) {
			check_node(obj);
		}

		obj = find_object(USR_DIR + "/Bigstruct/obj/deque/node#" + sz);

		if (obj) {
			check_node(obj);
		}

		obj = find_object(USR_DIR + "/Bigstruct/obj/map/node#" + sz);

		if (obj) {
			check_node(obj);
		}
	}

	if (sz >= 0) {
		call_out("node_check_tick", 0, sz);
	}
}

void check()
{
	call_out("root_check_tick", 0, status(ST_OTABSIZE) - 1);
}
