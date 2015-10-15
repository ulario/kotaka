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
#include <kotaka/privilege.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <game/paths.h>
#include <status.h>
#include <type.h>

object oindex2onum;
object objlist;	/* ({ idnum: obj, data }) */

object dirputqueue;
object objputqueue;

/* private helper functions */

/* - loading */

private void purge_object(object obj)
{
	int sz;
	object *inv;
	string name;

	inv = obj->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		objputqueue->push_back(inv[sz]);
	}

	name = obj->query_object_name();

	destruct_object(obj);
}

private void purge_directory(string dir)
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

void load_world_purge()
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

	catch {
		int ticks;
		int done;

		ticks = status(ST_TICKS);

		while (!done && ticks - status(ST_TICKS) < 10000) {
			if (!dirputqueue->empty()) {
				string dir;

				dir = dirputqueue->query_front();
				dirputqueue->pop_front();

				purge_directory(dir);
			} else if (!objputqueue->empty()) {
				object obj;

				obj = objputqueue->query_front();
				objputqueue->pop_front();

				if (obj) {
					purge_object(obj);
				}
			} else {
				done = 1;

				break;
			}
		}

		if (done) {
			SUSPENDD->queue_work("load_world_spawn", 1);
		} else {
			SUSPENDD->queue_work("load_world_purge");
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
	}
}

void load_world_spawn(int i)
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

	catch {
		int done;
		int ticks;

		ticks = status(ST_TICKS);

		while (ticks - status(ST_TICKS) < 10000) {
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
				break;
			}
		}

		if (done) {
			SUSPENDD->queue_work("load_world_name", i - 1);
		} else {
			SUSPENDD->queue_work("load_world_spawn", i);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
	}
}

void load_world_name(int i)
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

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
			SUSPENDD->queue_work("load_world_name", i);
		} else {
			SUSPENDD->queue_work("load_world_set", objlist->query_size() - 1);
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
	}
}

void load_world_set(int i)
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

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
			SUSPENDD->queue_work("load_world_set", i);
		} else {
			LOGD->post_message("system", LOG_INFO, "World loaded");
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World load aborted");
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

void save_world_put()
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

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
			SUSPENDD->queue_work("save_world_write", objlist->query_size());
		} else {
			SUSPENDD->queue_work("save_world_put");
		}
	} : {
		LOGD->post_message("system", LOG_INFO, "World save aborted");
	}
}

void save_world_write(int i)
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

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
			SUSPENDD->queue_work("save_world_write", i);
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

/* public functions */

void load_world()
{
	int sz;
	int count;

	oindex2onum = new_object(BIGSTRUCT_MAP_LWO);
	oindex2onum->claim();
	oindex2onum->set_type(T_INT);

	objlist = new_object(BIGSTRUCT_ARRAY_LWO);
	objlist->claim();

	dirputqueue = new_object(BIGSTRUCT_DEQUE_LWO);
	dirputqueue->claim();
	objputqueue = new_object(BIGSTRUCT_DEQUE_LWO);
	objputqueue->claim();

	dirputqueue->push_back(nil);

	SUSPENDD->suspend_system();
	SUSPENDD->queue_work("load_world_purge");
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

	SUSPENDD->suspend_system();
	SUSPENDD->queue_work("save_world_put");
}
