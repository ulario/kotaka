/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

object progdb;	/* program database, index -> program_info */
object pathdb;	/* path database, filename -> latest index */
object inhdb;	/* inherit database, filename -> inheriting objects */
object incdb;	/* include database, filename -> including objects */

atomic void create_database();
atomic void destruct_database();

/* create/destruct */

static void create()
{
}

static void destruct()
{
	destruct_database();
}

/* helpers */

atomic void create_database()
{
	ACCESS_CHECK(SYSTEM());
	ASSERT(find_object(USR_DIR + "/Bigstruct/initd"));
	ASSERT(find_object(BIGSTRUCT_MAP_OBJ));

	rlimits (0; -1) {
		progdb = clone_object(BIGSTRUCT_MAP_OBJ);
		progdb->claim();
		progdb->set_type(T_INT);

		inhdb = clone_object(BIGSTRUCT_MAP_OBJ);
		inhdb->claim();
		inhdb->set_type(T_INT);

		incdb = clone_object(BIGSTRUCT_MAP_OBJ);
		incdb->claim();
		incdb->set_type(T_STRING);

		pathdb = clone_object(BIGSTRUCT_MAP_OBJ);
		pathdb->claim();
		pathdb->set_type(T_STRING);
	}
}

atomic void destruct_database()
{
	ACCESS_CHECK(SYSTEM());

	rlimits (0; -1) {
		int i, sz;
		object *turkeys;

		turkeys = ({ });

		if (progdb) {
			turkeys += ({ progdb });
		}

		if (inhdb) {
			turkeys += ({ inhdb });
		}

		if (incdb) {
			turkeys += ({ incdb });
		}

		if (pathdb) {
			turkeys += ({ pathdb });
		}

		progdb = nil;
		inhdb = nil;
		incdb = nil;
		pathdb = nil;

		sz = sizeof(turkeys);

		for (i = 0; i < sz; i++) {
			destruct_object(turkeys[i]);
		}
	}
}

private void deindex_inherits(int oindex, int *inh)
{
	int sz;

	ASSERT(inhdb);

	for (sz = sizeof(inh); --sz >= 0; ) {
		object submap;
		submap = inhdb->query_element(inh[sz]);
		submap->set_element(oindex, nil);
	}
}

private void deindex_includes(int oindex, string *inc)
{
	int sz;

	ASSERT(incdb);

	for (sz = sizeof(inc); --sz >= 0; ) {
		object submap;
		submap = incdb->query_element(inc[sz]);
		submap->set_element(oindex, nil);
	}
}

private void index_inherits(int oindex, int *inh)
{
	int sz;

	ASSERT(inhdb);

	for (sz = sizeof(inh); --sz >= 0; ) {
		object submap;
		submap = inhdb->query_element(inh[sz]);

		if (!submap) {
			inhdb->set_element(inh[sz], submap = new_object(BIGSTRUCT_MAP_LWO));
			submap->claim();
			submap->set_type(T_INT);
		}

		submap->set_element(oindex, 1);
	}
}

private void index_includes(int oindex, string *inc)
{
	int sz;

	ASSERT(incdb);

	for (sz = sizeof(inc); --sz >= 0; ) {
		object submap;
		submap = incdb->query_element(inc[sz]);

		if (!submap) {
			incdb->set_element(inc[sz], submap = new_object(BIGSTRUCT_MAP_LWO));
			submap->claim();
			submap->set_type(T_INT);
		}

		submap->set_element(oindex, 1);
	}
}

/* objectd hooks */

