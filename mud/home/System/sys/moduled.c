/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019, 2020, 2021, 2022  Raymond Jennings
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

#define M    1000000
#define G 1000000000

#define MODULE_BOOT_TICKS G

inherit SECOND_AUTO;
inherit UTILITY_COMPILE;
inherit LIB_SYSTEM;
inherit "~/lib/struct/list";

mapping modules;

private void clear_quota(string module)
{
	if (sizeof( ({ module }) & KERNELD->query_owners())) {
		int sz;
		string *resources;

		resources = KERNELD->query_resources();

		for (sz = sizeof(resources); --sz >= 0; ) {
			KERNELD->rsrc_set_limit(module, resources[sz], -1);
		}
	}
}

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

private void send_module_boot_signal(string module)
{
	int sz;
	string *others;

	others = map_indices(modules);
	others -= ({ module });
	others |= ({ "System" });
	scramble(others);

	for (sz = sizeof(others) - 1; sz >= 0; --sz) {
		if (modules[others[sz]] != 1) {
			continue;
		}

		catch {
			object initd;

			if (initd = find_object(initd_of(others[sz]))) {
				initd->booted_module(module);
			}
		}
	}
}

private void send_module_shutdown_signal(string module)
{
	int sz;
	string *others;

	others = map_indices(modules);
	others -= ({ module });
	others |= ({ "System" });
	scramble(others);

	for (sz = sizeof(others) - 1; sz >= 0; --sz) {
		if (modules[others[sz]] != 1) {
			continue;
		}

		catch {
			object initd;

			if (initd = find_object(initd_of(others[sz]))) {
				initd->shutdown_module(module);
			}
		}
	}
}

private void purge_objects(string module)
{
	rlimits (0; -1) {
		mixed **list;

		list = OBJECTD->query_program_indices();

		while (!list_empty(list)) {
			int index;
			object pinfo;
			string path;

			index = list_front(list);
			list_pop_front(list);

			pinfo = OBJECTD->query_program_info(index);

			if (!pinfo) {
				continue;
			}

			if (pinfo->query_destructed()) {
				continue;
			}

			path = pinfo->query_path();

			if (DRIVER->creator(path) != module) {
				/* not theirs */
				continue;
			}

			destruct_object(path);
		}
	}
}

private void do_module_shutdown(string module, int reboot)
{
	string initd;

	if (module == "System") {
		error("Cannot reboot or shut down System module");
	}

	modules[module] = -1;

	LOGD->post_message("system", LOG_NOTICE, (reboot ? "Rebooting" : "Shutting down") + " " + (module ? module : "Ecru"));

	send_module_shutdown_signal(module);

	initd = initd_of(module);

	/* give it a chance to shut down cleanly first */
	if (find_object(initd)) {
		catch {
			destruct_object(initd);
		}
	}

	if (sizeof( ({ module }) & KERNELD->query_owners())) {
		KERNELD->rsrc_set_limit(module, "objects", 0);
	}

	purge_objects(module);

	call_out("purge_module_tick", 0, module, reboot);
}

static void purge_module_tick(string module, varargs int reboot)
{
	object cursor;

	cursor = KERNELD->first_link(module);

	if (cursor) {
		TLSD->set_tls_value("System", "destruct_force", cursor);

		destruct_object(cursor);

		call_out("purge_module_tick", 0, module, reboot);
	} else {
		modules[module] = nil;

		LOGD->post_message("system", LOG_NOTICE, "Shut down " + (module ? module : "Ecru"));

		clear_quota(module);

		if (reboot) {
			call_out("boot_module", 0, module, 1);
		}
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
	rlimits(0; -1) {
		rlimits(0; MODULE_BOOT_TICKS) {
			load_object(initd_of(module));
		}
	}
}

/***********/
/* creator */
/***********/

static void create()
{
	string *names;
	int sz;
	modules = ([ ]);

	if (find_object("/initd")) {
		LOGD->post_message("system", LOG_WARNING, "ModuleD: Rogue Ecru module discovered");
		modules[nil] = 1;
	}

	names = get_dir(USR_DIR + "/*")[0];

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;

		name = names[sz];

		if (name == "System") {
			continue;
		}

		if (find_object(USR_DIR + "/" + name + "/initd")) {
			LOGD->post_message("system", LOG_WARNING, "ModuleD: Rogue " + name + " module discovered");
			modules[name] = 1;
		}
	}
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	modules["System"] = nil;
}

