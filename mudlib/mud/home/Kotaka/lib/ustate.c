#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/paths.h>

object "user" user;
object "ustate" parent;
object "ustate" current;
object "ustate" *children;

nomask void _F_set_current(object "ustate" state);
nomask void _F_set_parent(object "ustate" state);
nomask void _F_set_user(object "user" user);
nomask void _F_add_child(object "ustate" state);
nomask void _F_del_child(object "ustate" state);

static void receive_in(string str);
static void receive_out(string str);
static void begin();
static void stop();
static void go();
static void end();

void send_in(string str);
void send_out(string str);
void push_state(object "ustate" state);
void pop_state();
void switch_state(object "ustate" state);
void swap_state(object "ustate" state);

static void create()
{
	children = ({ });
}

nomask object "user" query_user()
{
	return user;
}

nomask object "ustate" query_parent()
{
	return parent;
}

nomask object "ustate" query_current()
{
	return current;
}

nomask object "ustate" *query_children()
{
	return children - ({ nil });
}

nomask object "ustate" query_top()
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

nomask void _F_set_user(object "user" new_user)
{
	ACCESS_CHECK(KOTAKA());

	ASSERT(!!user != !!new_user);
	user = new_user;
}

nomask void _F_set_current(object "ustate" state)
{
	ACCESS_CHECK(KOTAKA());

	ASSERT(!state || user == state->query_user());
	ASSERT(state != (object "ustate")this_object());
	current = state;
}

nomask void _F_set_parent(object "ustate" state)
{
	ACCESS_CHECK(KOTAKA());

	ASSERT(!state || user == state->query_user());
	ASSERT(state != (object "ustate")this_object());
	parent = state;
}

nomask void _F_add_child(object "ustate" state)
{
	ACCESS_CHECK(KOTAKA());

	ASSERT(user == state->query_user());
	ASSERT(state != (object "ustate")this_object());
	children -= ({ nil });
	children += ({ state });
}

nomask void _F_del_child(object "ustate" state)
{
	ACCESS_CHECK(KOTAKA());

	ASSERT(state);
	ASSERT(user == state->query_user());
	ASSERT(state != (object "ustate")this_object());
	children -= ({ state, nil });
}

static void receive_in(string str)
{
	send_in(str);
}

static void receive_out(string str)
{
	send_out(str);
}

static void begin()
{
}

static void stop()
{
}

static void go()
{
}

static void end()
{
}

void send_in(string str)
{
	ACCESS_CHECK(PRIVILEGED() || calling_object() == this_object());
	
	if (parent) {
		parent->_F_receive_in(str);
	} else {
		user->send_in(str);
	}
}

void send_out(string str)
{
	ACCESS_CHECK(PRIVILEGED() || calling_object() == this_object());
	
	if (current) {
		current->_F_receive_out(str);
	} else {
		user->send_out(str);
	}
}

void push_state(object "ustate" state)
{
	ACCESS_CHECK(PRIVILEGED() || calling_object() == this_object());

	if (state->query_user()) {
		error("Busy");
	}

	user->_F_push_state(this_object(), state);
}

void pop_state()
{
	ACCESS_CHECK(PRIVILEGED() || calling_object() == this_object());

	user->_F_pop_state(this_object());
}

void switch_state(object "ustate" state)
{
	ACCESS_CHECK(PRIVILEGED() || calling_object() == this_object());

	ASSERT(this_object());

	if (state) {
		if (state->query_user() != user) {
			error("Cannot switch to foreign ustate");
		}
		if (state->query_parent() != this_object()) {
			error("Not a child");
		}
	}

	user->_F_switch_state(this_object(), state);
}

void swap_state(object "ustate" state)
{
	ACCESS_CHECK(PRIVILEGED() || calling_object() == this_object());

	if (state->query_user()) {
		error("Busy");
	}

	user->_F_swap_state(this_object(), state);
}
