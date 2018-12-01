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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kernel/version.h>
#include <kotaka/log.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/version.h>
#include <status.h>
#include <trace.h>
#include <type.h>

#define CONSOLE_LOGLEVEL 6

inherit SECOND_AUTO;
inherit LIB_INITD;
inherit UTILITY_COMPILE;

int version_major;
int version_minor;
int version_patch;

mixed **tasks;

void console_post(string str, int level);
void message(string str);

private void load()
{
	load_dir("lwo");
	load_dir("obj");
	load_dir("sys");
}

private void set_limits()
{
	reset_limits();
}

private void configure_klib()
{
	string *wizards;
	int sz;

	KERNELD->add_owner("Secret");

	wizards = KERNELD->query_users();

	for (sz = sizeof(wizards); --sz >= 0; ) {
		KERNELD->add_owner(wizards[sz]);
	}
}

private void configure_rsrc()
{
	KERNELD->set_rsrc("stack", 50, 0, 0);
	KERNELD->set_rsrc("ticks", 250000, 0, 0);
	KERNELD->set_rsrc("tick usage", -1, 1, 1);

	KERNELD->set_rsrc("callout peak", -1, 0, 0);
	KERNELD->set_rsrc("callout usage", -1, 1, 1);
}

private void clear_admin()
{
	string *resources;
	int sz;

	resources = KERNELD->query_resources();

	for (sz = sizeof(resources); --sz >= 0; ) {
		KERNELD->rsrc_set_limit("admin", resources[sz], -1);
	}
}

private void log_boot_error()
{
	LOGD->post_message("system", LOG_ERR, "Runtime error during boot");
	LOGD->post_message("system", LOG_ERR, TLSD->query_tls_value("System", "error-string"));
	LOGD->post_message("system", LOG_ERR, ERRORD->print_stack(TLSD->query_tls_value("System", "error-trace")));
}

private void check_config()
{
	if (status(ST_ARRAYSIZE) < 256) {
		error("Array size too small (minimum 256)");
	}

	if (status(ST_COTABSIZE) < 100) {
		error("Callout table too small (minimum 100)");
	}

	if (status(ST_UTABSIZE) < 3) {
		/* 1: reserved for regular logins */
		/* 2: reserved for emergency logins */
		/* 3: reserved for overflow burning */
		error("User table too small (minimum 3)");
	}

	if (status(ST_STRSIZE) < 4096) {
		error("String size too small (minimum 4096)");
	}
}

/* shamelessly stolen from phantasmal, kudos to Noah Gibbs */
private void check_versions()
{
	string dgdstatus;
	int major, minor, patch;

	dgdstatus = status(ST_VERSION);

	if (sscanf(dgdstatus, "DGD %d.%d.%d", major, minor, patch) != 3) {
		patch = 0;
		if (sscanf(dgdstatus, "DGD %d.%d", major, minor) != 2) {
			minor = 0;
			if(sscanf(dgdstatus, "DGD %d", major) != 1) {
				error("Cannot parse DGD driver version");
			}
		}
	}

	if (major < 1) {
		error("DGD major version " + major + " too low for this version of kotaka");
	}

	if (major >= 2) {
		error("DGD major version " + major + " too high for this version of kotaka");
	}

	if (minor < 6) {
		error("DGD minor version " + major + "." + minor + " too low for this version of kotaka");
	}

	DRIVER->message("Kernel library version: " + KERNEL_LIB_VERSION + "\n");

	if (sscanf(KERNEL_LIB_VERSION, "%d.%d.%d", major, minor, patch) != 3) {
		patch = 0;
		if (sscanf(KERNEL_LIB_VERSION, "%d.%d", major, minor) != 2) {
			minor = 0;
			if(sscanf(KERNEL_LIB_VERSION, "%d", major) != 1) {
				error("Cannot parse kernel library version");
			}
		}
	}

	if (major < 1) {
		error("Kernel library major version " + major + " too low for this version of kotaka");
	}

	if (major >= 2) {
		error("Kernel library major version " + major + " too high for this version of kotaka");
	}

	if (minor < 3) {
		error("Kernel library minor version " + major + "." + minor + " too low for this version of kotaka");
	}

	if (minor == 3 && patch < 4) {
		error("Kernel library patch version " + major + "." + minor + "." + patch + " too low for this version of kotaka");
	}
}

