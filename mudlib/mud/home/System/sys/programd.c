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
mixed progdb;	/* program database */

static void create()
{
	progdb = ([ ]);
}

void convert_database()
{
	int *ind;
	object *val;
	mapping old;

	int i, sz;

	ASSERT(!bigready);

	old = progdb;
	progdb = clone_object(BIGSTRUCT_MAP_OBJ);
	progdb->set_type(T_INT);

	ind = map_indices(old);
	val = map_values(old);

	sz = sizeof(ind);

	for (i = 0; i < sz; i++) {
		progdb->set_element(ind[i], val[i]);
	}

	bigready = 1;

	call_out("defragment", 5);
}

static void defragment()
{
	call_out("defragment", 60);

	progdb->reindex();
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
	} else {
		pinfo = progdb[oindex];
	}

	if (!pinfo) {
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
}

object query_program_indices()
{
	object indices;

	indices = progdb->get_indices();

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

object query_inheriters(int oindex)
{
	object inheriters;
	object indices;
	int i, sz;

	indices = progdb->get_indices();
	inheriters = new_object(BIGSTRUCT_ARRAY_LWO);
	inheriters->grant_access(previous_object(), READ_ACCESS);

	sz = indices->get_size();

	for (i = 0; i < sz; i++) {
		object pinfo;
		int suboindex;

		suboindex = indices->get_element(i);
		pinfo = progdb->get_element(suboindex);

		if (sizeof(pinfo->query_inherits() & ({ oindex }))) {
			inheriters->push_back(suboindex);
		}
	}

	return inheriters;
}

object query_includers(string path)
{
	object includers;
	object indices;
	int i, sz;

	indices = progdb->get_indices();
	includers = new_object(BIGSTRUCT_ARRAY_LWO);
	includers->grant_access(previous_object(), READ_ACCESS);

	sz = indices->get_size();

	for (i = 0; i < sz; i++) {
		object pinfo;
		int suboindex;

		suboindex = indices->get_element(i);
		pinfo = progdb->get_element(suboindex);

		if (sizeof(pinfo->query_includes() & ({ path }))) {
			includers->push_back(suboindex);
		}
	}

	return includers;
}

void remove_program(int index)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	if (bigready) {
		progdb->set_element(index, nil);
	} else {
		progdb[index] = nil;
	}
}

void reset_program_database()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	destruct_object(progdb);

	progdb = clone_object(BIGSTRUCT_MAP_OBJ);
	progdb->set_type(T_INT);
}
