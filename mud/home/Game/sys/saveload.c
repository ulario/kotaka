/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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

/* terms: */
/* object index: the object's index in the otable */
/* object number: the number assigned to the object's save file */

inherit "~System/lib/struct/list";
inherit "/lib/string/sprint";

object oi2on; /* ([ oindex : onum ]) */
object on2ob; /* ([ onum : obj ]) */
object on2op; /* ([ onum : path ]) */
object on2oi; /* ([ onum : inv ]) */

/* general helpers */

private void list_enqueue_directory(mixed **list, varargs string dir)
{
	string *names;
	string *directories;
	int sz;

	directories = IDD->query_directories(dir);
	names = IDD->query_names(dir);

	for (sz = sizeof(directories); --sz >= 0; ) {
		list_push_back(list, (dir ? dir + ":": "") + directories[sz]);
	}

	for (sz = sizeof(names); --sz >= 0; ) {
		list_push_back(list, IDD->find_object_by_name((dir ? dir + ":": "") + names[sz]));
	}
}

/* load helpers */

private void enqueue_save_directory(mixed **list, varargs string dir)
{
	string *names;
	int sz;

	names = CONFIGD->get_dir("save/" + (dir ? dir + "/" : "") + "*")[0];

	for (sz = sizeof(names); --sz >= 0; ) {
		list_push_front(list, (dir ? dir + "/" : "") + names[sz]);
	}
}

static void load_purge_tick(mixed **list)
{
	mixed head;

	head = list_front(list);
	list_pop_front(list);

	switch(typeof(head)) {
	case T_NIL:
		/* most likely an enqueued object got destructed before we hit it */
		break;

	case T_OBJECT:
		{
			object *inv;
			int sz;

			inv = head->query_inventory();

			for (sz = sizeof(inv); --sz >= 0; ) {
				list_push_front(list, inv[sz]);
			}
		}
		destruct_object(head);
		break;

	case T_STRING:
		/* directory */
		list_enqueue_directory(list, head);
	}

	if (list_empty(list)) {
		mixed *info;

		info = CONFIGD->file_info("save");

		if (!info) {
			error("No save folder");
		}

		if (info[0] != -2) {
			error("Corrupt save file");
		}

		enqueue_save_directory(list);

		if (!list_empty(list)) {
			on2ob = new_object("~System/lwo/struct/sparse_array");
			on2op = new_object("~System/lwo/struct/sparse_array");

			call_out("load_scan_tick", 0, list, -1);
		} else {
			error("Nothing to load");
		}
	} else {
		call_out("load_purge_tick", 0, list);
	}
}

static void load_scan_tick(mixed **list, int top)
{
	string path;
	mixed *info;

	path = list_front(list);
	list_pop_front(list);

	info = CONFIGD->file_info("save/" + path);

	if (!info) {
		error("Path not found");
	}

	if (info[0] == -2) {
		enqueue_save_directory(list, path);
	} else {
		int pos;
		int on;
		string opath;
		object obj;

		pos = strlen(path) - 1;
		opath = path;

		while (pos >= 0) {
			if (path[pos] == '/') {
				path = path[pos + 1 ..];
				break;
			}

			pos--;
		}

		sscanf(path, "%d.obj", on);

		on2ob->set_element(on, 1);
		on2op->set_element(on, opath);

		if (on > top) {
			top = on;
		}
	}

	if (list_empty(list)) {
		oi2on = new_object("~System/lwo/struct/sparse_array");

		call_out("load_spawn_tick", 0, top, top);
	} else {
		call_out("load_scan_tick", 0, list, top);
	}
}

static void load_spawn_tick(int cur, int top)
{
	if (on2ob->query_element(cur)) {
		object obj;
		int oi;

		obj = clone_object("~/obj/thing");

		sscanf(object_name(obj), "%*s#%d", oi);

		on2ob->set_element(cur, obj);
		oi2on->set_element(oi, cur);
	}

	if (cur) {
		call_out("load_spawn_tick", 0, cur - 1, top);
	} else {
		on2oi = new_object("~System/lwo/struct/sparse_array");

		call_out("load_read_tick", 0, top, top);
	}
}

static void load_read_tick(int cur, int top)
{
	object obj;

	obj = on2ob->query_element(cur);

	if (obj) {
		string save;
		mapping map;

		save = CONFIGD->read_file("save/" + on2op->query_element(cur));

		map = PARSER_VALUE->parse(save);

		on2oi->set_element(cur, map["inventory"]);

		/* inventory will be set after everything is loaded */
		map["inventory"] = nil;

		obj->load(map);
	}

	if (cur) {
		call_out("load_read_tick", 0, cur - 1, top);
	} else {
		oi2on = nil;
		on2op = nil;

		call_out("load_move_tick", 0, top);
	}
}

static void load_move_tick(int cur)
{
	object obj, *inv;

	obj = on2ob->query_element(cur);
	inv = on2oi->query_element(cur);

	if (obj && inv) {
		int sz;

		/* newly inserted objects are put at the front */
		/* just insert from back to front */
		for (sz = sizeof(inv); --sz >= 0; ) {
			inv[sz]->move(obj);
		}
	}

	if (cur) {
		call_out("load_move_tick", 0, cur - 1);
	} else {
		LOGD->post_message("debug", LOG_NOTICE, "World load complete");

		on2ob = nil;
		on2oi = nil;
	}
}

