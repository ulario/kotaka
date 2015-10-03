/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <status.h>
#include <type.h>

object oindex2onum;
object objlist;	/* ({ idnum: obj, data }) */

object dirputqueue;
object objputqueue;

/* public helper functions */

object *parse_object(string input)
{
	int onum;

	if (sscanf(input, "O:%d", onum)) {
		return ({ objlist->query_element(onum - 1)[0] });
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
			return "nil";
		default:
			error("Clone is not a thing (" + path + ")");
		}
	} else {
		return STRINGD->sprint_object(obj, seen);
	}
}

/* private helper functions */

private void put_object(object obj)
{
	int oindex;
	int sz;
	object *inv;

	ASSERT(sscanf(object_name(obj), "%*s#%d", oindex));

	if (oindex2onum->query_element(oindex) != nil) {
		return;
	}

	sz = objlist->query_size();
	oindex2onum->set_element(oindex, sz + 1);
	objlist->push_back( ({ obj, obj->save() }) );

	inv = obj->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		objputqueue->push_back(inv[sz]);
	}
}

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

private void put_directory(string dir)
{
	mapping list;
	int *keys;
	string *names;
	int i, sz;

	list = CATALOGD->list_directory(dir);

	names = map_indices(list);
	keys = map_values(list);

	sz = sizeof(keys);

	for (i = 0; i < sz; i++) {
		if (keys[i] == 2) {
			if (dir) {
				dirputqueue->push_front(dir + ":" + names[i]);
			} else {
				dirputqueue->push_front(names[i]);
			}
		} else {
			if (dir) {
				objputqueue->push_front(CATALOGD->lookup_object(dir + ":" + names[i]));
			} else {
				objputqueue->push_front(CATALOGD->lookup_object(names[i]));
			}
		}
	}
}

static void save_world_put()
{
	catch {
		int ticks;
		int done;

		ticks = status(ST_TICKS);

		while (!done && ticks - status(ST_TICKS) < 10000) {
			if (!objputqueue->empty()) {
				object obj;

				obj = objputqueue->query_front();
				objputqueue->pop_front();

				put_object(obj);
			} else if (!dirputqueue->empty()) {
				string dir;

				dir = dirputqueue->query_front();
				dirputqueue->pop_front();

				put_directory(dir);
			} else {
				done = 1;

				break;
			}
		}

		if (done) {
			CONFIGD->make_dir(".");
			CONFIGD->make_dir("save");

			call_out("save_world_write", 0, objlist->query_size());
		} else {
			call_out("save_world_put", 0);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World save aborted");
		INITD->release_system("saveworld");
	}
}

static void save_world_write(int i)
{
	catch {
		int ticks;

		ticks = status(ST_TICKS);

		while (i > 0 && ticks - status(ST_TICKS) < 10000) {
			i--;

			CONFIGD->write_file("save/" + (i + 1) + ".obj",
				STRINGD->hybrid_sprint(
					objlist->query_element(i)[1]
				) + "\n"
			);
		}

		if (i > 0) {
			call_out("save_world_write", 0, i);
		} else {
			INITD->release_system("saveworld");
			LOGD->post_message("system", LOG_INFO, "World saved");
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World save aborted");
		INITD->release_system("saveworld");
	}
}

void save_world()
{
	int sz;

	oindex2onum = new_object(BIGSTRUCT_MAP_LWO);
	oindex2onum->claim();
	oindex2onum->set_type(T_INT);

	objlist = new_object(BIGSTRUCT_ARRAY_LWO);
	objlist->claim();
	dirputqueue = new_object(BIGSTRUCT_DEQUE_LWO);
	dirputqueue->claim();
	objputqueue = new_object(BIGSTRUCT_DEQUE_LWO);
	objputqueue->claim();

	purge_savedir();

	dirputqueue->push_back(nil);

	INITD->suspend_system("saveworld");
	call_out("save_world_put", 0);
}

static void load_world_purge(int i)
{
	catch {
		int ticks;

		ticks = status(ST_TICKS);

		while (i >= 0 && ticks - status(ST_TICKS) < 10000) {
			object obj;

			if (obj = find_object("~Game/obj/thing#" + i)) {
				destruct_object(obj);
			}

			i--;
		}

		if (i >= 0) {
			call_out("load_world_purge", 0, i);
		} else {
			call_out("load_world_spawn", 0, 1);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
		INITD->release_system("loadworld");
	}
}

static void load_world_spawn(int i)
{
	catch {
		int done;
		int ticks;

		ticks = status(ST_TICKS);

		while (ticks - status(ST_TICKS) < 10000) {
			if (CONFIGD->file_info("save/" + i + ".obj")) {
				object obj;
				int j;

				obj = clone_object("~Game/obj/thing");
				sscanf(object_name(obj), "%*s#%d", j);

				oindex2onum->set_element(j, i);

				objlist->push_back( ({ obj,
					CONFIGD->read_file("save/" + i + ".obj") }) );
				i++;
			} else {
				done = 1;
				break;
			}
		}

		if (done) {
			call_out("load_world_name", 0, i - 1);
		} else {
			call_out("load_world_spawn", 0, i);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
		INITD->release_system("loadworld");
	}
}

static void load_world_name(int i)
{
	catch {
		int ticks;

		ticks = status(ST_TICKS);

		while (i > 0 && ticks - status(ST_TICKS) < 10000) {
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
		}

		if (i > 0) {
			call_out("load_world_name", 0, i);
		} else {
			call_out("load_world_set", 0, objlist->query_size() - 1);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
		INITD->release_system("loadworld");
	}
}

static void load_world_set(int i)
{
	catch {
		int ticks;

		while (i > 0 && ticks - status(ST_TICKS) < 10000) {
			object obj;
			mixed data;

			({ obj, data }) = objlist->query_element(i - 1);

			obj->load(data);
			i--;
		}

		if (i > 0) {
			call_out("load_world_set", 0, i);
		} else {
			LOGD->post_message("system", LOG_INFO, "World loaded");

			INITD->release_system("loadworld");
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
		INITD->release_system("loadworld");
	}
}

void load_world()
{
	int sz;
	int count;

	oindex2onum = new_object(BIGSTRUCT_MAP_LWO);
	oindex2onum->claim();
	oindex2onum->set_type(T_INT);

	objlist = new_object(BIGSTRUCT_ARRAY_LWO);
	objlist->claim();

	INITD->suspend_system("loadworld");

	call_out("load_world_purge", 0, status(ST_OTABSIZE) - 1);
}
