/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
#include <kernel/user.h>
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/ustate.h>
#include <status.h>

inherit system_user LIB_SYSTEM_USER;

private object root;
int suspend;

nomask void force_quit();
void quit();

void pop_state(object state);
void push_state(object state, object parent);
void switch_state(object parent, object new_state);
void swap_state(object old, object new_state);
void suspend_user();
void resume_user();

static void feed_in(string str);
static void feed_out(string str);

static void nuke_state_tree(varargs object base);

private void do_escape(string str)
{
	string *words;
	object *children;
	object parent;

	words = explode(str, " ") - ({ "" });

	if (!sizeof(words)) {
		return;
	}

	switch(words[0]) {
	case "help":
		::message(
			"help\n----\n" +
			"help\tHelp\n" +
			"list\tList child ustates\n" +
			"trace\tList all objects in connection chain\n" +
			"states\tList all ustates in current stack\n" +
			"abort\tKill current ustate\n" +
			"quit\tForce quit\n" +
			"suspend\tSuspend current ustate\n" +
			"switch\tResume specified ustate\n"
		);

	case "list":
		children = root->query_top()->query_children();

		if (sizeof(children)) {
			int i;

			::message("List of child ustates:\n");

			for (i = 0; i < sizeof(children); i++) {
				::message(i + ": " + object_name(children[i]) + "\n");
			}
		}
		break;

	case "trace":
		parent = this_object();

		while(parent) {
			::message(object_name(parent) + " (owned by " + parent->query_owner() + ")\n");

			if (parent <- LIB_USER) {
				parent = parent->query_conn();
			} else {
				break;
			}
		}
		break;

	case "states":
		parent = root->query_top();

		while(parent) {
			::message(object_name(parent) + "\n");
			parent = parent->query_parent();
		}
		break;

	case "suspend":
		parent = root->query_top()->query_parent();

		if (!parent) {
			::message("Cannot suspend root ustate\n");
		} else {
			parent->switch_state(nil);
		}
		break;

	case "switch":
		{
			int i;

			i = (int)words[1];

			children = root->query_top()->query_children();

			if (i >= sizeof(children)) {
				::message("Index out of range.\n");
				return;
			}

			root->query_top()->switch_state(children[i]);
		}
		break;

	case "abort":
		if (root == root->query_top()) {
			::message("Cannot abort root ustate\n");
		} else {
			root->query_top()->pop_state();
		}
		break;

	case "quit":
		force_quit();
		break;

	default:
		::message("Invalid escape\n");
	}
}

static void create()
{
	system_user::create();
}

static void feed_in(string str)
{
	root->query_top()->receive_in(str);
}

static void feed_out(string str)
{
	root->receive_out(str);
}

static void nuke_state_tree(varargs object base)
{
	int i;
	object parent;
	object *children;

	if (!base) {
		return;
	}

	if (base->query_user() != this_object()) {
		error("Bad nuke");
	}

	parent = base->query_parent();
	children = base->query_children();

	if (base) {
		base->pre_end();
	}

	for (i = 0; i < sizeof(children); i++) {
		nuke_state_tree(children[i]);
	}

	catch {
		if (base) {
			base->_F_set_parent(nil);
		}

		if (parent) {
			parent->_F_del_child(base);
			parent->pop(base);
		}

		if (base) {
			base->end();
		}

		if (base) {
			base->_F_set_user(nil);
		}
	}
}

static object query_top_state()
{
	return root->query_top();
}

int is_wizard()
{
	if (query_name() == "admin") {
		return 1;
	}

	return !!sizeof(KERNELD->query_users() & ({ query_name() }) );
}

int is_admin()
{
	if (query_name() == "admin") {
		return 1;
	}

	return KERNELD->access(query_name(), "/", FULL_ACCESS);
}

nomask object query_root()
{
	return root;
}

nomask void force_quit()
{
	ACCESS_CHECK(INTERFACE());

	disconnect();
}

void send_out(string str)
{
	ACCESS_CHECK(INTERFACE() || LOCAL());

	::message(str);
}

void send_in(string str)
{
	ACCESS_CHECK(INTERFACE() || LOCAL());

	error("Stack underflow");
}

int login(string method)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| LOCAL());

	::connection(previous_object());

	return MODE_NOCHANGE;
}

void logout()
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| LOCAL());

	call_limited("nuke_state_tree", root);
}


int receive_message(string str)
{
	ACCESS_CHECK(INTERFACE() || LOCAL());

	if (str && strlen(str) > 0 && str[0] == '!') {
		do_escape(str[1 ..]);
		return MODE_NOCHANGE;
	}

	call_limited("feed_in", str);

	return MODE_NOCHANGE;
}

