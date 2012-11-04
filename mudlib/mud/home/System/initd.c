/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kernel/rsrc.h>
#include <kernel/tls.h>
#include <kernel/user.h>
#include <kernel/version.h>

#include <kotaka/assert.h>
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/version.h>

#include <status.h>
#include <limits.h>
#include <trace.h>
#include <type.h>

#define CONSOLE_LOGLEVEL 6

inherit SECOND_AUTO;
inherit LIB_INITD;
inherit UTILITY_COMPILE;

int dumped;
string *subsystems;

void console_post(string str, int level);
void message(string str);
void boot_subsystem(string subsystem);

private void configure_klib();
private void configure_logging();
private void check_config();
private void check_versions();

private void initialize()
{
	subsystems = ({ "System" });

	load_dir("lib/bigstruct", 1);
	load_dir("lwo/bigstruct", 1);
	load_dir("obj/bigstruct", 1);

	configure_logging();

	LOGD->post_message("boot", LOG_NOTICE,
		"-----------------------------------------------------");
	LOGD->post_message("boot", LOG_NOTICE,
		"Kotaka mudlib " + KOTAKA_VERSION + " booting...");
	LOGD->post_message("boot", LOG_NOTICE,
		"-----------------------------------------------------");

	load_object(TESTD);
	TESTD->test();

	LOGD->post_message("boot", LOG_DEBUG, "Loading kernel manager");
	load_object(KERNELD);

	LOGD->post_message("boot", LOG_DEBUG, "Configuring kernel");
	configure_klib();

	LOGD->post_message("boot", LOG_DEBUG, "Loading object manager");

	load_object(OBJECT_INFO);
	load_object("obj/canary");
	load_object(OBJECTD);

	LOGD->post_message("boot", LOG_DEBUG, "Initializing object manager");
	OBJECTD->enable();
	OBJECTD->discover_objects();
	OBJECTD->discover_clones();

	LOGD->post_message("boot", LOG_DEBUG, "Testing object manager");
	OBJECTD->audit_clones();

	LOGD->post_message("boot", LOG_DEBUG, "Loading error manager");
	load_object(ERRORD);
	ERRORD->enable();

	LOGD->post_message("boot", LOG_DEBUG, "Loading system");
	load_dir("closed", 1);
	load_dir("lib", 1);
	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("sys", 1);

	SYSTEM_USERD->set_reserve(2);
	SYSTEM_USERD->enable();
	STATUSD->enable();
	WATCHDOGD->enable();

	KERNELD->set_global_access("System", 1);

	/* Booted up */

	boot_subsystem("Kotaka");
	boot_subsystem("Help");
	boot_subsystem("Game");
}

static void create()
{
	check_config();
	check_versions();

	rlimits (0; -1) {
		catch {
			initialize();
		} : {
			LOGD->flush();
			shutdown();
			error("System setup failed");
		}
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

void prepare_reboot()
{
	int sz;
	int index;

	ACCESS_CHECK(KERNEL());

	ACCESSD->save();

	dumped = call_out("dumped_state", 0);

	LOGD->post_message("system", LOG_NOTICE, "dumping state");
	CALLOUTD->suspend_callouts();
	SYSTEM_USERD->block_connections();

	sz = sizeof(subsystems);

	for (index = sz - 1; index >= 1; index--) {
		catch {
			call_other(USR_DIR + "/" + subsystems[index] + "/initd", "prepare_reboot");
		}
	}
}

void clear_admin()
{
	string *resources;
	int index;

	resources = KERNELD->query_resources();

	for (index = 0; index < sizeof(resources); index++) {
		KERNELD->rsrc_set_limit("admin", resources[index],
			-1);
	}
}

void hotboot()
{
	ACCESS_CHECK(KERNEL());

	LOGD->post_message("system", LOG_NOTICE, "hotbooted!");
}

void reboot()
{
	int index;
	int sz;

	ACCESS_CHECK(KERNEL());

	check_config();
	check_versions();

	LOGD->post_message("system", LOG_NOTICE, "rebooted");

	clear_admin();
	remove_call_out(dumped);
	dumped = 0;

	OBJECTD->reboot();
	WATCHDOGD->reboot();
	SYSTEM_USERD->reboot();
	WIZTOOLD->reboot();

	sz = sizeof(subsystems);

	for (index = 1; index < sz; index++) {
		catch {
			call_other(USR_DIR + "/" + subsystems[index] + "/initd", "reboot");
		}
	}

	CALLOUTD->release_callouts();
	SYSTEM_USERD->unblock_connections();
}

static void dumped_state()
{
	int sz;
	int index;

	LOGD->post_message("system", LOG_NOTICE, "state dumped");
	dumped = 0;

	clear_admin();

	sz = sizeof(subsystems);

	for (index = 1; index < sz; index++) {
		catch {
			call_other(USR_DIR + "/" + subsystems[index] + "/initd", "dumped_state");
		}
	}

	CALLOUTD->release_callouts();
	SYSTEM_USERD->unblock_connections();
}

/** Used to output messages to the console */

/*********/
/* Hooks */
/*********/

string query_constructor(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

string query_destructor(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

/********************/
/* Helper functions */
/********************/

/* Miscellaneous */

void message(string str)
{
	catch {
		error(previous_program() + " needs to use the log daemon");
	}

	LOGD->post_message("misc", LOG_NOTICE, str);
}

void boot_subsystem(string subsystem)
{
	ACCESS_CHECK(SYSTEM() || GAME());

	subsystems += ({ subsystem });

	KERNELD->add_user(subsystem);
	KERNELD->add_owner(subsystem);

	rlimits(100; -1) {
		load_object(USR_DIR + "/" + subsystem + "/initd");
	}
}

private void configure_klib()
{
	string *wizards;
	int index;

	KERNELD->add_owner("Secret");

	wizards = KERNELD->query_users();

	for (index = 0; index < sizeof(wizards); index++) {
		KERNELD->add_owner(wizards[index]);
	}

	KERNELD->set_rsrc("ticks", 10000000, 0, 0);
}

mapping read_init_file(string subsystem)
{
	string buf;

	if (!file_info(USR_DIR + "/" + subsystem + "/boot.ini")) {
		return nil;
	}
}

private void configure_logging()
{
	load_object(LOGD);
	remove_file("/log/session.log");

	LOGD->set_target("*", 255, "driver");
	LOGD->set_target("*", 255, "file:/log/general.log");
	LOGD->set_target("*", 255, "file:/log/session.log");
	LOGD->set_target("*", 255, "user");
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
		error("Array size setting is too small");
	}
}

/* shamelessly stolen from phantasmal, kudos to Noah Gibbs */
private void check_versions()
{
	int major, minor, patch;

	if (sscanf(status(ST_VERSION), "DGD %d.%d.%d", major, minor, patch) != 3) {
		patch = 0;
		if (sscanf(status(ST_VERSION), "DGD %d.%d", major, minor) != 2) {
			minor = 0;
			if(sscanf(status(ST_VERSION), "DGD %d", major) != 1) {
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
