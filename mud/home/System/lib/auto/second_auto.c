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
#include <status.h>
#include <trace.h>
#include <type.h>
#include <kernel/rsrc.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit "call_guard";
inherit "catalog";

private string name;

private int enough_free_objects()
{
	int used;
	int free;
	int total;

	used = ::status(ST_NOBJECTS);
	total = ::status(ST_OTABSIZE);
	free = total - used;

	return free >= 20;
}

private int enough_free_callouts()
{
	int used;
	int free;
	int total;
	int quota;

	used = ::status(ST_NCOSHORT) + ::status(ST_NCOLONG);
	total = ::status(ST_COTABSIZE);
	free = total - used;
	quota = total / 50;

	if (quota < 10) {
		quota = 10;
	}

	return free >= quota;
}

private mixed **convert_callouts(mixed **callouts)
{
	int sz;

	for (sz = sizeof(callouts); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[sz];

		if (callout[CO_FUNCTION] == "_F_sys_callout") {
			callouts[sz] = ({ callout[CO_HANDLE], callout[CO_FIRSTXARG], callout[CO_DELAY] }) + callout[CO_FIRSTXARG + 1];
		}
	}

	return callouts;
}

static void wipe_callouts()
{
	mixed **callouts;
	int sz;

	callouts = ::status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

static void dump_state(varargs int incr)
{
	if (incr) {
		TLSD->set_tls_value("System", "incremental-snapshot", 1);
	}

	::dump_state(incr);
}

static int find_call_out(string func)
{
	mixed **callouts;
	int sz;

	callouts = ::status(this_object(), O_CALLOUTS);
	callouts = convert_callouts(callouts);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[sz];

		if (callout[CO_FUNCTION] == func) {
			return callout[CO_HANDLE];
		}
	}

	return 0;
}

static void _F_sys_callout(string func, mixed *args)
{
	int ticks;

	ticks = ::status(ST_TICKS);

	rlimits (0; -1) {
		INITD->begin_task();

		catch {
			rlimits(0; ticks) {
				call_other(this_object(), func, args...);
			}
		}
	}
}

static int call_out(string func, mixed delay, mixed args...)
{
	int handle;
	string owner;
	object this;

	this = this_object();

	if (!this) {
		error("Cannot call_out from destructed object");
	}

	if (!function_object(func, this)) {
		error("Call_out to undefined function " + func);
	}

	if (!SYSTEM() && !enough_free_callouts()) {
		error("Too many callouts");
	}

	handle = ::call_out("_F_sys_callout", delay, func, args);

	owner = query_owner();

	if (KERNELD->query_rsrc("callout peak")) {
		mixed *rsrcp, rsrcc;

		rsrcp = KERNELD->rsrc_get(owner, "callout peak");
		rsrcc = KERNELD->rsrc_get(owner, "callouts");

		if (rsrcp[RSRC_USAGE] < rsrcc[RSRC_USAGE]) {
			KERNELD->rsrc_incr(owner, "callout peak", nil, rsrcc[RSRC_USAGE] - rsrcp[RSRC_USAGE]);
		}
	}

	if (KERNELD->query_rsrc("callout usage")) {
		KERNELD->rsrc_incr(owner, "callout usage", nil, 1);
	}

	return handle;
}

static mixed status(varargs mixed obj, mixed index)
{
	mixed status;

	switch(typeof(obj)) {
	case T_NIL:
		return ::status();

	case T_INT:
		return ::status(obj);

	case T_STRING:
	case T_OBJECT:
		switch(typeof(index)) {
		case T_NIL:
			status = ::status(obj);

			if (status) {
				status[O_CALLOUTS] = convert_callouts(status[O_CALLOUTS]);
			}

			return status;

		case T_INT:
			status = ::status(obj, index);
			if (status && index == O_CALLOUTS) {
				status = convert_callouts(status);
			}
			return status;

		default:
			error("Bad argument 2");
		}

	default:
		error("Bad argument 1");
	}
}

static object compile_object(string path, string source...)
{
	object obj;

	obj = find_object(path);

	if (!SYSTEM() &&
		DRIVER->creator(path) != "System" &&
		!obj && !enough_free_objects()) {
		error("Too many objects");
	}

	return ::compile_object(path, source...);
}

static object load_object(string path)
{
	object obj;

	obj = find_object(path);

	if (obj) {
		return obj;
	}

	if (!SYSTEM() &&
		DRIVER->creator(path) != "System" &&
		!obj && !enough_free_objects()) {
		error("Too many objects");
	}

