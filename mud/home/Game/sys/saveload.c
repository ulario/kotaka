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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <game/paths.h>
#include <status.h>
#include <type.h>

inherit "~System/lib/struct/list";
inherit "/lib/string/sprint";

object oindex2onum; /* ([ oindex: idnum ]) */
object objlist;	/* ({ idnum: obj, data }) */

/* private helper functions */

/* - loading */

private void purge_object(object obj, mixed **list)
{
	int sz;
	object *inv;
	string name;

	inv = obj->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		list_push_front(list, inv[sz]);
	}

	name = obj->query_object_name();

	destruct_object(obj);
}

private void purge_directory(string dir, mixed **list)
{
	mapping dlist;
	int *keys;
	string *names;
	int i, sz;

	if (CATALOGD->test_name(dir) != 2) {
		/* if the directory's already empty, don't purge it */
		return;
	}

	dlist = CATALOGD->list_directory(dir);

	names = map_indices(dlist);
	keys = map_values(dlist);

	sz = sizeof(keys);

	for (i = 0; i < sz; i++) {
		if (keys[i] == 2) {
			if (dir) {
				list_push_front(list, dir + ":" + names[i]);
			} else {
				list_push_front(list, names[i]);
			}
		} else {
			if (dir) {
				list_push_front(list, CATALOGD->lookup_object(dir + ":" + names[i]));
			} else {
				list_push_front(list, CATALOGD->lookup_object(names[i]));
			}
		}
	}
}