/* scan for objects to save and generate their onums */
static void save_scan_tick(mixed **list, int on)
{
	mixed head;
	int oi;
	mixed val;

	head = list_front(list);
	list_pop_front(list);

	switch(typeof(head)) {
	case T_NIL:
		error("Object destructed during scan");

	case T_STRING:
		list_enqueue_directory(list, head);
		break;

	case T_OBJECT:
		if (!sscanf(object_name(head), "%*s#%d", oi)) {
			error("Tried to scan a master object");
		}

		val = oi2on->query_element(oi);

		if (val == nil) {
			/* new object, register it and add its inventory to the list */
			object *inv;
			int sz;

			/* register new onum to oindex */
			oi2on->set_element(oi, on);
			on2ob->set_element(on, head);

			on++;

			inv = head->query_inventory();

			for (sz = sizeof(inv); --sz >= 0; ) {
				list_push_front(list, inv[sz]);
			}
		}
		break;

	default:
		error("Bad type in scan queue");
	}

	if (list_empty(list)) {
		call_out("save_write_tick", 0, on - 1);
	} else {
		call_out("save_scan_tick", 0, list, on);
	}
}

/* write objects to disk */
static void save_write_tick(int on)
{
	object obj;
	mapping map;
	string save;

	obj = on2ob->query_element(on);

	if (obj) {
		map = obj->save();

		save = hybrid_sprint(map);

		CONFIGD->write_file("save-tmp/" + on + ".obj", save);
	}

	if (on) {
		call_out("save_write_tick", 0, on - 1);
	} else {
		call_out("save_post_tick", 0);
	}
}

/* finalize save */
static void save_post_tick()
{
	mixed *info;

	/* current save?  move it to old */
	/*   old already exists?  delete it */
	info = CONFIGD->file_info("save-old");

	if (info) {
		if (info[0] == -2) {
			call_out("save_purgedir_tick", 0, ({ nil, nil }), "save-old");
			return;
		} else {
			/* corrupt */
			CONFIGD->remove_file("save");
		}
	}

	CONFIGD->rename_file("save", "save-old");
	CONFIGD->rename_file("save-tmp", "save");

	oi2on = nil;
	on2ob = nil;

	LOGD->post_message("debug", LOG_NOTICE, "World save complete");
}

/* purge folder */
static void save_purgedir_tick(mixed **list, string dir)
{
	string path;
	int size;

	if (list_empty(list)) {
		mixed *info;

		info = CONFIGD->file_info(dir);

		if (info) {
			if (info[0] == -2) {
				string *names;
				int sz;

				names = CONFIGD->get_dir(dir + "/*")[0];

				for (sz = sizeof(names); --sz >= 0; ) {
					list_push_front(list, names[sz]);
				}

				if (!list_empty(list)) {
					call_out("save_purgedir_tick", 0, list, dir);
					return;
				}

				CONFIGD->remove_dir(dir);
			} else {
				CONFIGD->remove_file(dir);
			}
		}

		switch(dir) {
		case "save-tmp":
			/* we were wiping the temp dir */
			list = ({ nil, nil });
			oi2on = new_object("~System/lwo/struct/sparse_array");
			on2ob = new_object("~System/lwo/struct/sparse_array");
			list_enqueue_directory(list);
			CONFIGD->make_dir("save-tmp");
			call_out("save_scan_tick", 0, list, 0);
			break;

		case "save-old":
			/* we were wiping the archive */
			call_out("save_post_tick", 0);
		}

		return;
	}

	path = list_front(list);
	list_pop_front(list);

	size = CONFIGD->file_info(dir + "/" + path)[0];

	if (size == -2) {
		string *names;
		int sz;

		names = CONFIGD->get_dir(dir + "/" + path + "/*")[0];

		for (sz = sizeof(names); --sz >= 0; ) {
			list_push_front(list, path + "/" + names[sz]);
		}
	} else {
		CONFIGD->remove_file(dir + "/" + path);
	}

	call_out("save_purgedir_tick", 0, list, dir);
}

/* parse and print helpers */

object *parse_object(string input)
{
	int on;
	string username;

	if (sscanf(input, "O:%d", on)) {
		return ({ on2ob->query_element(on) });
	} else if (sscanf(input, "U:%s", username)) {
		return ({ TEXT_USERD->find_user(username) });
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
			return "<O:" + oi2on->query_element(oindex) + ">";

		case USR_DIR + "/Text/obj/user":
			return "<U:" + obj->query_name() + ">";

		default:
			error("Unknown object type (" + path + ")");
		}
	} else {
		return ::sprint_object(obj, seen);
	}
}

/* main */

void load_world()
{
	mixed **list;
	mixed *info;

	info = CONFIGD->file_info("save");

	if (!info) {
		error("No save folder");
	}

	if (info[0] != -2) {
		error("Corrupt save file");
	}

	list = ({ nil, nil });

	list_enqueue_directory(list);

	if (list_empty(list)) {
		enqueue_save_directory(list);

		on2ob = new_object("~System/lwo/struct/sparse_array");
		on2op = new_object("~System/lwo/struct/sparse_array");

		call_out("load_scan_tick", 0, list, -1);
	} else {
		call_out("load_purge_tick", 0, list);
	}
}

void save_world()
{
	mixed **list;

	list = ({ nil, nil });

	call_out("save_purgedir_tick", 0, list, "save-tmp");
}