private void set_version()
{
	if (sscanf(KOTAKA_VERSION, "%d.%d.%d", version_major, version_minor, version_patch) != 3) {
		version_patch = 0;

		if (sscanf(KOTAKA_VERSION, "%d.%d", version_major, version_minor) != 2) {
			version_minor = 0;

			if(sscanf(KOTAKA_VERSION, "%d", version_major) != 1) {
				error("Cannot parse Kotaka version");
			}
		}
	}
}

void configure_logging()
{
	ACCESS_CHECK(SYSTEM());

	LOGD->clear_targets();

	LOGD->set_target("*", 63, "driver");

	LOGD->set_target("debug", 0, "driver");
	LOGD->set_target("compile", 255, "driver");
	LOGD->set_target("error", 255, "driver");
	LOGD->set_target("trace", 255, "driver");

	LOGD->set_target("debug", 255, "null");
	LOGD->set_target("compile", 255, "null");
	LOGD->set_target("trace", 255, "null");

	LOGD->set_target("*", 255, "file:general");

	LOGD->set_target("error", 255, "file:error");
	LOGD->set_target("trace", 255, "file:error");
	LOGD->set_target("compile", 255, "file:error");

	LOGD->set_target("*", 127, "file:session");
	LOGD->set_target("debug", 0, "file:session");

	LOGD->set_target("*", 128, "file:debug");
	LOGD->set_target("debug", 255, "file:debug");

	LOGD->set_target("system", 255, "file:general");
	LOGD->set_target("system", 255, "file:session");

	LOGD->set_target("system", 63, "channel:system");

	LOGD->set_target("compile", 255, "channel:compile");
	LOGD->set_target("error", 255, "channel:error");
	LOGD->set_target("trace", 255, "channel:trace");
	LOGD->set_target("debug", 255, "channel:debug");
}

private void reboot_common()
{
	check_config();
	check_versions();

	LOGD->post_message("debug", LOG_NOTICE, "Auditing filequota");
	DRIVER->fix_filequota();

	PATCHD->reboot();
	DUMPD->reboot();
}

private void upgrade_check_kotaka_version()
{
	if (version_major) {
		error("Must upgrade from version 0.55 or 0.56");
	}

	switch(version_minor) {
	case 55:
	case 56:
		switch(version_patch) {
		case 0:
			break;

		default:
			error("Must upgrade from version 0.55 or 0.56");
		}
		break;

	default:
		error("Must upgrade from version 0.55 or 0.56");
	}
}

static void create()
{
	check_config();
	check_versions();
	set_version();

	catch {
		load_object(KERNELD);		/* needed for LogD */

		KERNELD->set_global_access("System", 1);

		configure_klib();
		clear_admin();
		configure_rsrc();
		set_limits();

		load_object(SECRETD);		/* needed for LogD */
		load_object(LOGD);		/* we need to log any error messages */
		load_object(TLSD);		/* depends on an updated tls size, also needed by ObjectD */
		load_object(SYSTEM_USERD);	/* prevents default logins, suspends connections */

		SECRETD->remove_file("logs/session.log");

		call_out("boot", 0);

		LOGD->post_message("system", LOG_NOTICE, "System core loaded");
	} : {
		LOGD->flush();
		shutdown();
		error("Failed to load system core");
	}
}

static void boot()
{
	catch {
		load_object(ERRORD);		/* depends on TLS */
		load_object(MODULED);

		load_object(PROGRAM_INFO);
		load_object(SPARSE_ARRAY);
		load_object(PATCHD);
		load_object(OBJECTD);		/* depends on TLS */

		rlimits (0; -1) {
			OBJECTD->reset();
		}

		LOGD->post_message("system", LOG_NOTICE, "System discovered");

		load();
		LOGD->post_message("system", LOG_NOTICE, "System loaded");

		DUMPD->set_parameters(3600, 0, 24);

		call_out("ready", 0);
	} : {
		log_boot_error();
		LOGD->flush();
		shutdown();
		error("System setup failed");
	}
}

