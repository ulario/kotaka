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
	ACCESS_CHECK(KOTAKA());

	ASSERT(!!user != !!new_user);
	user = new_user;
}

nomask void _F_set_current(object state)
{
	ACCESS_CHECK(KOTAKA());

	ASSERT(!state || user == state->query_user());
	ASSERT(state != (object)this_object());
	current = state;
}

nomask void _F_set_parent(object state)
{
	ACCESS_CHECK(KOTAKA());

	ASSERT(!state || user == state->query_user());
	ASSERT(state != (object)this_object());
	parent = state;
}

nomask void _F_add_child(object state)
{
	ACCESS_CHECK(KOTAKA());

	ASSERT(user == state->query_user());
	ASSERT(state != (object)this_object());
	children -= ({ nil });
	children += ({ state });
}

nomask void _F_del_child(object state)
{
	ACCESS_CHECK(KOTAKA());

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
