/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <status.h>

inherit system_user LIB_SYSTEM_USER;

/*************/
/* Variables */
/*************/

private object root;
int suspend;
int disconnecting;
int destructing;

/****************/
/* Declarations */
/****************/

nomask void force_quit();
void quit();

void pop_state(object state);
void push_state(object state, object parent);
void switch_state(object parent, object new);
void swap_state(object old, object new);
void suspend_user();
void resume_user();

static void feed_in(string str);
static void feed_out(string str);

static void nuke_state_tree(varargs object base);

/***************/
/* Definitions */
/***************/

static void create()
{
	system_user::create();
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

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| LOCAL());

	disconnecting = 1;

	call_limited("nuke_state_tree", root);

	if (!destructing) {
		destruct_object(this_object());
	}
}

static void destruct()
{
	destructing = 1;

	if (!disconnecting) {
		disconnect();
	}
}

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
			::message(object_name(parent) + "\n");

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

static void feed_in(string str)
{
	root->query_top()->receive_in(str);
}

static void feed_out(string str)
{
	root->receive_out(str);
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
void switch_state(object parent, object new)
{
	object oldtop;
	object newtop;

	ACCESS_CHECK(TEXT());
	ASSERT(parent);

	if (parent->query_current() == new) {
		return;
	}

	if (new) {
		ASSERT(new->query_parent() == parent);
	}

	oldtop = root->query_top();

	parent->_F_set_current(new);

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
void swap_state(object old, object new)
{
	/* the old ustate is being replaced */
	object parent;

	ACCESS_CHECK(TEXT());

	ASSERT(old);
	ASSERT(new);
	ASSERT(!new->query_user());

	parent = old->query_parent();

	if (root == old) {
		root = new;
	}

	new->_F_set_user(this_object());
	new->_F_set_parent(parent);
	new->pre_begin();

	if (parent) {
		parent->_F_add_child(new);
		parent->_F_set_current(new);
	}

	if (old == root) {
		root = new;
	}

	nuke_state_tree(old);

	if (new) {
		new->begin();
	}

	if (new && !suspend) {
		new->go();
	}
}

void collapse_state(object old, object new)
{
	/* the old ustate is being replaced */
	object parent;
	object newtop;

	ACCESS_CHECK(TEXT());

	ASSERT(old);
	ASSERT(new);

	parent = old->query_parent();

	if (root == old) {
		root = new;
	}

	new->_F_set_parent(parent);
	new->pre_begin();

	if (parent) {
		parent->_F_add_child(new);
		parent->_F_set_current(new);
		parent->_F_del_child(old);
	}

	old->_F_del_child(new);

	if (old == root) {
		root = new;
	}

	nuke_state_tree(old);
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

void quit()
{
	disconnect();
}

nomask int ustate_call_out(object ustate, string function, mixed delay, mixed *args)
{
	ACCESS_CHECK(TEXT());

	return call_out("user_callout", delay, ustate, function, args);
}

static void user_callout(object ustate, string function, mixed *args)
{
	if (ustate) {
		ustate->ustate_callout(function, args);
	}
}

private mixed *find_call_out(int handle)
{
	mixed **callouts;
	int i, sz;

	callouts = status(this_object(), O_CALLOUTS);
	sz = sizeof(callouts);

	for (i = 0; i < sz; i++) {
		mixed *callout;

		callout = callouts[i];

		if (callout[CO_HANDLE] == handle) {
			return callout;
		}
	}
}

nomask mixed ustate_remove_call_out(object ustate, int handle)
{
	mixed *callout;

	ACCESS_CHECK(TEXT());

	callout = find_call_out(handle);

	if (!callout) {
		error("No such callout");
	}

	ACCESS_CHECK(callout[CO_FIRSTXARG] == ustate);

	return remove_call_out(handle);
}