static void ready()
{
	MODULED->boot_module("Game");
}

static void upgrade_system_post_recompile()
{
	LOGD->post_message("system", LOG_NOTICE, "InitD recompiled for system upgrade");

	/* first, upgrade System */
	/* do NOT bother other modules for upgrades until after System itself is successfully upgraded */

	LOGD->post_message("system", LOG_NOTICE, "Upgrading System module");

	upgrade_check_kotaka_version();
	set_version();

	destruct_dir("/kernel/lib");

	compile_dir("/kernel/obj");
	compile_dir("/kernel/sys");

	LOGD->post_message("system", LOG_NOTICE, "Kernel library rebuilt");

	compile_object(PROGRAM_INFO);
	compile_object(PATCHD);
	compile_object(OBJECTD);

	LOGD->post_message("system", LOG_NOTICE, "Object manager rebuilt");

	call_out("upgrade_system_post_recompile_2", 0);
}

static void upgrade_system_post_recompile_2()
{
	rlimits (0; -1) {
		upgrade_purge();
		upgrade_build();
	}

	LOGD->post_message("system", LOG_NOTICE, "System rebuilt");

	call_out("upgrade_system_post_recompile_3", 0);
}

static void upgrade_system_post_recompile_3()
{
	MODULED->upgrade_modules();
	LOGD->post_message("system", LOG_NOTICE, "Upgrade processing completed");

	call_out("do_upgrade_rebuild", 0);
}

static void do_upgrade_rebuild()
{
	MODULED->upgrade_purge();
	LOGD->post_message("system", LOG_NOTICE, "Upgrade purge completed");

	MODULED->upgrade_build();
	LOGD->post_message("system", LOG_NOTICE, "Upgrade build completed");
}

void prepare_reboot()
{
	ACCESS_CHECK(KERNEL());

	if (TLSD->query_tls_value("System", "incremental-snapshot")) {
		LOGD->post_message("system", LOG_NOTICE, "Incremental snapshot");
	} else {
		LOGD->post_message("system", LOG_NOTICE, "Full snapshot");
	}

	ACCESSD->save();
	MODULED->prepare_reboot();
}

void reboot()
{
	ACCESS_CHECK(KERNEL());

	reboot_common();

	catch {
		SYSTEM_USERD->reboot();
	}
	catch {
		ACCESSD->restore();
	}
	catch {
		MODULED->reboot();
	}
}

void hotboot()
{
	ACCESS_CHECK(KERNEL());

	reboot_common();

	catch {
		SYSTEM_USERD->hotboot();
	}
	catch {
		MODULED->hotboot();
	}
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	clear_admin();
	configure_rsrc();
	set_limits();
	configure_logging();
}

int forbid_inherit(string from, string path, int priv)
{
	string creator;
	int firstchar;

	ACCESS_CHECK(previous_program() == OBJECTD);

	if (sscanf(path, USR_DIR + "/System/lib/auto/%*s")) {
		return 0;
	}

	creator = DRIVER->creator(from);

	if (creator == "System") {
		return 0;
	}

	if (sscanf(path, USR_DIR + "/System/closed/%*s")) {
		return 1;
	}

	return 0;
}

void upgrade_system()
{
	ACCESS_CHECK(VERB());

	purge_dir("lib");

	compile_object(INITD);

	call_out("upgrade_system_post_recompile", 0);
}

void enqueue_task_prefix(string path, string func, mixed args ...)
{
	ACCESS_CHECK(SYSTEM());

	if (!tasks) {
		tasks = ({ nil, nil });
		call_out("_F_dummy", 0);
	}

	list_push_back(tasks, ({ path, func, args }) );
}

void begin_task()
{
	ACCESS_CHECK(SYSTEM());

	if (!tasks) {
		return;
	}

	rlimits (0; -1) {
		mixed **active;

		active = tasks;
		tasks = nil;

		while (!list_empty(active)) {
			string path;
			string func;
			mixed *args;

			({ path, func, args }) = list_front(active);
			list_pop_front(active);

			catch {
				call_other(path, func, args ...);
			}
		}
	}
}

void end_task()
{
	ACCESS_CHECK(SYSTEM());
}