void load_world_purge(mixed **list)
{
	ACCESS_CHECK(SYSTEM());

	catch {
		int done;
		mixed data;

		if (!list_empty(list)) {
			data = list_front(list);
			list_pop_front(list);

			switch(typeof(data)) {
			case T_STRING:
				purge_directory(data, list);
				break;

			case T_OBJECT:
				purge_object(data, list);
				break;

			case T_NIL:
				/* object already destructed */
				break;
			}
		} else {
			done = 1;
		}

		if (done) {
			call_out("load_world_spawn", 0, 1);
		} else {
			call_out("load_world_purge", 0, list);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
	}
}

void load_world_spawn(int i)
{
	ACCESS_CHECK(SYSTEM());

	catch {
		int done;

		if (CONFIGD->file_info("save/" + i + ".obj")) {
			object obj;
			int j;

			obj = GAME_INITD->create_thing();
			sscanf(object_name(obj), "%*s#%d", j);

			oindex2onum->set_element(j, i);

			objlist->push_back( ({ obj,
				CONFIGD->read_file("save/" + i + ".obj") }) );
			i++;
		} else {
			done = 1;
		}

		if (done) {
			call_out("load_world_name", 0, i - 1);
		} else {
			call_out("load_world_spawn", 0, i);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
	}
}

void load_world_name(int i)
{
	ACCESS_CHECK(SYSTEM());

	catch {
		if (i > 0) {
			object obj;
			mixed *arr;
			mixed data;

			({ obj, data }) = objlist->query_element(i - 1);

			data = PARSER_VALUE->parse(data);

			obj->move(data["environment"]);
			data["environment"] = nil;

			obj->set_object_name(data["name"]);
			data["name"] = nil;

			objlist->set_element(i - 1, ({ obj, data }) );

			i--;

			call_out("load_world_name", 0, i);
		} else {
			call_out("load_world_set", 0, objlist->query_size() - 1);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted parsing data for object " + i);
	}
}

void load_world_set(int i)
{
	ACCESS_CHECK(SYSTEM());

	catch {
		if (i > 0) {
			object obj;
			mixed data;

			({ obj, data }) = objlist->query_element(i - 1);

			obj->load(data);
			i--;

			call_out("load_world_set", 0, i);
		} else {
			LOGD->post_message("system", LOG_INFO, "World loaded");
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted loading data for object " + i);
	}
}

/* - saving */

private void purge_savedir()
{
	string *names;

	do {
		int sz;

		names = CONFIGD->get_dir("save/*")[0];

		if (names) {
			for (sz = sizeof(names); --sz >= 0; ) {
				CONFIGD->remove_file("save/" + names[sz]);
			}
		}
	} while (names && sizeof(names));
}

private void put_object(object obj, mixed **list)
{
	object *inv;
	int oindex;
	int sz;

	ASSERT(sscanf(object_name(obj), "%*s#%d", oindex));

	if (oindex2onum->query_element(oindex) != nil) {
		return;
	}

	sz = objlist->query_size();
	oindex2onum->set_element(oindex, sz + 1);
	objlist->push_back( ({ obj, obj->save() }) );

	inv = obj->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		list_push_front(list, inv[sz]);
	}
}

private void put_directory(string dir, mixed **list)
{
	mapping dlist;
	int *keys;
	string *names;
	int i, sz;

	dlist = CATALOGD->list_directory(dir);

	names = map_indices(dlist);
	keys = map_values(dlist);

	for (sz = sizeof(keys); --sz >= 0; ) {
		if (keys[sz] == 2) {
			if (dir) {
				list_push_front(list, dir + ":" + names[sz]);
			} else {
				list_push_front(list, names[sz]);
			}
		} else {
			if (dir) {
				list_push_front(list, CATALOGD->lookup_object(dir + ":" + names[sz]));
			} else {
				list_push_front(list, CATALOGD->lookup_object(names[sz]));
			}
		}
	}
}

void save_world_put(mixed **list)
{
	ACCESS_CHECK(SYSTEM());

	catch {
		int done;
		mixed data;

		if (!list_empty(list)) {
			data = list_front(list);
			list_pop_front(list);

			switch(typeof(data)) {
			case T_STRING:
				put_directory(data, list);
				break;

			case T_OBJECT:
				put_object(data, list);
				break;

			case T_NIL:
				ASSERT(0);
			}
		} else {
			done = 1;
		}

		if (done) {
			CONFIGD->make_dir(".");
			CONFIGD->make_dir("save");

			call_out("save_world_write", 0, objlist->query_size());
		} else {
			call_out("save_world_put", 0, list);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World save aborted");
	}
}

void save_world_write(int i)
{
	ACCESS_CHECK(SYSTEM());

	catch {
		i--;

		CONFIGD->write_file("save/" + (i + 1) + ".obj",
			hybrid_sprint(
				objlist->query_element(i)[1]
			) + "\n"
		);

		if (i > 0) {
			call_out("save_world_write", 0, i);
		} else {
			LOGD->post_message("system", LOG_INFO, "World saved");
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World save aborted");
	}
}

/* public helper functions */

object *parse_object(string input)
{
	int onum;
	string uname;

	if (sscanf(input, "O:%d", onum)) {
		return ({ objlist->query_element(onum - 1)[0] });
	} else if (sscanf(input, "U:%s", uname)) {
		return ({ TEXT_USERD->find_user(uname) });
	} else {
		error("Cannot parse: " + input);
	}
}

string sprint_object(object obj, varargs mapping seen)
{
	string path;
	int oindex;

	if (sscanf(object_name(obj), "%s#%d", path, oindex)) {
		switch(path) {
		case USR_DIR + "/Game/obj/thing":
			return "<O:" + oindex2onum->query_element(oindex) + ">";
		case USR_DIR + "/Text/obj/user":
			return "<U:" + obj->query_name() + ">";
		default:
			error("Unknown object type (" + path + ")");
		}
	} else {
		return ::sprint_object(obj, seen);
	}
}

/* public functions */

void load_world()
{
	int sz;
	int count;
	mixed **list;

	oindex2onum = new_object(BIGSTRUCT_MAP_LWO);
	oindex2onum->claim();
	oindex2onum->set_type(T_INT);

	objlist = new_object(BIGSTRUCT_ARRAY_LWO);
	objlist->claim();

	list = ({ nil, nil });

	purge_directory(nil, list);

	call_out("load_world_purge", 0, list);
}

void save_world()
{
	int sz;
	mixed **list;

	oindex2onum = new_object(BIGSTRUCT_MAP_LWO);
	oindex2onum->claim();
	oindex2onum->set_type(T_INT);

	objlist = new_object(BIGSTRUCT_ARRAY_LWO);
	objlist->claim();

	list = ({ nil, nil });

	purge_savedir();

	put_directory(nil, list);

	call_out("save_world_put", 0, list);
}