atomic object register_program(string path, string *inherits, string *includes)
{
	int oindex;
	object pinfo;

	int *oinherits;
	string *oincludes;

	ACCESS_CHECK(previous_program() == OBJECTD);

	if (!progdb) {
		LOGD->post_message("system", LOG_WARNING, "Skipping registration of " + path + ", no progdb");

		return nil;
	}

	oindex = status(path, O_INDEX);

	if (pathdb) {
		pathdb->set_element(path, oindex);
	}

	pinfo = progdb->query_element(oindex);

	if (!pinfo) {
		pinfo = new_object(PROGRAM_INFO);
		pinfo->set_path(path);

		progdb->set_element(oindex, pinfo);
	}

	oinherits = pinfo->query_inherits();
	oincludes = pinfo->query_includes();

	if (oinherits) {
		deindex_inherits(oindex, oinherits);
	}

	if (oincludes) {
		deindex_includes(oindex, oincludes);
	}

	if (inherits) {
		string *ctors;
		string *dtors;
		int sz;
		int i;

		int *oindices;

		ASSERT(includes);

		sz = sizeof(inherits);
		oindices = allocate(sz);
		ctors = ({ });
		dtors = ({ });

		for (i = 0; i < sz; i++) {
			object subpinfo;
			int suboindex;

			suboindex = status(inherits[i], O_INDEX);
			oindices[i] = suboindex;

			subpinfo = progdb->query_element(suboindex);

			if (subpinfo) {
				mixed inh;

				inh = subpinfo->query_inherited_constructors();

				if (inh) {
					ctors |= inh;
				}

				ctors |= ({ subpinfo->query_constructor() });

				inh = subpinfo->query_inherited_destructors();

				if (inh) {
					dtors |= inh;
				}

				dtors |= ({ subpinfo->query_destructor() });
			} else {
				LOGD->post_message("debug", LOG_WARN, "No program info for inherited program " + inherits[i]);
			}
		}

		ctors -= ({ nil });
		dtors -= ({ nil });

		pinfo->set_inherits(oindices);
		pinfo->set_includes(includes);
		pinfo->set_inherited_constructors(ctors);
		pinfo->set_inherited_destructors(dtors);

		index_inherits(oindex, oindices);
		index_includes(oindex, includes);
	}

	return pinfo;
}

atomic void remove_program(int index)
{
	object pinfo;
	string path;

	int *oinherits;
	string *oincludes;

	ACCESS_CHECK(SYSTEM());

	if (!progdb) {
		return;
	}

	pinfo = progdb->query_element(index);

	if (pinfo) {
		path = pinfo->query_path();

		oinherits = pinfo->query_inherits();
		oincludes = pinfo->query_includes();

		if (oinherits) {
			deindex_inherits(index, oinherits);
		}

		if (oincludes) {
			deindex_includes(index, oincludes);
		}
	}

	if (path && pathdb->query_element(path) == index) {
		pathdb->set_element(path, nil);
	}

	progdb->set_element(index, nil);
}

atomic void reset_program_database()
{
	ACCESS_CHECK(SYSTEM());

	destruct_database();
	create_database();
}

/* public functions */

object query_program_indices()
{
	object indices;

	ASSERT(progdb);

	indices = progdb->query_indices();

	indices->grant_access(previous_object(), FULL_ACCESS);
	indices->grant_access(this_object(), 0);

	return indices;
}

object query_program_info(int oindex)
{
	if (progdb) {
		return progdb->query_element(oindex);
	} else {
		ASSERT(SYSTEM());
	}
}

int query_program_index(string path)
{
	mixed index;

	index = pathdb->query_element(path);

	if (index != nil) {
		return index;
	} else {
		return -1;
	}
}

object query_inheriters(int oindex)
{
	object list;

	ASSERT(inhdb);

	list = inhdb->query_element(oindex);

	if (list) {
		list = list->query_indices();
	} else {
		list = new_object(BIGSTRUCT_ARRAY_LWO);
		list->claim();
	}

	list->grant_access(previous_object(), FULL_ACCESS);
	list->grant_access(this_object(), 0);

	return list;
}

object query_includers(string path)
{
	object list;

	ASSERT(incdb);

	list = incdb->query_element(path);

	if (list) {
		list = list->query_indices();
	} else {
		list = new_object(BIGSTRUCT_ARRAY_LWO);
		list->claim();
	}

	list->grant_access(previous_object(), FULL_ACCESS);
	list->grant_access(this_object(), 0);

	return list;
}