	return ::compile_object(path);
}

static object clone_object(string path, varargs string uid)
{
	if (!SYSTEM() &&
		query_owner() != "System" &&
		!enough_free_objects()) {
		error("Too many objects");
	}

	return ::clone_object(path, uid);
}

static void destruct(varargs int clone)
{
}

nomask void _F_dummy()
{
}

nomask int _F_sys_create(int clone)
{
	string oname;
	object this;
	object objectd;

	ACCESS_CHECK(KERNEL() || SYSTEM());

	this = this_object();
	oname = object_name(this);

	sscanf(oname, "%s#%*d", oname);

	objectd = find_object(OBJECTD);

	if (objectd) {
		object pinfo;

		pinfo = OBJECTD->query_program_info(::status(this, O_INDEX));

		if (pinfo) {
			string ctor;
			mixed *ctors;
			int i, sz;

			ctor = pinfo->query_constructor();
			ctors = pinfo->query_inherited_constructors();

			if (ctors) {
				sz = sizeof(ctors);

				for (i = 0; i < sz; i++) {
					call_limited(ctors[i]);
				}
			}

			if (ctor) {
				call_limited(ctor);
			}
		}
	}

	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		call_limited("create");
	} else {
		call_limited("create", clone);
	}

	return 1;
}

nomask void _F_sys_destruct()
{
	string oname;
	object this;
	object objectd;

	int clone;

	ACCESS_CHECK(KERNEL() || SYSTEM());

	this = this_object();
	oname = object_name(this);

	clone = !!sscanf(oname, "%*s#");

	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
		sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
		destruct();
	} else {
		destruct(clone);
	}

	objectd = find_object(OBJECTD);

	if (objectd) {
		object pinfo;

		pinfo = objectd->query_program_info(
			status(this, O_INDEX)
		);

		if (pinfo) {
			string dtor;
			string *dtors;
			int i, sz;

			dtor = pinfo->query_destructor();
			dtors = pinfo->query_inherited_destructors();

			if (dtor) {
				call_limited(dtor);
			}

			if (dtors) {
				sz = sizeof(dtors);

				for (i = sz - 1; i >= 0; i--) {
					call_limited(dtors[i]);
				}
			}
		}
	}
}

nomask int _F_touch(string func)
{
	object this;
	object pinfo;
	string patcher;
	string *patchers;
	int i, sz;

	ACCESS_CHECK(previous_program() == OBJECTD);

	if (!find_object(PATCHD)) {
		return 0;
	}

	this = this_object();

	if (TLSD->query_tls_value("System", "destruct_force") == this) {
		return 0;
	}

	if (!PATCHD->query_marked(this)) {
		return 0;
	}

	pinfo = OBJECTD->query_program_info(status(this, O_INDEX));

	if (!pinfo) {
		LOGD->post_message("system", LOG_WARNING, "Attempted to patch " + object_name(this) + " without program_info");

		return 0;
	}

	patcher = pinfo->query_patcher();
	patchers = pinfo->query_inherited_patchers();

	if (patcher) {
		patchers |= ({ patcher });
	}

	PATCHD->clear_mark(this);

	for (sz = sizeof(patchers), i = 0; i < sz; i++) {
		catch {
			call_limited(patchers[i]);
		}
	}

	return 0;
}

/*
void add_object(string name, object obj)
void remove_object(string name)
object lookup_object(string name)
*/

void patch_object_name()
{
	if (name) {
		return;
	}

	name = ::query_object_name();

	if (name) {
		LOGD->post_message("system", LOG_NOTICE, "Migrating name " + name);

		IDD->add_object_name(name, this_object());

		::set_object_name(nil);
	}
}

void set_object_name(string new_name)
{
	patch_object_name();

	if (name == new_name) {
		return;
	}

	if (new_name) {
		if (CATALOGD->lookup_object(new_name)) {
			error("Duplicate name");
		}

		if (IDD->find_object_by_name(new_name)) {
			error("Duplicate name");
		}
	}

	if (name) {
		IDD->remove_object_name(name);
	}

	name = new_name;

	if (name) {
		IDD->add_object_name(name, this_object());
	}
}

string query_object_name()
{
	patch_object_name();

	return name;
}

static void call_out_unique(string func, mixed delay)
{
	mixed **callouts;
	int sz;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		if (callouts[sz][CO_FUNCTION] == func) {
			return;
		}
	}

	call_out(func, delay);
}
