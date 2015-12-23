/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2000, 2002, 2009, 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>
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

void console_post(string str, int level);
void message(string str);

private void configure_klib();
void configure_logging();
void configure_rsrc();
private void check_config();
private void check_versions();

private void load()
{
	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("sys");
}

private void set_limits()
{
	KERNELD->rsrc_set_limit("System", "ticks", 250000);
}

static void create()
{
	check_config();
	check_versions();

	catch {
		DRIVER->fix_filequota();

		load_object(LOGD);		/* we need to log any error messages */
		load_object(OBJECTD);		/* enforces static invariants */
		load_object(TLSD);		/* depends on an updated tls size */

		load_object(SYSTEM_USERD);	/* prevents default logins, suspends connections */
		load_object(CALLOUTD);		/* suspends callouts */
		load_object(SUSPENDD);		/* suspends system */

		remove_file("/log/session.log");

		call_out("boot", 0);
	} : {
		LOGD->flush();
		shutdown();
		error("Failed to load system core");
	}
}

private void boot_error()
{
	LOGD->post_message("system", LOG_ERROR, "Runtime error during boot");
	LOGD->post_message("system", LOG_ERROR, TLSD->query_tls_value("System", "error-string"));
	LOGD->post_message("system", LOG_ERROR, ERRORD->print_stack(TLSD->query_tls_value("System", "error-trace")));
}

static void boot()
{
	catch {
		LOGD->post_message("system", LOG_INFO, "------------------");
		LOGD->post_message("system", LOG_INFO, "System core loaded");
		LOGD->post_message("system", LOG_INFO, "------------------");

		load_object(KERNELD);

		KERNELD->set_global_access("System", 1);

		set_limits();

		configure_klib();
		configure_rsrc();
		configure_logging();

		load_object(MODULED);

		MODULED->boot_module("Bigstruct");

		load();

		call_out("boot_2", 0);
	} : {
		boot_error();
		LOGD->flush();
		shutdown();
		error("Failed to load system");
	}
}

static void boot_2()
{
	catch {
		LOGD->post_message("system", LOG_INFO, "-------------");
		LOGD->post_message("system", LOG_INFO, "System loaded");
		LOGD->post_message("system", LOG_INFO, "-------------");

		OBJECTD->discover_objects();
		CLONED->discover_clones();

		call_out("boot_3", 0);
	} : {
		boot_error();
		LOGD->flush();
		shutdown();
		error("Failed to initialize the program database");
	}
}

static void boot_3()
{
	catch {
		LOGD->post_message("system", LOG_INFO, "-----------------");
		LOGD->post_message("system", LOG_INFO, "System discovered");
		LOGD->post_message("system", LOG_INFO, "-----------------");

		MODULED->boot_module("Game");
	} : {
		boot_error();
		LOGD->flush();
		shutdown();
		error("System setup failed");
	}
}

int booting()
{
	mixed *frame;

	frame = call_trace()[0];

	return frame[TRACE_PROGNAME] == DRIVER
		&& frame[TRACE_FUNCTION] == "initialize";
}

int restoring()
{
	mixed *frame;

	frame = call_trace()[0];

	return frame[TRACE_PROGNAME] == DRIVER
		&& frame[TRACE_FUNCTION] == "restored";
}

void clear_admin()
{
	/* remove all admin quotas */
	string *resources;
	int index;

	resources = KERNELD->query_resources();

	for (index = 0; index < sizeof(resources); index++) {
		KERNELD->rsrc_set_limit("admin", resources[index],
			-1);
	}
}

/* kernel hooks */

void prepare_reboot()
{
	ACCESS_CHECK(KERNEL());

	MODULED->prepare_reboot_modules();
	ACCESSD->save();
}

void reboot()
{
	ACCESS_CHECK(KERNEL());

	check_config();
	check_versions();

	clear_admin();
	configure_rsrc();
	configure_logging();

	SYSTEM_USERD->reboot();
	CALLOUTD->reboot();
	ACCESSD->restore();
	DUMPD->reboot();

	DRIVER->fix_filequota();
	MODULED->reboot_modules();
}

void hotboot()
{
	ACCESS_CHECK(KERNEL());

	check_config();
	check_versions();

	clear_admin();
	configure_rsrc();
	configure_logging();

	SYSTEM_USERD->hotboot();
	CALLOUTD->reboot();

	MODULED->hotboot_modules();
	DRIVER->fix_filequota();
}

/* miscellaneous */

