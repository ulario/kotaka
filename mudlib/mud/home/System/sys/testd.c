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
#include <kernel/tls.h>

#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

#include <kotaka/bigstruct.h>

#include <type.h>
#include <status.h>

inherit SECOND_AUTO;

private inherit tls API_TLS;

static void create()
{
	tls::create();

	call_out("verify", 1);
}

static void tls_test()
{
	set_tlvar(0, "foobar");

	ASSERT(get_tlvar(0) == "foobar");

	LOGD->post_message("test", LOG_DEBUG, "TLS on callout test passed");
}

private void dump_progdb()
{
	object ind;
	int sz, i;

	ind = PROGRAMD->query_program_indices();
	sz = ind->get_size();

	for (i = 0; i < sz; i++) {
		int oindex;
		object pinfo;
		string buf;
		int ssz, j;

		mixed arr;

		oindex = ind->get_element(i);
		pinfo = PROGRAMD->query_program_info(oindex);

		buf = "Program info for " + oindex + "\n";
		buf += "Name: " + pinfo->query_path() + "\n";
		buf += "Inherits:\n";

		arr = pinfo->query_inherits();
		ssz = sizeof(arr);

		for (j = 0; j < ssz; j++) {
			buf += "    " + PROGRAMD->query_program_info(arr[j])->query_path() + "\n";
		}

		buf += "Includes:\n";

		arr = pinfo->query_includes();
		ssz = sizeof(arr);

		for (j = 0; j < ssz; j++) {
			buf += "    " + arr[j] + "\n";
		}

		arr = PROGRAMD->query_inheriters(oindex);

		if (arr) {
			buf += "Inherited by:\n";

			ssz = arr->get_size();;

			for (j = 0; j < ssz; j++) {
				buf += "    " + PROGRAMD->query_program_info(
					arr->get_element(j)
				)->query_path() + "\n";
			}
		}

		LOGD->post_message("test", LOG_DEBUG, buf);
	}

	ind = PROGRAMD->query_includer_indices();
	sz = ind->get_size();

	for (i = 0; i < sz; i++) {
		object arr;
		string buf;
		int j, ssz;

		buf = "Include file " + ind->get_element(i) + "\n";
		buf += "Included by:\n";

		arr = PROGRAMD->query_includers(ind->get_element(i));
		ssz = arr->get_size();

		for (j = 0; j < ssz; j++) {
			buf += "    " + PROGRAMD->query_program_info(arr->get_element(j))->query_path() + "\n";
		}

		LOGD->post_message("test", LOG_DEBUG, buf);
	}
}

void test()
{
	string err;

	ACCESS_CHECK(SYSTEM());

	dump_progdb();

	call_out("tls_test", 0);
}

private void verify_objregd_owner(string owner)
{
	mapping seen;
	object first;
	object obj;

	seen = ([ ]);
	first = KERNELD->first_link(owner);

	if (!first) {
		return;
	}

	obj = first;

	do {
		seen[obj] = 1;
		obj = KERNELD->next_link(obj);

		if (!obj) {
			shutdown();
			LOGD->post_message("system", LOG_EMERG, "Fatal error:  Corrupted ObjRegD database for owner " + owner);
			send_out("nil\n");
			break;
		}
	} while (!seen[obj]);
}

void verify_objregd()
{
	string *owners;
	int i, sz;

	owners = KERNELD->query_owners();

	sz = sizeof(owners);

	for (i = 0; i < sz; i++) {
		verify_objregd_owner(owners[i]);
	}
}
