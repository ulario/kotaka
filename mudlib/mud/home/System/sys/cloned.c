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
