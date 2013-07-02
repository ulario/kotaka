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
#include <kernel/access.h>
#include <kotaka/assert.h>
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

int bigready;	/* if we're using bigstructs */

mixed progdb;	/* program database, index -> program_info */
mixed pathdb;	/* path database, name -> latest index */
mixed inhdb;	/* inherit database, index -> inherited indices */
mixed incdb;	/* include database, index -> included files */

static void create()
{
	progdb = ([ ]);
	inhdb = ([ ]);
	incdb = ([ ]);
	pathdb = ([ ]);
}

void convert_database()
{
	mixed *ind;
	mixed *val;
	mapping old;

	int i, sz;

	ASSERT(!bigready);

	/* program database */

	old = progdb;
	progdb = clone_object(BIGSTRUCT_MAP_OBJ);
	progdb->set_type(T_INT);

	ind = map_indices(old);
	val = map_values(old);

	sz = sizeof(ind);

	for (i = 0; i < sz; i++) {
		progdb->set_element(ind[i], val[i]);
	}

	/* inherit database */

	old = inhdb;
	inhdb = clone_object(BIGSTRUCT_MAP_OBJ);
	inhdb->set_type(T_INT);

	ind = map_indices(old);
	val = map_values(old);

	sz = sizeof(ind);

	for (i = 0; i < sz; i++) {
		int *sub;
		int j;
		object submap;

		int ssz;

		sub = map_indices(val[i]);
		ssz = sizeof(sub);

		submap = new_object(BIGSTRUCT_MAP_LWO);
		submap->set_type(T_INT);
		inhdb->set_element(ind[i], submap);

		for (j = 0; j < ssz; j++) {
			submap->set_element(sub[j], 1);
		}
	}

	/* include database */

	old = incdb;
	incdb = clone_object(BIGSTRUCT_MAP_OBJ);
	incdb->set_type(T_STRING);

	ind = map_indices(old);
	val = map_values(old);

	sz = sizeof(ind);

	for (i = 0; i < sz; i++) {
		int *sub;
		int j;
		object submap;

		int ssz;

		sub = map_indices(val[i]);
		ssz = sizeof(sub);

		submap = new_object(BIGSTRUCT_MAP_LWO);
		submap->set_type(T_INT);
		incdb->set_element(ind[i], submap);

		for (j = 0; j < ssz; j++) {
			submap->set_element(sub[j], 1);
		}
	}

	/* path database */

	old = pathdb;
	pathdb = clone_object(BIGSTRUCT_MAP_OBJ);
	pathdb->set_type(T_STRING);

	ind = map_indices(old);
	val = map_values(old);

	sz = sizeof(ind);

	for (i = 0; i < sz; i++) {
		pathdb->set_element(ind[i], val[i]);
	}

	bigready = 1;
	call_out("defragment", 5);
}

static void defragment()
{
	call_out("defragment", 3600 + random(1800));

	progdb->reindex();
}

private void deindex_inherits(int oindex, int *inh)
{
	if (bigready) {
		int sz;
		int i;

		sz = sizeof(inh);

		for (i = 0; i < sz; i++) {
			object submap;
			submap = inhdb->get_element(inh[i]);
			submap->set_element(oindex, nil);
		}
	} else {
		int sz;
		int i;

		sz = sizeof(inh);

		for (i = 0; i < sz; i++) {
			inhdb[inh[i]][oindex] = nil;
		}
	}
}

private void deindex_includes(int oindex, string *inc)
{
	if (bigready) {
		int sz;
		int i;

		sz = sizeof(inc);

		for (i = 0; i < sz; i++) {
			object submap;
			submap = incdb->get_element(inc[i]);
			submap->set_element(oindex, nil);
		}
	} else {
		int sz;
		int i;

		sz = sizeof(inc);

		for (i = 0; i < sz; i++) {
			incdb[inc[i]][oindex] = nil;
		}
	}
}

private void index_inherits(int oindex, int *inh)
{
	if (bigready) {
		int sz;
		int i;

		sz = sizeof(inh);

		for (i = 0; i < sz; i++) {
			object submap;
			submap = inhdb->get_element(inh[i]);

			if (!submap) {
				inhdb->set_element(inh[i], submap = new_object(BIGSTRUCT_MAP_LWO));
				submap->set_type(T_INT);
			}

			submap->set_element(oindex, 1);
		}
	} else {
		int sz;
		int i;

		sz = sizeof(inh);

		for (i = 0; i < sz; i++) {
			if (!inhdb[inh[i]]) {
				inhdb[inh[i]] = ([ ]);
			}

			inhdb[inh[i]][oindex] = 1;
		}
	}
}

private void index_includes(int oindex, string *inc)
{
	if (bigready) {
		int sz;
		int i;

		sz = sizeof(inc);

		for (i = 0; i < sz; i++) {
			object submap;
			submap = incdb->get_element(inc[i]);

			if (!submap) {
				incdb->set_element(inc[i], submap = new_object(BIGSTRUCT_MAP_LWO));
				submap->set_type(T_INT);
			}

			submap->set_element(oindex, 1);
		}
	} else {
		int sz;
		int i;

		sz = sizeof(inc);

		for (i = 0; i < sz; i++) {
			if (!incdb[inc[i]]) {
				incdb[inc[i]] = ([ ]);
			}

			incdb[inc[i]][oindex] = 1;
		}
	}
}

