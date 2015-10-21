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
#include <kernel/rsrc.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;
inherit UTILITY_COMPILE;

mapping modules;

/* 1: module is online */
/* -1: module is shutting down */

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

private void freeze_module(string module)
{
	KERNELD->rsrc_set_limit(module, "objects", 0);
	KERNELD->rsrc_set_limit(module, "callouts", 0);
	KERNELD->rsrc_set_limit(module, "stack", 0);
	KERNELD->rsrc_set_limit(module, "ticks", 0);
}

private mapping save_module_limits(string module)
{
	mapping limits;

	limits = ([ ]);

	limits["objects"] = KERNELD->rsrc_get(module, "objects")[RSRC_MAX];
	limits["callouts"] = KERNELD->rsrc_get(module, "callouts")[RSRC_MAX];
	limits["stack"] = KERNELD->rsrc_get(module, "stack")[RSRC_MAX];
	limits["ticks"] = KERNELD->rsrc_get(module, "ticks")[RSRC_MAX];

	return limits;
}

private void thaw_module(string module, mapping limits)
{
	KERNELD->rsrc_set_limit(module, "objects", limits["objects"]);
	KERNELD->rsrc_set_limit(module, "callouts", limits["callouts"]);
	KERNELD->rsrc_set_limit(module, "stack", limits["stack"]);
	KERNELD->rsrc_set_limit(module, "ticks", limits["ticks"]);
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

	ACCESS_CHECK(previous_program() == INITD);

	list = map_indices(modules);
	list -= ({ "System" });

	scramble(list);

	rlimits(0; -1) {
		for (sz = sizeof(list) - 1; sz >= 0; --sz) {
			string module;

			module = list[sz];

			rlimits(0; 100000) {
				if (!file_info(USR_DIR + "/" + module + "/initd.c")) {
					call_out("shutdown_module", 0, module);
				} else {
					catch {
						LOGD->post_message("debug", LOG_DEBUG, "Recompiling initd for " + module);
						compile_object(USR_DIR + "/" + module + "/initd");

						SUSPENDD->queue_work("upgrade_module", module);
					}
				}
			}
		}
	}
}

void upgrade_module(string module)
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

	rlimits(0; -1) {
		rlimits(0; 100000000) {
			(USR_DIR + "/" + module + "/initd")->upgrade_module();
		}
	}
}

private void wipe_module(string module)
{
	string *rsrc_names;
	int sz;
	int clear;

	clear = 1;

	rsrc_names = KERNELD->query_resources();

	for (sz = sizeof(rsrc_names); --sz >= 0; ) {
		mixed *rsrc;
		string rsrc_name;

		rsrc_name = rsrc_names[sz];

		KERNELD->rsrc_set_limit(module, rsrc_name, -1);

		if (KERNELD->rsrc_get(module, rsrc_name)
			[RSRC_USAGE]) {
			clear = 0;
		}
	}

	if (clear) {
		KERNELD->remove_owner(module);
	}
}

static void purge_module_tick(string module, int reboot, mapping limits)
{
	int done;
	int ticks;

	ticks = status(ST_TICKS);

	while (ticks - status(ST_TICKS) < 10000) {
		object cursor;

		cursor = KERNELD->first_link(module);

		if (cursor) {
			destruct_object(cursor);
		} else {
			done = 1;
			break;
		}
	}

	if (!done) {
		call_out("purge_module_tick", 0, module, reboot, limits);
		return;
	}

	modules[module] = nil;
	LOGD->post_message("system", LOG_NOTICE, "Shutdown " + module);

	wipe_module(module);
	thaw_module(module, limits);

	if (reboot) {
		call_out("boot_module", 0, module);
	}
}

string *query_modules()
{
	return map_indices(modules);
}

/* commands */

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
			find_object(USR_DIR + "/" + others[sz] + "/initd")
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
			find_object(USR_DIR + "/" + others[sz] + "/initd")
			->shutdown_module(module);
		}
	}
}

static void load_module(string module)
{
	load_object(USR_DIR + "/" + module + "/initd");
}

void boot_module(string module)
{
	string *others;
	int sz;

	if (!file_info(USR_DIR + "/" + module + "/initd.c")) {
		error("No initd for module");
	}

	switch(modules[module]) {
	case -1:
		error("Module is being shut down");

	case 1:
		return;
	}

	if (!sizeof(KERNELD->query_users() & ({ module }))) {
		KERNELD->add_user(module);
	}

	if (!sizeof(KERNELD->query_owners() & ({ module }))) {
		KERNELD->add_owner(module);
	}

	rlimits(0; 100000000) {
		call_limited("load_module", module);
	}

	if (!sizeof(KERNELD->query_global_access() & ({ module }))) {
		mapping limits;
		modules[module] = -1;
		limits = save_module_limits(module);
		freeze_module(module);
		call_out("purge_module_tick", 0, module, 0, limits);
		error("Failure to grant global access by " + module);
	}

	modules[module] = 1;

	LOGD->post_message("system", LOG_NOTICE, "Booted " + module);

	send_module_boot_signal(module);
}

void shutdown_module(string module)
{
	object cursor;
	mapping limits;

	ACCESS_CHECK(KERNEL() || SYSTEM() || INTERFACE() || KADMIN() || module == DRIVER->creator(object_name(previous_object())));

	switch(module) {
	case "Bigstruct":
	case "String":
	case "System":
	case "Utility":
		error("Cannot shutdown " + module);
	}

	limits = save_module_limits(module);
	freeze_module(module);
	modules[module] = -1;

	LOGD->post_message("system", LOG_NOTICE, "Shutting down " + module);

	send_module_shutdown_signal(module);

	call_out("purge_module_tick", 0, module, 0, limits);
}

void reboot_module(string module)
{
	object cursor;
	mapping limits;

	ACCESS_CHECK(INTERFACE() || KADMIN() || module == DRIVER->creator(object_name(previous_object())));

	switch(module) {
	case "Bigstruct":
	case "System":
		error("Cannot reboot " + module);
	}

	limits = save_module_limits(module);
	freeze_module(module);
	modules[module] = -1;

	LOGD->post_message("system", LOG_NOTICE, "Shutting down " + module);

	send_module_shutdown_signal(module);

	call_out("purge_module_tick", 0, module, 1, limits);
}

/* signals */

void prepare_reboot_modules()
{
	int sz;
	string *list;

	ACCESS_CHECK(previous_program() == INITD);

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

	ACCESS_CHECK(previous_program() == INITD);

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

	ACCESS_CHECK(previous_program() == INITD);

	list = map_indices(modules);
	list -= ({ "System" });
	scramble(list);

	for (sz = sizeof(list) - 1; sz >= 0; --sz) {
		catch {
			(USR_DIR + "/" + list[sz] + "/initd")->hotboot();
		}
	}
}