void set_mode(int new_mode)
{
	ACCESS_CHECK(INTERFACE() || LOCAL());

	::set_mode(new_mode);
}

void message(string str)
{
	feed_out(str);
}

/* push: new begin, parent push, new go */
void push_state(object state, object parent)
{
	object oldtop;
	object newtop;

	ACCESS_CHECK(TEXT());
	ASSERT(parent);

	if (!root) {
		error("Internal error: cannot stack ustates without a root");
	}

	oldtop = root->query_top();

	state->_F_set_user(this_object());
	state->pre_begin();

	if (!state) {
		return;
	}

	state->_F_set_parent(parent);
	parent->_F_add_child(state);
	parent->_F_set_current(state);

	newtop = root->query_top();

	if (oldtop != newtop) {
		oldtop->stop();
	}

	if (state) {
		state->begin();
	}

	if (parent) {
		parent->push(state);
	}

	if (state && !suspend) {
		state->go();
	}
}

/* pop: state stop, parent pop, tree end */
void pop_state(object state)
{
	object oldtop;
	object newtop;
	object parent;

	ACCESS_CHECK(TEXT());
	ASSERT(state);
	parent = state->query_parent();
	ASSERT(parent);

	if (sizeof(state->query_children())) {
		error("Cannot pop a full ustate");
	}

	oldtop = root->query_top();

	if (state) {
		state->pre_end();
	}

	if (parent) {
		if (state) {
			parent->_F_del_child(state);

			if (state == parent->query_current()) {
				parent->_F_set_current(nil);
			}
		}
	}

	if (state) {
		state->_F_set_parent(nil);
	}

	if (parent) {
		parent->pop(state);
	}

	if (state) {
		state->end();

		if (state) {
			state->_F_set_user(nil);
		}
	}

	if (!suspend) {
		newtop = root->query_top();

		if (oldtop != newtop) {
			newtop->go();
		}
	}
}

/* switch: old stop, new go */
void switch_state(object parent, object new_state)
{
	object oldtop;
	object newtop;

	ACCESS_CHECK(TEXT());
	ASSERT(parent);

	if (parent->query_current() == new_state) {
		return;
	}

	if (new_state) {
		ASSERT(new_state->query_parent() == parent);
	}

	oldtop = root->query_top();

	parent->_F_set_current(new_state);

	newtop = root->query_top();

	if (!suspend) {
		if (oldtop != newtop) {
			oldtop->stop();

			if (newtop) {
				newtop->go();
			}
		}
	}
}

/* swap: old stop, parent pop, old end, new begin, parent push, new go */
void swap_state(object old_state, object new_state)
{
	/* the old ustate is being replaced */
	object parent;

	ACCESS_CHECK(TEXT());

	ASSERT(old_state);
	ASSERT(new_state);

	parent = old_state->query_parent();

	if (root == old_state) {
		root = new_state;
	}

	new_state->_F_set_user(this_object());
	new_state->_F_set_parent(parent);
	new_state->pre_begin();

	if (parent) {
		parent->_F_add_child(new_state);
		parent->_F_set_current(new_state);
	}

	if (old_state == root) {
		root = new_state;
	}

	nuke_state_tree(old_state);

	if (new_state) {
		new_state->begin();
	}

	if (new_state && !suspend) {
		new_state->go();
	}
}

void collapse_state(object old_state, object new_state)
{
	/* the old ustate is being replaced */
	object parent;
	object newtop;

	ACCESS_CHECK(TEXT());

	ASSERT(old_state);
	ASSERT(new_state);

	parent = old_state->query_parent();

	if (root == old_state) {
		root = new_state;
	}

	new_state->_F_set_parent(parent);
	new_state->pre_begin();

	if (parent) {
		parent->_F_add_child(new_state);
		parent->_F_set_current(new_state);
		parent->_F_del_child(old_state);
	}

	old_state->_F_del_child(new_state);

	if (old_state == root) {
		root = new_state;
	}

	nuke_state_tree(old_state);
}

object query_root_state()
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	return root;
}

void suspend_user()
{
	ACCESS_CHECK(TEXT());

	if (!suspend) {
		object top;

		top = root->query_top();
		top->stop();

		suspend = 1;
	}
}

void release_user()
{
	ACCESS_CHECK(TEXT());

	if (suspend) {
		object top;

		top = root->query_top();
		top->go();

		suspend = 0;
	}
}

void set_root_state(object state)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	if (root) {
		error("User object already has a root ustate");
	}

	root = state;

	state->_F_set_user(this_object());

	state->begin();
	state->go();
}

object clone_ustate(string id)
{
	object ustate;

	ustate = clone_object(USTATE_DIR + "/" + id);

	return ustate;
}

void quit()
{
	disconnect();
}