void register_program(string path, string *inherits,
	string *includes, string constructor, string destructor)
{
	int i;
	int sz;
	int oindex;
	object pinfo;
	int *oindices;
	string *ctors;
	string *dtors;

	ACCESS_CHECK(previous_program() == OBJECTD);

	oindex = status(path)[O_INDEX];

	sz = sizeof(inherits);
	oindices = allocate(sz);
	ctors = ({ });
	dtors = ({ });

	for (i = 0; i < sz; i++) {
		object subpinfo;
		int suboindex;

		suboindex = status(inherits[i])[O_INDEX];
		oindices[i] = suboindex;

		if (bigready) {
			subpinfo = progdb->get_element(suboindex);
		} else {
			subpinfo = progdb[suboindex];
		}

		if (subpinfo) {
			ctors |= subpinfo->query_inherited_constructors();
			ctors |= ({ subpinfo->query_constructor() });
			dtors |= subpinfo->query_inherited_destructors();
			dtors |= ({ subpinfo->query_destructor() });
		}
	}

	ctors -= ({ nil });
	dtors -= ({ nil });

	if (bigready) {
		pinfo = progdb->get_element(oindex);

		if (pathdb)
			pathdb->set_element(path, oindex);
	} else {
		pinfo = progdb[oindex];

		if (pathdb)
			pathdb[path] = oindex;
	}

	if (pinfo) {
		deindex_inherits(oindex, pinfo->query_inherits());
		deindex_includes(oindex, pinfo->query_includes());
	} else {
		pinfo = new_object(PROGRAM_INFO);
		pinfo->set_path(path);

		if (bigready) {
			progdb->set_element(oindex, pinfo);
		} else {
			progdb[oindex] = pinfo;
		}
	}

	pinfo->set_inherits(oindices);
	pinfo->set_includes(includes);
	pinfo->set_inherited_constructors(ctors);
	pinfo->set_constructor(constructor);
	pinfo->set_inherited_destructors(dtors);
	pinfo->set_destructor(destructor);

	index_inherits(oindex, oindices);
	index_includes(oindex, includes);
}

object query_program_indices()
{
	object indices;

	indices = progdb->get_indices();

	indices->grant_access(previous_object(), FULL_ACCESS);

	return indices;
}

object query_includer_indices()
{
	object indices;

	indices = incdb->get_indices();

	indices->grant_access(previous_object(), FULL_ACCESS);

	return indices;
}

object query_program_info(int oindex)
{
	if (bigready) {
		return progdb->get_element(oindex);
	} else {
		return progdb[oindex];
	}
}

int query_program_index(string path)
{
	if (bigready) {
		return pathdb->get_element(path);
	} else {
		return pathdb[path];
	}
}

object query_inheriters(int oindex)
{
	object list;

	ASSERT(bigready);

	list = inhdb->get_element(oindex);

	if (list) {
		list = list->get_indices();
		list->grant_access(previous_object(), FULL_ACCESS);
		return list;
	}
}

object query_includers(string path)
{
	object list;

	ASSERT(bigready);

	list = incdb->get_element(path);

	if (list) {
		list = list->get_indices();
		list->grant_access(previous_object(), FULL_ACCESS);
		return list;
	}
}

void remove_program(int index)
{
	object pinfo;
	string path;

	ACCESS_CHECK(previous_program() == OBJECTD);

	pinfo = query_program_info(index);

	if (pinfo) {
		path = pinfo->query_path();

		deindex_inherits(index, pinfo->query_inherits());
		deindex_includes(index, pinfo->query_includes());
	}

	if (bigready) {
		if (path && pathdb->query_element(path) == index) {
			pathdb->set_element(path, nil);
		}

		progdb->set_element(index, nil);
	} else {
		if (path && pathdb[path] == index) {
			pathdb[path] = nil;
		}

		progdb[index] = nil;
	}
}

void reset_program_database()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	destruct_object(progdb);
	progdb = clone_object(BIGSTRUCT_MAP_OBJ);
	progdb->set_type(T_INT);

	destruct_object(inhdb);
	inhdb = clone_object(BIGSTRUCT_MAP_OBJ);
	inhdb->set_type(T_INT);

	destruct_object(incdb);
	incdb = clone_object(BIGSTRUCT_MAP_OBJ);
	incdb->set_type(T_STRING);

	destruct_object(incdb);
	pathdb = clone_object(BIGSTRUCT_MAP_OBJ);
	pathdb->set_type(T_STRING);
}

void upgrading()
{
	ACCESS_CHECK(SYSTEM());

	if (!pathdb) {
		pathdb = clone_object(BIGSTRUCT_MAP_OBJ);
		pathdb->set_type(T_STRING);
	}
}
