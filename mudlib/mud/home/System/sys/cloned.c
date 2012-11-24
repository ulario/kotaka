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
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

object db;
object queue;
int depth;

mapping kclones;	/* tracker for kernel clones */

private void flush();
private void discover_clones();

static void create()
{
	depth++;

	kclones = ([ ]);
	queue = new_object(BIGSTRUCT_DEQUE_LWO);
	db = clone_object(BIGSTRUCT_MAP_OBJ);
	db->set_type(T_INT);

	depth--;

	discover_clones();
	flush();
}

private void flush()
{
	while (!queue->empty()) {
		object first;
		object next;
		int ad;
		int index;
		int kernel;
		int count;
		object obj;

		({ ad, obj }) = queue->get_front();
		queue->pop_front();

		if (!obj) {
			continue;
		}

		index = status(obj, O_INDEX);
		first = db->get_element(index);

		kernel = sscanf(object_name(obj), "/kernel/%*s");

		depth++;

		if (ad) {
			if (kernel) {
				if (!kclones[index]) {
					kclones[index] = ([ ]);
				}

				kclones[index][obj] = 1;
			} else {
				if (first) {
					next = first->query_next_clone();
				} else {
					db->set_element(status(obj, O_INDEX), obj);
					first = obj;
					next = obj;
				}

				next->set_prev_clone(obj);
				first->set_next_clone(obj);
				obj->set_prev_clone(first);
				obj->set_next_clone(next);
			}
		} else {
			if (kernel) {
				kclones[index][obj] = nil;

				if (!map_sizeof(kclones[index])) {
					kclones[index] = nil;
				}
			} else {
				next = first->query_next_clone();

				if (first == next) {
					db->set_element(index, nil);
				} else {
					object prev;

					if (first == obj) {
						first = next;
					}

					next = obj->query_next_clone();
					prev = obj->query_prev_clone();

					prev->set_next_clone(next);
					next->set_prev_clone(prev);
				}
			}
		}

		depth--;
	}
}

void add_clone(object obj)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	if (!db) {
		return;
	}

	depth++;

	queue->push_back( ({ 1, obj }) );

	depth--;

	if (depth == 0) {
		flush();
	}
}

void remove_clone(object obj)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	if (!db) {
		return;
	}

	depth++;

	queue->push_back( ({ 0, obj }) );

	depth--;

	if (depth == 0) {
		flush();
	}
}

private void discover_clones()
{
	string *owners;
	int i, sz;
	int count;

	owners = KERNELD->query_owners();
	sz = sizeof(owners);

	for (i = 0; i < sz; i++) {
		object first;

		first = KERNELD->first_link(owners[i]);

		if (first) {
			object obj;
			string name;

			obj = first;

			do {
				name = object_name(obj);

				if (sscanf(name, "%*s#%*d")) {
					queue->push_back( ({ 1, obj }) );
					count++;
				}
				obj = KERNELD->next_link(obj);
			} while (obj != first);
		}
	}
}
