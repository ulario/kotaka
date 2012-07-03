#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kernel/rsrc.h>
#include <kernel/tls.h>
#include <kernel/user.h>

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

int bogus;

string *subsystems;

void console_post(string str, int level);
void message(string str);
private void set_status(string str);
private void configure_klib();
private void boot_subsystem(string subsystem);
private void configure_logging();
private void check_config();

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
		"Kotaka mudlib v" + KOTAKA_VERSION + " booting...");
	LOGD->post_message("boot", LOG_NOTICE,
		"-----------------------------------------------------");

	LOGD->post_message("boot", LOG_DEBUG, "Testing bigstruct library");
	load_object(TESTD);
	TESTD->test();

	LOGD->post_message("boot", LOG_DEBUG, "Loading kernel manager");
	load_object(KERNELD);

	LOGD->post_message("boot", LOG_DEBUG, "Configuring kernel");
	configure_klib();

	LOGD->post_message("boot", LOG_DEBUG, "Loading object manager");
	load_object(OBJECT_INFO);
	load_object(OBJECTD);

	LOGD->post_message("boot", LOG_DEBUG, "Enabling and initializing");
	OBJECTD->enable();
	OBJECTD->full_reset();

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
	TRASHD->enable();
	SYSTEM_USERD->enable();
	STATUSD->enable();
	WATCHDOGD->enable();

	KERNELD->set_global_access("Kotaka", 1);
	KERNELD->set_global_access("System", 1);

	/* Booted up */

	boot_subsystem("Kotaka");
	boot_subsystem("Game");

	set_status("ok");
}

static void create()
{
	check_config();

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

	set_status("rebooting");

	sz = sizeof(subsystems);

	for (index = sz - 1; index >= 1; index--) {
		catch {
			call_other(USR_DIR + "/" + subsystems[index] + "/initd", "prepare_reboot");
		}
	}

	LOGD->post_message("system", LOG_NOTICE, "saving");
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

void reboot()
{
	int index;
	int sz;

	ACCESS_CHECK(KERNEL());

	LOGD->post_message("system", LOG_NOTICE, "rebooted successfully");
	set_status("ok");

	bogus = 0;

	clear_admin();

	catch {
		SYSTEM_USERD->reboot();
	}

	WATCHDOGD->reboot();

	sz = sizeof(subsystems);

	for (index = 1; index < sz; index++) {
		catch {
			call_other(USR_DIR + "/" + subsystems[index] + "/initd", "reboot");
		}
	}
}

/** Handles a bogus reboot */
void bogus_reboot()
{
	int sz;
	int index;

	LOGD->post_message("system", LOG_NOTICE, "state dumped");

	set_status("ok");

	sz = sizeof(subsystems);

	for (index = 1; index < sz; index++) {
		catch {
			call_other(USR_DIR + "/" + subsystems[index] + "/initd", "bogus_reboot");
		}
	}
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

private void set_status(string str)
{
}

void message(string str)
{
	catch {
		error(previous_program() + " needs to use the log daemon");
	}

	LOGD->post_message("misc", LOG_NOTICE, str);
}

private void boot_subsystem(string subsystem)
{
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

	KERNELD->set_rsrc("ticks", 100000000, 0, 0);
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