/*******************/
/* called by INITD */
/*******************/

/* part of system upgrade */

void upgrade_check_modules()
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

		rlimits(0; -1) {
			rlimits(0; 100000) {
				initd_of(module)->upgrade_check();
			}
		}
	}
}

void upgrade_modules()
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

		rlimits(0; -1) {
			rlimits(0; MODULE_BOOT_TICKS) {
				string initd;

				initd = initd_of(module);

				if (!file_info(initd + ".c")) {
					/* loaded module with source removed, shut it down */
					LOGD->post_message("system", LOG_WARNING, "ModuleD: Shutting down " + (module ? module : "Ecru") + ", missing initd.c in upgrade_modules");
					call_out("shutdown_module", 0, module);
				} else {
					catch {
						compile_object(initd);

						call_out("upgrade_module", 0, module);
					}
				}
			}
		}
	}
}

/* instruct all modules to purge */
/* destruct inheritables and objects with removed sources */
void upgrade_purge()
{
	int sz;
	string *list;

	ACCESS_CHECK(previous_program() == INITD);

	list = map_indices(modules);
	list -= ({ "System" });

	scramble(list);

	rlimits(0; -1) {
		for (sz = sizeof(list) - 1; sz >= 0; --sz) {
			string module;

			module = list[sz];

			if (modules[module] == -1) {
				continue;
			}

			rlimits(0; MODULE_BOOT_TICKS) {
				initd_of(module)->upgrade_purge();
			}
		}
	}
}

/* instruct all modules to build */
/* compile all objects, includes recompiling those that already exist */
void upgrade_build()
{
	int sz;
	string *list;

	ACCESS_CHECK(previous_program() == INITD);

	list = map_indices(modules);
	list -= ({ "System" });

	scramble(list);

	rlimits(0; -1) {
		for (sz = sizeof(list) - 1; sz >= 0; --sz) {
			string module;

			module = list[sz];

			if (modules[module] == -1) {
				continue;
			}

			rlimits(0; MODULE_BOOT_TICKS) {
				initd_of(module)->upgrade_build();
			}
		}
	}
}

/* boot hooks */

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

/**********/
/* public */
/**********/

string *query_modules()
{
	return map_indices(modules) - ({ "System" });
}

int query_module(string module)
{
	if (module == "System") {
		return 1;
	}

	if (!modules[module]) {
		return 0;
	}

	return modules[module];
}

/* directive, start up a module */
void boot_module(string module, varargs int reboot)
{
	string *others;
	int existed;
	string creator;

	creator = DRIVER->creator(previous_program());

	if (module == "System") {
		error("Cannot boot System module");
	}

	if (modules[module] == -1) {
		error("Cannot boot module while it is shutting down");
	}

	if (!file_info(initd_of(module) + ".c")) {
		error("No initd for " + (module ? module : "Ecru") + " module");
	}

	existed = !!find_object(initd_of(module));

	if (module && !sizeof(KERNELD->query_users() & ({ module }))) {
		KERNELD->add_user(module);
	}

	if (!sizeof(KERNELD->query_owners() & ({ module }))) {
		KERNELD->add_owner(module);
	}

	if (!existed) {
		string err;

		clear_quota(module);

		err = catch(load_module(module));

		if (err) {
			if (module) {
				error("Error booting module " + module + ": " + err);
			} else {
				error("Error booting nil module: " + err);
			}
		}

		modules[module] = 1;

		if (reboot) {
			LOGD->post_message("system", LOG_NOTICE, "Rebooted " + (module ? module : "Ecru"));
		} else {
			LOGD->post_message("system", LOG_NOTICE, "Booted " + (module ? module : "Ecru"));
		}

		send_module_boot_signal(module);
	}
}

void shutdown_module(string module)
{
	ACCESS_CHECK(KERNEL() || SYSTEM() || INTERFACE() || KADMIN() || module == DRIVER->creator(object_name(previous_object())));

	do_module_shutdown(module, 0);
}

void reboot_module(string module)
{
	ACCESS_CHECK(KERNEL() || SYSTEM() || INTERFACE() || KADMIN() || module == DRIVER->creator(object_name(previous_object())));

	do_module_shutdown(module, 1);
}
