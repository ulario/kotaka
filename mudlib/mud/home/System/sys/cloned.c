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
#include <kotaka/paths.h>

inherit SECOND_AUTO;
/*
void discover_clones()
{
	ACCESS_CHECK(PRIVILEGED());

	rlimits (0; -1) {
		object indices;
		string *owners;
		object first;
		object selfqueue;
		int sz, i;

		indices = PROGRAMD->query_object_indices();
		sz = indices->get_size();

		for (i = 0; i < sz; i++) {
			object pinfo;
			int count;

			pinfo = PROGRAMD->get_element(indices->get_element(i));
			pinfo->reset_clones();
		}

		owners = KERNELD->query_owners();
		sz = sizeof(owners);

		for (i = 0; i < sz; i++) {
			object current;
			int nclones;

			first = KERNELD->first_link(owners[i]);

			current = first;

			if (!current) {
				continue;
			}

			do {
				if (sscanf(object_name(current), "%*s#%*d")) {
					rqueue->push_back( ({ 1, status(current, O_INDEX), current }) );
				}
				current = KERNELD->next_link(current);
				nclones++;
			} while (current != first);
		}

		if (!in_objectd) {
			flush_rqueue();
		}
	}
}
*/

/*
void audit_clones()
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	rlimits (0; -1) {
		object indices;
		string *owners;
		object first;
		int orsz, odsz, i;

		object truecounts;

		truecounts = new_object(BIGSTRUCT_MAP_LWO);
		truecounts->set_type(T_INT);
		indices = objdb->get_indices();
		odsz = indices->get_size();

		for (i = 0; i < odsz; i++) {
			truecounts->set_element(indices->get_element(i), 0);
		}

		owners = KERNELD->query_owners();
		orsz = sizeof(owners);

		for (i = 0; i < orsz; i++) {
			object current;
			int nclones;

			first = KERNELD->first_link(owners[i]);

			current = first;

			if (!current) {
				continue;
			}

			do {
				int oindex;
				string path;
				object pinfo;

				path = object_name(current);

				if (!sscanf(path, "%*s#%*d")) {
					current = KERNELD->next_link(current);
					continue;
				}

				oindex = status(current, O_INDEX);
				truecounts->set_element(oindex,
					truecounts->get_element(oindex) + 1);
				current = KERNELD->next_link(current);
			} while (current != first);
		}

		ASSERT(truecounts->get_indices()->get_size() == odsz);

		for (i = 0; i < odsz; i++) {
			int mycount;
			int truecount;
			int oindex;
			object pinfo;

			oindex = indices->get_element(i);
			pinfo = objdb->get_element(oindex);
			mycount = pinfo->query_clone_count();
			truecount = truecounts->get_element(oindex);

			if (mycount != truecount) {
				DRIVER->message("ObjectD audit fail: " + pinfo->query_path() + ", recorded " + mycount + ", counted " + truecount + "\n");
			}
		}
	}
}
*/
