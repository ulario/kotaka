#include <kernel/kernel.h>
#include <kernel/user.h>
#include <kernel/access.h>
#include <kotaka/bigstruct.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>

#include <status.h>

inherit system_user LIB_SYSTEM_USER;

/*************/
/* Variables */
/*************/

private object root;

/****************/
/* Declarations */
/****************/

nomask void force_quit();
void quit();

nomask void _F_pop_state(object state);
nomask void _F_push_state(object state, object parent);
nomask void _F_switch_state(object old, object new);
nomask void _F_swap_state(object old, object new);

static void feed_in(string str);
static void feed_out(string str);

static void nuke_state_tree(varargs object "ustate" base);

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
	ACCESS_CHECK(PRIVILEGED());
	disconnect();
}

void send_out(string str)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	::message(str);
}

private void boot_ustate(object "ustate" ustate)
{
	catch {
		ustate->_F_begin();
	}
	catch {
		if (ustate == root->query_top()) {
			ustate->_F_go();
		}
	}
}

void send_in(string str)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

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

	call_limited("nuke_state_tree", root);

	destruct_object(this_object());
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
			"bt\tBacktrace\n" +
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

	case "bt":
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
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	if (str && strlen(str) > 0 && str[0] == '!') {
		do_escape(str[1 ..]);
		return MODE_NOCHANGE;
	}
	
	call_limited("feed_in", str);
	
	return MODE_NOCHANGE;
}

void set_mode(int new_mode)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	::set_mode(new_mode);
}

void message(string str)
{
	feed_out(str);
}

static void feed_in(string str)
{
	root->query_top()->_F_receive_in(str);
}

static void feed_out(string str)
{
	root->_F_receive_out(str);
}

nomask void _F_pop_state(object state)
{
	object old_active;
	object parent;
	object new_active;
	
	ACCESS_CHECK(KOTAKA());
	ASSERT(state);

	if (sizeof(state->query_children())) {
		error("Cannot pop a full ustate");
	}
	
	old_active = root->query_top();
	parent = state->query_parent();
	
	if (!parent) {
		error("Cannot pop bottom ustate");
	}
	
	catch {
		state->_F_end();
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
		state->_F_set_user(nil);
	}
	
	new_active = root->query_top();

	if (old_active != new_active) {
		new_active->_F_go();
	}
}

nomask void _F_push_state(object parent, object state)
{
	int active_changed;
	
	ACCESS_CHECK(KOTAKA());
	ASSERT(parent);

	if (!root) {
		error("Internal error: cannot stack ustates without a root");
	}

	if (root->query_top() == parent) {
		catch {
			root->query_top()->_F_stop();
		}
	}

	state->_F_set_user(this_object());
	state->_F_set_parent(parent);
			
	parent->_F_add_child(state);
	parent->_F_set_current(state);
	
	boot_ustate(state);
}

nomask void _F_switch_state(object parent, object new)
{
	int active_changed;

	ACCESS_CHECK(KOTAKA());
	
	ASSERT(parent);
	ASSERT(new);
	ASSERT(new->query_parent() == parent);

	if (parent->query_top() == root->query_top()) {
		active_changed = 1;
		
		catch {
			root->query_top()->_F_stop();
		}
	}

	parent->_F_set_current(new);
	
	if (active_changed) {
		root->query_top()->_F_go();
	}
}

nomask void _F_swap_state(object old, object new)
{
	/* the old ustate is being replaced */
	object parent;
	object pcurrent;
	object pnewcurrent;
	object pnewroot;

	ACCESS_CHECK(KOTAKA());
	
	ASSERT(old);
	ASSERT(new);
	ASSERT(!new->query_user());
	
	parent = old->query_parent();
	
	nuke_state_tree(old);
	
	new->_F_set_user(this_object());
	new->_F_set_parent(parent);
	
	if (parent) {
		parent->_F_set_current(new);
	}
	
	if (old == root) {
		root = new;
	}
	
	
	boot_ustate(new);
}

object query_root_state()
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());
	
	return root;
}

void set_root_state(object state)
{
	ACCESS_CHECK(PRIVILEGED() || LOCAL());

	if (root) {
		error("User object already has a root ustate");
	}
		
	root = state;

	state->_F_set_user(this_object());
	
	boot_ustate(state);
}

static void nuke_state_tree(varargs object "ustate" base)
{
	int i;
	object parent;
	object *children;
	
	if (!base) {
		base = root;
	}

	if (base->query_user() != this_object()) {
		error("Bad nuke");
	}
	
	parent = base->query_parent();
	children = base->query_children();
	
	for (i = 0; i < sizeof(children); i++) {
		nuke_state_tree(children[i]);
	}
	
	catch {
		LOGD->post_message("user", LOG_INFO,
			"Ending ustate: " + object_name(base));
		base->_F_end();
	} : {
		CHANNELD->post_message("warning", "Common::user",
			"Error nuking: "
			+ object_name(base) + "\n");
	}
	
	if (base) {
		base->_F_set_parent(nil);
		base->_F_set_user(nil);
		
		if (parent) {
			LOGD->post_message("user", LOG_INFO,
				"Removing from parent: " + object_name(parent));
			parent->_F_del_child(base);
		}
	}
}

void quit()
{
	disconnect();
}