private void configure_klib()
{
	string *wizards;
	int index;

	KERNELD->add_owner("Secret");

	wizards = KERNELD->query_users();

	for (index = 0; index < sizeof(wizards); index++) {
		KERNELD->add_owner(wizards[index]);
	}
}

void configure_rsrc()
{
	KERNELD->set_rsrc("stack", 50, 0, 0);
	KERNELD->set_rsrc("ticks", 250000, 0, 0);
	KERNELD->set_rsrc("tick usage", -1, 1, 60);
}

void configure_logging()
{
	/* wipe the slate clean */
	LOGD->clear_targets();

	/* log to console by default */
	LOGD->set_target("*", 127, "driver");

	/* don't log these to the console */
	LOGD->set_target("debug", 0, "driver");
	LOGD->set_target("compile", 63, "driver");
	/* LOGD->set_target("trace", 0, "driver"); */

	/* prevent default logging */
	LOGD->set_target("debug", 255, "null");
	LOGD->set_target("compile", 255, "null");
	LOGD->set_target("trace", 255, "null");

	/* general log gets everything */
	LOGD->set_target("*", 255, "file:/log/general.log");
	LOGD->set_target("debug", 255, "file:/log/general.log");
	LOGD->set_target("compile", 255, "file:/log/general.log");
	LOGD->set_target("trace", 63, "file:/log/general.log");

	/* error log gets errors and traces */
	LOGD->set_target("error", 255, "file:/log/error.log");
	LOGD->set_target("trace", 255, "file:/log/error.log");
	LOGD->set_target("compile", 63, "file:/log/error.log");

	/* session log gets only non debug */
	LOGD->set_target("*", 127, "file:/log/session.log");
	LOGD->set_target("debug", 0, "file:/log/session.log");

	/* debug log gets only debug */
	LOGD->set_target("*", 128, "file:/log/debug.log");
	LOGD->set_target("debug", 255, "file:/log/debug.log");

	/* general system log goes to general and logged in staff */
	LOGD->set_target("system", 255, "file:/log/general.log");
	LOGD->set_target("system", 255, "file:/log/session.log");

	/* post these on the system channel */
	LOGD->set_target("system", 255, "channel:system");

	LOGD->set_target("compile", 255, "channel:compile");
	LOGD->set_target("error", 255, "channel:error");
	LOGD->set_target("trace", 255, "channel:trace");
	LOGD->set_target("debug", 255, "channel:debug");
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

	if (!creator) {
		return 1;
	}

	if (creator == "System") {
		return 0;
	}

	if (sscanf(path, USR_DIR + "/System/closed/%*s")) {
		return 1;
	}

	firstchar = creator[0];

	if (firstchar < 'A' || firstchar > 'Z') {
		return 1;
	}

	return 0;
}

private void check_config()
{
	if (status(ST_ARRAYSIZE) < 256) {
		error("Array size setting is too small (minimum 256 elements)");
	}

	if (status(ST_UTABSIZE) < 3) {
		/* 1: reserved for regular logins */
		/* 2: reserved for emergency logins */
		/* 3: reserved for overflow burning */
		error("Not enough user slots (minimum is 3)");
	}

	if (status(ST_STRSIZE) < 1024) {
		error("String size limit too small (minimum 1024 bytes)");
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

	if (minor < 4) {
		error("DGD minor version " + major + "." + minor + " too low for this version of kotaka");
	}

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
}

string query_version()
{
	return KOTAKA_VERSION;
}

void upgrade_system()
{
	string *safe_versions;
	string *users;
	int sz;

	safe_versions = explode(read_file("~/data/safe_upgrade_versions"), "\n");

	for (sz = sizeof(safe_versions) - 1; sz >= 0; --sz) {
		if (safe_versions[sz] == KOTAKA_VERSION) {
			break;
		}
	}

	if (sz == -1) {
		error("Cannot safely upgrade from version " + KOTAKA_VERSION);
	}

	SUSPENDD->suspend_system();

	compile_object("initd");

	SUSPENDD->queue_work("upgrade_system_2");
}

void upgrade_system_2()
{
	mixed **dir;
	string *names;
	object *objs;
	int sz;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	dir = get_dir("sys/upgrade/*");
	names = dir[0];
	objs = dir[3];

	for (sz = sizeof(names) - 1; sz >= 0; --sz) {
		if (objs[sz]) {
			destruct_object(objs[sz]);
		}
	}

	compile_object("sys/upgrade/1");
}

void upgrade_system_3()
{
	ACCESS_CHECK(SYSTEM());

	load();

	configure_rsrc();
	set_limits();

	purge_orphans("System");

	MODULED->upgrade_modules();
}