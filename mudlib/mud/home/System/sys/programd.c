/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2002, 2010, 2012, 2013, 2014  Raymond Jennings
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
object pathdb;	/* path database, name -> latest index */
object inhdb;	/* inherit database, index -> inherited indices */
object incdb;	/* include database, index -> included files */

private void setup_database()
{
	ASSERT(!progdb);
	ASSERT(!inhdb);
	ASSERT(!incdb);
	ASSERT(!pathdb);

	progdb = clone_object(BIGSTRUCT_MAP_OBJ);
	progdb->set_type(T_INT);

	inhdb = clone_object(BIGSTRUCT_MAP_OBJ);
	inhdb->set_type(T_INT);

	incdb = clone_object(BIGSTRUCT_MAP_OBJ);
	incdb->set_type(T_STRING);

	pathdb = clone_object(BIGSTRUCT_MAP_OBJ);
	pathdb->set_type(T_STRING);
}

static void create()
{
	setup_database();
}

private void deindex_inherits(int oindex, int *inh)
{
	int sz;
	int i;

	sz = sizeof(inh);

	for (i = 0; i < sz; i++) {
		object submap;
		submap = inhdb->query_element(inh[i]);
		submap->set_element(oindex, nil);
	}
}

private void deindex_includes(int oindex, string *inc)
{
	int sz;
	int i;

	sz = sizeof(inc);

	for (i = 0; i < sz; i++) {
		object submap;
		submap = incdb->query_element(inc[i]);
		submap->set_element(oindex, nil);
	}
}

private void index_inherits(int oindex, int *inh)
{
	int sz;
	int i;

	sz = sizeof(inh);

	for (i = 0; i < sz; i++) {
		object submap;
		submap = inhdb->query_element(inh[i]);

		if (!submap) {
			inhdb->set_element(inh[i], submap = new_object(BIGSTRUCT_MAP_LWO));
			submap->set_type(T_INT);
		}

		submap->set_element(oindex, 1);
	}
}

private void index_includes(int oindex, string *inc)
{
	int sz;
	int i;

	sz = sizeof(inc);

	for (i = 0; i < sz; i++) {
		object submap;
		submap = incdb->query_element(inc[i]);

		if (!submap) {
			incdb->set_element(inc[i], submap = new_object(BIGSTRUCT_MAP_LWO));
			submap->set_type(T_INT);
		}

		submap->set_element(oindex, 1);
	}
}

object register_program(string path, string *inherits,
	string *includes)
{
	int i;
	int sz;
	int oindex;
	object pinfo;
	int *oindices;
	string *ctors;
	string *dtors;
	string *touchers;

	ACCESS_CHECK(previous_program() == OBJECTD);

	oindex = status(path, O_INDEX);

	sz = sizeof(inherits);
	oindices = allocate(sz);
	ctors = ({ });
	dtors = ({ });
	touchers = ({ });

	for (i = 0; i < sz; i++) {
		object subpinfo;
		int suboindex;

		suboindex = status(inherits[i])[O_INDEX];
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

			inh = subpinfo->query_inherited_touchers();

			if (inh) {
				touchers |= inh;
			}

			touchers |= ({ subpinfo->query_toucher() });
		}
	}

	ctors -= ({ nil });
	dtors -= ({ nil });
	touchers -= ({ nil });

	pinfo = progdb->query_element(oindex);

	if (pathdb)
		pathdb->set_element(path, oindex);

	if (pinfo) {
		deindex_inherits(oindex, pinfo->query_inherits());
		deindex_includes(oindex, pinfo->query_includes());
	} else {
		pinfo = new_object(PROGRAM_INFO);
		pinfo->set_path(path);

		progdb->set_element(oindex, pinfo);
	}

	pinfo->set_inherits(oindices);
	pinfo->set_includes(includes);
	pinfo->set_inherited_constructors(ctors);
	pinfo->set_inherited_destructors(dtors);
	pinfo->set_inherited_touchers(touchers);

	index_inherits(oindex, oindices);
	index_includes(oindex, includes);

	return pinfo;
}

object query_program_indices()
{
	object indices;

	indices = progdb->query_indices();

	indices->grant_access(previous_object(), FULL_ACCESS);
	indices->grant_access(this_object(), 0);

	return indices;
}

object query_includer_indices()
{
	object indices;

	indices = incdb->query_indices();

	indices->grant_access(previous_object(), FULL_ACCESS);
	indices->grant_access(this_object(), 0);

	return indices;
}

object query_program_info(int oindex)
{
	return progdb->query_element(oindex);
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

	list = inhdb->query_element(oindex);

	if (list) {
		list = list->query_indices();
	} else {
		list = new_object(BIGSTRUCT_ARRAY_LWO);
	}

	list->grant_access(previous_object(), FULL_ACCESS);
	list->grant_access(this_object(), 0);

	return list;
}

object query_includers(string path)
{
	object list;

	list = incdb->query_element(path);

	if (list) {
		list = list->query_indices();
	} else {
		list = new_object(BIGSTRUCT_ARRAY_LWO);
	}

	list->grant_access(previous_object(), FULL_ACCESS);
	list->grant_access(this_object(), 0);

	return list;
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

	if (path && pathdb->query_element(path) == index) {
		pathdb->set_element(path, nil);
	}

	progdb->set_element(index, nil);
}

atomic void reset_program_database()
{
	int i, sz;
	object *turkeys;

	ACCESS_CHECK(previous_program() == OBJECTD);

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

	setup_database();
}

static void destruct()
{
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
