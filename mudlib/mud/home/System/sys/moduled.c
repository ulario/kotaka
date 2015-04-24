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
#include <kernel/kernel.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;
inherit UTILITY_COMPILE;

mapping modules;

private void reset_modules_list();

static void create()
{
	modules = ([ ]);

	reset_modules_list();
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

private void reset_modules_list()
{
	string *dirs;
	int sz;

	dirs = get_dir(USR_DIR + "/*")[0];
	dirs -= ({ "System" });

	for (sz = sizeof(dirs) - 1; sz >= 0; --sz) {
		string path;
		string module;

		path = USR_DIR + "/" + dirs[sz] + "/initd";

		if (find_object(path)) {
			modules[dirs[sz]] = 1;
		}
	}
}

void upgrade_modules()
{
	int sz;
	string *list;

	ACCESS_CHECK(SYSTEM());

	list = map_indices(modules);
	list -= ({ "System" });

	scramble(list);

	for (sz = sizeof(list) - 1; sz >= 0; --sz) {
		call_out("recompile_initd", 0, list[sz]);
	}
}

static void recompile_initd(string module)
{
	compile_object(USR_DIR + "/" + module + "/initd");
	call_out("upgrade_module", 0, module);
}

static void upgrade_module(string module)
{
	(USR_DIR + "/" + module + "/initd")->upgrade_module();
}

void add_module(string module)
{
	string *others;
	int sz;

	ACCESS_CHECK(SYSTEM());

	if (!find_object(USR_DIR + "/" + module + "/initd")) {
		error("Module initd not loaded");
	}

	if (!sizeof(KERNELD->query_global_access() & ({ module }))) {
		error("Failure to grant global access by " + module);
	}

	if (modules[module]) {
		error("Module already ready");
	}

	others = map_indices(modules);
	scramble(others);

	modules[module] = 1;

	for (sz = sizeof(others) - 1; sz >= 0; --sz) {
		catch {
			find_object(USR_DIR + "/" + others[sz] + "/initd")
			->booted_module(module);
		}
	}
}

void remove_module(string module)
{
	int sz;
	string *others;

	ACCESS_CHECK(SYSTEM());

	KERNELD->remove_user(module);

	modules[module] = nil;

	others = map_indices(modules);
	scramble(others);

	for (sz = sizeof(others) - 1; sz >= 0; --sz) {
		catch {
			find_object(USR_DIR + "/" + others[sz] + "/initd")
			->shutdown_module(module);
		}
	}
}

static void purge_module_tick(string module, varargs int reboot)
{
	int done;

	rlimits(0; 200000) {
		while (status(ST_TICKS) > 20000) {
			object cursor;

			cursor = KERNELD->first_link(module);

			if (cursor) {
				destruct_object(cursor);
			} else {
				done = 1;
				break;
			}
		}
	}

	if (!done) {
		call_out("purge_module_tick", 0, module, reboot);
		return;
	}

	LOGD->post_message("system", LOG_NOTICE, "Shutdown " + module);

	if (reboot) {
		catch {
			KERNELD->rsrc_set_limit(module, "objects", -1);
		}

		catch {
			KERNELD->rsrc_set_limit(module, "callouts", -1);
		}

		call_out("boot_module", 0, module);
	}
}

string *query_modules()
{
	return map_indices(modules);
}

/* commands */

private void deprovision_module(string module)
{
	KERNELD->rsrc_set_limit(module, "objects", 0);
	KERNELD->rsrc_set_limit(module, "callouts", 0);
	KERNELD->rsrc_set_limit(module, "stack", 0);
	KERNELD->rsrc_set_limit(module, "ticks", 0);
}

private void provision_module(string module)
{
	KERNELD->rsrc_set_limit(module, "objects", -1);
	KERNELD->rsrc_set_limit(module, "callouts", -1);
	KERNELD->rsrc_set_limit(module, "stack", -1);
	KERNELD->rsrc_set_limit(module, "ticks", -1);
}

void boot_module(string module)
{
	if (!file_info(USR_DIR + "/" + module + "/initd.c")) {
		error("No initd for module");
	}

	if (modules[module]) {
		/* module already loaded */
		return;
	}

	KERNELD->add_user(module);
	KERNELD->add_owner(module);

	provision_module(module);

	rlimits(100; -1) {
		load_object(USR_DIR + "/" + module + "/initd");
	}

	add_module(module);

	LOGD->post_message("system", LOG_NOTICE, "Booted " + module);
}

void shutdown_module(string module)
{
	object cursor;

	ACCESS_CHECK(INTERFACE() || KADMIN() || module == DRIVER->creator(object_name(previous_object())));

	switch(module) {
	case "Algorithm":
	case "Bigstruct":
	case "String":
	case "System":
		error("Cannot shutdown " + module);
	}

	deprovision_module(module);
	remove_module(module);

	LOGD->post_message("system", LOG_NOTICE, "Shutting down " + module);

	call_out("purge_module_tick", 0, module);
}

void reboot_module(string module)
{
	object cursor;

	ACCESS_CHECK(INTERFACE() || KADMIN() || module == DRIVER->creator(object_name(previous_object())));

	switch(module) {
	case "Bigstruct":
	case "System":
		error("Cannot reboot " + module);
	}

	deprovision_module(module);
	remove_module(module);

	LOGD->post_message("system", LOG_NOTICE, "Shutting down " + module);

	call_out("purge_module_tick", 0, module, 1);
}

/* signals */

void prepare_reboot_modules()
{
	int sz;
	string *list;

	ACCESS_CHECK(SYSTEM());

	list = map_indices(modules);
	list -= ({ "System" });
	scramble(list);

	for (sz = sizeof(list) - 1; sz >= 0; --sz) {
		catch {
			(USR_DIR + "/" + list[sz] + "/initd")->prepare_reboot();
		}
	}
}

void reboot_modules()
{
	int sz;
	string *list;

	ACCESS_CHECK(SYSTEM());

	list = map_indices(modules);
	list -= ({ "System" });
	scramble(list);

	for (sz = sizeof(list) - 1; sz >= 0; --sz) {
		catch {
			(USR_DIR + "/" + list[sz] + "/initd")->reboot();
		}
	}
}

void hotboot_modules()
{
	int sz;
	string *list;

	ACCESS_CHECK(SYSTEM());

	list = map_indices(modules);
	list -= ({ "System" });
	scramble(list);

	for (sz = sizeof(list) - 1; sz >= 0; --sz) {
		catch {
			(USR_DIR + "/" + list[sz] + "/initd")->hotboot();
		}
	}
}
