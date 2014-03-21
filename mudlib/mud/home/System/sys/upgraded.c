/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2014  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

object hitlist;

int major;
int minor;
int patch;

static void create()
{
}

atomic void upgrade_system()
{
	compile_object(object_name(this_object()));

	call_out("check_version", 0);
}

static void check_version()
{
	major = 0;
	minor = 33;
	patch = 0;

	call_out("upgrade_system_1", 0);
}

static void upgrade_system_1()
{
	string *subs;
	int i, sz;

	ACCESS_CHECK(VERB());

	subs = INITD->query_subsystems();

	CALLOUTD->suspend_callouts();
	SYSTEM_USERD->block_connections();

	sz = sizeof(subs);

	/* upgrade all the initds */
	/* so that they are ready to receive upgrade calls */
	for (i = 0; i < sz; i++) {
		compile_object(USR_DIR + "/" + subs[i] + "/initd");
	}

	call_out("upgrade_system_2", 0, subs);
}

void add_upgrade_required(int progid)
{
	ACCESS_CHECK(SYSTEM());

	if (!hitlist->query_element(progid)) {
		object users;
		object stack;
		int i, sz;

		hitlist->set_element(progid, 1);
		users = PROGRAMD->query_inheriters(progid);

		if (!users) {
			CHANNELD->post_message("system", "upgraded", "wtf?  progid " + progid + " has no inheriter list");
			return;
		}

		sz = users->query_size();

		for (i = 0; i < sz; i++) {
			add_upgrade_required(users->query_element(i));
		}
	}
}

static void upgrade_system_2(string *subs)
{
	object libs;
	object objs;
	int i, sz;

	call_out("upgrade_system_3", 0);

	sz = sizeof(subs);

	/* to upgrade, we call all the (freshly compiled) initds */
	/* and ask them what needs upgraded */

	/* we compile that into a list and upgrade/touch all users */

	hitlist = new_object(BIGSTRUCT_MAP_LWO);
	hitlist->set_type(T_INT);

	for (i = 0; i < sz; i++) {
		catch {
			(USR_DIR + "/" + subs[i] + "/initd")->do_upgrade();
		}
	}

	hitlist = hitlist->query_indices();
	sz = hitlist->query_size();

	objs = new_object(BIGSTRUCT_ARRAY_LWO);
	libs = new_object(BIGSTRUCT_ARRAY_LWO);

	while (!hitlist->empty()) {
		object pinfo;
		string path;

		pinfo = PROGRAMD->query_program_info(hitlist->query_back());
		hitlist->pop_back();

		if (pinfo->query_destructed()) {
			continue;
		}

		path = pinfo->query_path();

		if (sscanf(path, "%*s" + INHERITABLE_SUBDIR + "%*s")) {
			libs->push_back(path);
		} else {
			objs->push_back(path);
		}
	}

	while (!libs->empty()) {
		string path;

		path = libs->query_back();
		libs->pop_back();

		CHANNELD->post_message("system", "upgraded", "Destructing " + path);

		destruct_object(path);
	}

	while (!objs->empty()) {
		string path;

		path = objs->query_back();
		objs->pop_back();

		CHANNELD->post_message("system", "upgraded", "Recompiling " + path);
		compile_object(path);

		if (sscanf(path, "/kernel/%*s")) {
			continue;
		}
	}

	/* upgrades: */
	/* 1.  Find all inheriters of a library needing an update */
	/* 2.  Have all the libraries destructed */
	/* 3.  Recompile all non libraries inheriting them */
	/* 4.  Wait for all touches to be completed */
}

static void upgrade_system_3()
{
	hitlist = nil;

	CALLOUTD->release_callouts();
	SYSTEM_USERD->unblock_connections();
	CHANNELD->post_message("system", "upgraded", "Upgrade completed");
}
