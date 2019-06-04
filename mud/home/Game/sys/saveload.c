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

#define FANOUT 100

object oindex2onum; /* ([ oindex: idnum ]) */
object objlist;	/* ({ idnum: obj, data }) */

private void purge_object(object obj, mixed **list)
{
	object *inv;
	int sz;

	inv = obj->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		list_push_front(list, inv[sz]);
	}

	destruct_object(obj);
}

private void purge_object_directory(string dir, mixed **list)
{
	string *names;
	string *directories;
	int sz;

	directories = IDD->query_directories(dir);
	names = IDD->query_names(dir);

	if (!directories) {
		return;
	}

	for (sz = sizeof(directories); --sz >= 0; ) {
		if (dir) {
			list_push_back(list, dir + ":" + directories[sz]);
		} else {
			list_push_back(list, directories[sz]);
		}
	}

	for (sz = sizeof(names); --sz >= 0; ) {
		if (dir) {
			list_push_back(list, IDD->find_object_by_name(dir + ":" + names[sz]));
		} else {
			list_push_back(list, IDD->find_object_by_name(names[sz]));
		}
	}
}

static void load_world_purge(mixed **list)
{
	catch {
		int done;
		mixed data;

		if (!list_empty(list)) {
			data = list_front(list);
			list_pop_front(list);

			switch(typeof(data)) {
			case T_STRING:
				purge_object_directory(data, list);
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
			call_out("load_world_spawn", 0, 0);
		} else {
			call_out("load_world_purge", 0, list);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
	}
}

private void spawn_object(int i, string data)
{
	object obj;
	int j;

	obj = GAME_INITD->create_thing();
	sscanf(object_name(obj), "%*s#%d", j);

	oindex2onum->set_element(j, i);
	objlist->push_back( ({ obj, data }) );
}

static void load_world_spawn(int i)
{
	catch {
		int done;
		string data;

		if (CONFIGD->file_info("save/" + i + ".obj")) {
			data = CONFIGD->read_file("save/" + i + ".obj");
		} else if (CONFIGD->file_info("save/" + (i - i % FANOUT) + "/" + i + ".obj")) {
			data = CONFIGD->read_file("save/" + (i - i % FANOUT) + "/" + i + ".obj");
		} else if (i > 10) {
			call_out("load_world_name", 0, i);
			return;
		}

		if (data) {
			spawn_object(i, data);
		}

		i++;

		call_out("load_world_spawn", 0, i);
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
	}
}

static void load_world_name(int i)
{
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

static void load_world_set(int i)
{
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

private void purge_directory(string dir)
{
	string *names;

	LOGD->post_message("system", LOG_NOTICE, "Purging directory " + dir);

	// loop in case of overflow
	do {
		mixed **arr;
		int *sizes;
		int sz;

		arr = CONFIGD->get_dir(dir + "/*");

		if (!arr) {
			return;
		}

		names = arr[0];
		sizes = arr[1];

		// loop in case of overflow
		LOGD->post_message("system", LOG_NOTICE, "There are " + sizeof(names) + " entries");

		for (sz = sizeof(names); --sz >= 0; ) {
			if (sizes[sz] == -2) {
				purge_directory(dir + "/" + names[sz]);
			} else {
				LOGD->post_message("system", LOG_NOTICE, "Removing file " + names[sz]);
				CONFIGD->remove_file(dir + "/" + names[sz]);
			}
		}
	} while (names && sizeof(names));

	CONFIGD->remove_dir(dir);
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

private void put_object_directory(string dir, mixed **list)
{
	string *directories;
	string *names;
	int sz;

	directories = IDD->query_directories(dir);
	names = IDD->query_names(dir);

	for (sz = sizeof(directories); --sz >= 0; ) {
		if (dir) {
			list_push_front(list, dir + ":" + directories[sz]);
		} else {
			list_push_front(list, directories[sz]);
		}
	}

	for (sz = sizeof(names); --sz >= 0; ) {
		if (dir) {
			list_push_front(list, IDD->find_object_by_name(dir + ":" + names[sz]));
		} else {
			list_push_front(list, IDD->find_object_by_name(names[sz]));
		}
	}
}

private void prepare_save_dirs(int n)
{
	int i;

	for (i = n - n % FANOUT; i >= FANOUT; i -= FANOUT) {
		CONFIGD->make_dir("save-tmp/" + i);
	}
}

static void save_world_put(mixed **list)
{
	catch {
		int done;
		mixed data;

		if (!list_empty(list)) {
			data = list_front(list);
			list_pop_front(list);

			switch(typeof(data)) {
			case T_STRING:
				put_object_directory(data, list);
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
			int nobj;

			nobj = objlist->query_size();

			if (!nobj) {
				return;
			}

			if (nobj > FANOUT * FANOUT) {
				error("Too many objects to save");
			}

			CONFIGD->make_dir(".");
			CONFIGD->make_dir("save-tmp");

			if (nobj > FANOUT) {
				prepare_save_dirs(nobj);
			}

			call_out("save_world_write", 0, nobj - 1);
		} else {
			call_out("save_world_put", 0, list);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World save aborted");
	}
}

static void save_world_write(int i)
{
	catch {
		string sprint;

		sprint = hybrid_sprint(
			objlist->query_element(i)[1]
		) + "\n";

		switch(i) {
		case 0 .. FANOUT - 1:
			CONFIGD->write_file("save-tmp/" + i + ".obj", sprint);
			break;

		case FANOUT .. FANOUT * FANOUT - 1:
			CONFIGD->write_file("save-tmp/" + (i - i % FANOUT) + "/" + i + ".obj", sprint);
			break;
		}

		if (i > 0) {
			call_out("save_world_write", 0, i - 1);
		} else {
			if (CONFIGD->file_info("save")) {
				if (CONFIGD->file_info("save-old")) {
					purge_directory("save-old");
				}
				CONFIGD->rename_file("save", "save-old");

				ASSERT(CONFIGD->file_info("save-old"));
				ASSERT(!CONFIGD->file_info("save"));
			}

			CONFIGD->rename_file("save-tmp", "save");
			ASSERT(CONFIGD->file_info("save"));
			ASSERT(!CONFIGD->file_info("save-tmp"));

			LOGD->post_message("system", LOG_INFO, "World saved");
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World save aborted, error writing object #" + i);
	}
}

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

	purge_object_directory(nil, list);

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

	if (CONFIGD->file_info("save-tmp")) {
		purge_directory("save-tmp");
	}

	ASSERT(!CONFIGD->file_info("save-tmp"));

	put_object_directory(nil, list);

	call_out("save_world_put", 0, list);
}
