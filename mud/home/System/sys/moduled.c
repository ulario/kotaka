/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kernel/rsrc.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

#define MODULE_BOOT_TICKS 100000000

inherit SECOND_AUTO;
inherit UTILITY_COMPILE;
inherit LIB_SYSTEM;
inherit "~/lib/system/list";

mapping modules;

/* 1: module is online */
/* -1: module is shutting down */

/* declarations */

private void reset_modules_list();

/* creator */

static void create()
{
	modules = ([ ]);

	reset_modules_list();
}

/* helpers */

private void scramble(mixed *arr)
{
	int sz;
	int i;

	sz = sizeof(arr);

	for (i = 0; i < sz; i++) {
		int j;
		mixed tmp;

		j = random(sz);

		tmp = arr[i];
		arr[i] = arr[j];
		arr[j] = tmp;
	}
}

private void freeze_module(string module)
{
	KERNELD->rsrc_set_limit(module, "objects", 0);
}

private void thaw_module(string module)
{
	int sz;
	string *resources;

	resources = KERNELD->query_resources();

	for (sz = sizeof(resources); --sz >= 0; ) {
		KERNELD->rsrc_set_limit(module, resources[sz], -1);
	}
}

private void reset_modules_list()
{
	string *dirs;
	int sz;

	dirs = get_dir(USR_DIR + "/*")[0];
	dirs += ({ nil });

	for (sz = sizeof(dirs) - 1; sz >= 0; --sz) {
		string path;
		string module;

		path = initd_of(dirs[sz]);

		if (find_object(path)) {
			modules[dirs[sz]] = 1;
		}
	}
}

private void send_module_boot_signal(string module)
{
	int sz;
	string *others;

	others = map_indices(modules);
	others -= ({ module });
	scramble(others);

	for (sz = sizeof(others) - 1; sz >= 0; --sz) {
		if (modules[others[sz]] != 1) {
			continue;
		}

		catch {
			find_object(initd_of(others[sz]))
			->booted_module(module);
		}
	}
}

private void send_module_shutdown_signal(string module)
{
	int sz;
	string *others;

	others = map_indices(modules);
	others -= ({ module });
	scramble(others);

	for (sz = sizeof(others) - 1; sz >= 0; --sz) {
		if (modules[others[sz]] != 1) {
			continue;
		}

		catch {
			find_object(initd_of(others[sz]))
			->shutdown_module(module);
		}
	}
}

private mixed **initial_purge_list(string module)
{
	mixed **list;

	list = ({ nil, nil });

	if (module) {
		mixed **dir;

		string *names;
		int *sizes;

		int sz;

		dir = get_dir(USR_DIR + "/" + module + "/*");
		names = dir[0];
		sizes = dir[1];

		for (sz = sizeof(sizes) - 1; sz >= 0; --sz) {
			list_push_back(list, USR_DIR + "/" + module + "/" + names[sz]);
		}
	} else {
		mixed **dir;

		string *names;
		int *sizes;

		int sz;

		/* / */
		dir = get_dir("/*");
		names = dir[0];
		sizes = dir[1];

		for (sz = sizeof(sizes) - 1; sz >= 0; --sz) {
			string name;

			name = names[sz];

			if ("/" + name == USR_DIR) {
				/* skip /home */
				continue;
			}

			if (name == "kernel") {
				/* we can't destruct kernel objects! */
				continue;
			}

			list_push_back(list, "/" + names[sz]);
		}

		/* non-dirs in /home */
		dir = get_dir(USR_DIR + "/*");
		names = dir[0];
		sizes = dir[1];

		for (sz = sizeof(sizes) - 1; sz >= 0; --sz) {
			string name;

			name = names[sz];

			if (sizes[sz] == -2) {
				continue;
			}

			list_push_back(list, USR_DIR + "/" + names[sz]);
		}
	}

	return list;
}

static void purge_module_master_tick(string module, mixed **list, int reboot)
{
	string path;
	mixed *info;

	path = list_front(list);
	list_pop_front(list);

	info = file_info(path);

	if (info[0] == -2) {
		/* subdir */
		mixed **dir;

		string *names;
		int *sizes;

		int sz;

		dir = get_dir(path + "/*");
		names = dir[0];
		sizes = dir[1];

		for (sz = sizeof(sizes) - 1; sz >= 0; --sz) {
			list_push_front(list, path + "/" + names[sz]);
		}
	} else if (info[2]) {
		/* live object */
		catch {
			sscanf(path, "%s.c", path);
			destruct_object(path);
		}
	}

	if (list_empty(list)) {
		call_out("purge_module_tick", 0, module, reboot);
	} else {
		call_out("purge_module_master_tick", 0, module, list, reboot);
	}
}

static void purge_module_tick(string module, int reboot)
{
	object cursor;

	cursor = KERNELD->first_link(module);

	if (cursor) {
		destruct_object(cursor);
		call_out("purge_module_tick", 0, module, reboot);
		return;
	}

	modules[module] = nil;

	LOGD->post_message("system", LOG_NOTICE, "Shutdown " + (module ? module : "Ecru"));

	thaw_module(module);

	if (reboot) {
		call_out("boot_module", 0, module);
	}
}

