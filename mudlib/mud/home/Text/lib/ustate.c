/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>

object user;
object parent;
object current;
object *children;

int events;

/* we need to prevent reentry during tree manipulation */

nomask void _F_set_current(object state);
nomask void _F_set_parent(object state);
nomask void _F_set_user(object user);
nomask void _F_add_child(object state);
nomask void _F_del_child(object state);

/* tree events */
void begin();
void end();

/* stack events */
void push(object state);
void pop(object state);

/* shell events */
void stop();
void go();

/* I/O events */
void receive_in(string str);
void receive_out(string str);

/* precedence: */
/* membership <-> stack <-> shell */

void send_in(string str);
void send_out(string str);
void push_state(object state);
void pop_state();
void switch_state(object state);
void swap_state(object state);

static void create()
{
	children = ({ });
}

nomask object query_user()
{
	return user;
}

nomask object query_parent()
{
	return parent;
}

nomask object query_current()
{
	return current;
}

nomask object *query_children()
{
	return children - ({ nil });
}

nomask object query_top()
{
	object top;
	object newtop;

	if (!current) {
		return this_object();
	}

	top = current;

	while (newtop = top->query_current()) {
		top = newtop;
	}

	return top;
}

nomask void _F_set_user(object new_user)
{
	ACCESS_CHECK(TEXT());

	ASSERT(!!user != !!new_user);
	user = new_user;
}

nomask void _F_set_current(object state)
{
	ACCESS_CHECK(TEXT());

	ASSERT(!state || user == state->query_user());
	ASSERT(state != (object)this_object());
	current = state;
}

nomask void _F_set_parent(object state)
{
	ACCESS_CHECK(TEXT());

	ASSERT(!state || user == state->query_user());
	ASSERT(state != (object)this_object());
	parent = state;
}

nomask void _F_add_child(object state)
{
	ACCESS_CHECK(TEXT());

	ASSERT(user == state->query_user());
	ASSERT(state != (object)this_object());
	children -= ({ nil });
	children += ({ state });
}

nomask void _F_del_child(object state)
{
	ACCESS_CHECK(TEXT());

	ASSERT(state);
	ASSERT(user == state->query_user());
	ASSERT(state != (object)this_object());
	children -= ({ state, nil });
}

void pre_begin()
{
}

void begin()
{
}

void pre_end()
{
}

void end()
{
}

void push(object state)
{
}

void pop(object state)
{
}

void stop()
{
}

void go()
{
}

void receive_in(string str)
{
	send_in(str);
}

void receive_out(string str)
{
	send_out(str);
}

void send_in(string str)
{
	if (parent) {
		parent->receive_in(str);
	} else {
		user->send_in(str);
	}
}

void send_out(string str)
{
	if (current) {
		current->receive_out(str);
	} else {
		user->send_out(str);
	}
}

void push_state(object state)
{
	ASSERT(user);

	user->push_state(state, this_object());
}

void pop_state()
{
	ASSERT(user);

	user->pop_state(this_object());
}

void switch_state(object new)
{
	ASSERT(user);

	user->switch_state(this_object(), new);
}

void swap_state(object new)
{
	ASSERT(user);

	user->swap_state(this_object(), new);
}

static void terminate_account_state()
{
	object parent;

	parent = query_parent();

	if (parent <- "~/obj/ustate/start") {
		parent->swap_state(clone_object("shell"));
	} else {
		pop_state();
	}
}

static int call_out(string function, mixed delay, mixed args ...)
{
	return user->ustate_call_out(this_object(), function, delay, args);
}

static mixed remove_call_out(int handle)
{
	return user->ustate_remove_call_out(this_object(), handle);
}

nomask void ustate_callout(string function, mixed *args)
{
	ACCESS_CHECK(TEXT());

	call_other(this_object(), function, args ...);
}