static void upgrade_module(string module)
{
	if (modules[module] != 1) {
		return;
	}

	rlimits(0; -1) {
		rlimits(0; MODULE_BOOT_TICKS) {
			initd_of(module)->upgrade_module();
		}
	}
}

static void load_module(string module)
{
	load_object(initd_of(module));
}

/* initd hooks */

void prepare_reboot()
{
	int sz;
	string *list;

	ACCESS_CHECK(previous_program() == INITD);

	list = map_indices(modules);
	list -= ({ "System" });
	scramble(list);

	for (sz = sizeof(list) - 1; sz >= 0; --sz) {
		string module;

		module = list[sz];

		if (modules[module] == -1) {
			continue;
		}

		catch {
			initd_of(module)->prepare_reboot();
		}
	}
}

void reboot()
{
	int sz;
	string *list;

	ACCESS_CHECK(previous_program() == INITD);

	list = map_indices(modules);
	list -= ({ "System" });
	scramble(list);

	for (sz = sizeof(list) - 1; sz >= 0; --sz) {
		string module;

		module = list[sz];

		if (modules[module] == -1) {
			continue;
		}

		catch {
			initd_of(module)->reboot();
		}
	}
}

void hotboot()
{
	int sz;
	string *list;

	ACCESS_CHECK(previous_program() == INITD);

	list = map_indices(modules);
	list -= ({ "System" });
	scramble(list);

	for (sz = sizeof(list) - 1; sz >= 0; --sz) {
		string module;

		module = list[sz];

		if (modules[module] == -1) {
			continue;
		}

		catch {
			initd_of(module)->hotboot();
		}
	}
}

void upgrade_modules()
{
	int sz;
	string *list;

	ACCESS_CHECK(SYSTEM());

	list = map_indices(modules);

	scramble(list);

	rlimits(0; -1) {
		for (sz = sizeof(list) - 1; sz >= 0; --sz) {
			string module;

			module = list[sz];

			if (modules[module] == -1) {
				continue;
			}

			rlimits(0; 100000) {
				if (!file_info(initd_of(module) + ".c")) {
					LOGD->post_message("debug", LOG_DEBUG, "Initd missing for module " + module + ", shutting down");
					call_out("shutdown_module", 0, module);
				} else {
					catch {
						LOGD->post_message("debug", LOG_DEBUG, "Recompiling initd for " + (module ? module : "Ecru"));
						compile_object(initd_of(module));

						call_out("upgrade_module", 0, module);
					}
				}
			}
		}
	}
}

/**********/
/* public */
/**********/

string *query_modules()
{
	return map_indices(modules);
}

/* directive, start up a module */
void boot_module(string module)
{
	string *others;
	int sz;

	if (!file_info(initd_of(module) + ".c")) {
		error("No initd for module");
	}

	switch(modules[module]) {
	case -1:
		error("Module is being shut down");

	case 1:
		return;
	}

	if (module && !sizeof(KERNELD->query_users() & ({ module }))) {
		KERNELD->add_user(module);
	}

	if (!sizeof(KERNELD->query_owners() & ({ module }))) {
		KERNELD->add_owner(module);
	}

	rlimits(0; -1) {
		rlimits(0; MODULE_BOOT_TICKS) {
			call_limited("load_module", module);
		}
	}

	modules[module] = 1;

	if (module && !sizeof(KERNELD->query_global_access() & ({ module }))) {
		call_out("shutdown_module", 0, module);

		error("Failure to grant global access by " + module);
	}

	LOGD->post_message("system", LOG_NOTICE, "Booted " + (module ? module : "Ecru"));

	send_module_boot_signal(module);
}

void reboot_module(string module)
{
	object cursor;
	mixed **list;

	ACCESS_CHECK(INTERFACE() || KADMIN() || module == DRIVER->creator(object_name(previous_object())));

	switch(module) {
	case "Bigstruct":
	case "System":
		error("Cannot reboot " + module);
	}

	freeze_module(module);
	modules[module] = -1;

	LOGD->post_message("system", LOG_NOTICE, "Shutting down " + (module ? module : "Ecru"));

	send_module_shutdown_signal(module);

	list = initial_purge_list(module);

	if (list_empty(list)) {
		call_out("purge_module_tick", 0, module, nil, 1);
	} else {
		call_out("purge_module_master_tick", 0, module, list, 1);
	}
}

void shutdown_module(string module)
{
	object cursor;
	mixed **list;

	ACCESS_CHECK(KERNEL() || SYSTEM() || INTERFACE() || KADMIN() || module == DRIVER->creator(object_name(previous_object())));

	switch(module) {
	case "Bigstruct":
	case "String":
	case "System":
		error("Cannot shutdown " + module);
	}

	freeze_module(module);
	modules[module] = -1;

	LOGD->post_message("system", LOG_NOTICE, "Shutting down " + (module ? module : "Ecru"));

	send_module_shutdown_signal(module);

	list = initial_purge_list(module);

	if (list_empty(list)) {
		call_out("purge_module_tick", 0, module, nil, 0);
	} else {
		call_out("purge_module_master_tick", 0, module, list, 0);
	}
}
