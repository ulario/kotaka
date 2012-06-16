#include <kernel/kernel.h>
#include <kernel/user.h>

#include <kotaka/assert.h>
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>

#include <trace.h>

inherit SECOND_AUTO;

inherit conn LIB_CONN;
inherit user LIB_USER;

static void create()
{
	user::create();
	conn::create("telnet");
}

/*********************************/
/* user hooks (conn::user->func) */
/*********************************/

/*
login
logout
receive_message
message_done
open_datagram
receive_datagram
*/

int login(string str)
{
	int newmode;

	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());
	
	connection(previous_object());

	conn::open(nil);
	
	newmode = conn::receive_message(nil, str);
	
	if (!this_object()) {
		return MODE_DISCONNECT;
	}

	return newmode;
}

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	/* apparently, to say we're going linkdead, we have to mean it */
	
	/* 0 means linkdead, 1 means destruct/quit */

	if (quit) {
		/* we won't be destructed */
	} else {
		/* we will be klib destructed */

		/* apparently we have to be honest to the Klib here.

		We cannot return linkdead, unless we also allow
		the klib to destruct us */
	}

	conn::close(nil, quit);

	if (quit) {
		destruct_object(this_object());
	}
}

int receive_message(string str)
{
	int newmode;
	string err;

	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	newmode = conn::receive_message(nil, str);
	
	return newmode;
}

int message_done()
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	conn::message_done(nil);

	return MODE_NOCHANGE;
}

# ifndef SYS_NETWORKING
void open_datagram()
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	conn::open_datagram(nil);
}
# endif

void receive_datagram(string packet)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	conn::receive_datagram(nil, packet);
}

/***************************************/
/* connection hooks (user::conn->func) */
/***************************************/

/*
message
disconnect
datagram_challenge
datagram
timeout
*/

int message(string str)
{
	ACCESS_CHECK(previous_object() == query_user()
		|| calling_object() == this_object());

	return user::message(str);
}

void disconnect()
{
	ACCESS_CHECK(previous_object() == query_user()
		|| calling_object() == this_object());

	user::disconnect();
}

void datagram_challenge(string str)
{
	ACCESS_CHECK(previous_object() == query_user()
		|| calling_object() == this_object());

	user::datagram_challenge(str);
}

int datagram(string str)
{
	ACCESS_CHECK(previous_object() == query_user()
		|| calling_object() == this_object());

	return user::datagram(str);
}

static void timeout()
{
	::timeout(allocate(DRIVER->query_tls_size()));
}

/*************************************/
/* connection api (this->conn::func) */
/*************************************/

void set_mode(int new_mode)
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN || SYSTEM() ||
		calling_object() == this_object());

	if (!this_object()) {
		if (new_mode != MODE_NOCHANGE) {
			error("No object: trying to set mode " + new_mode);
		}
		
		return;
	}

	conn = query_conn();
	
	if (!conn) {
		return;
	}
	
	switch(new_mode) {
	case MODE_NOCHANGE:
		break;
	default:
		conn->set_mode(new_mode);
	}
}

static void self_destruct()
{
	destruct_object(this_object());
}

static void self_disconnect()
{
	disconnect();
}

/******************************/
/* user api (this->user::func) */
/******************************/

/***********/
/* Unknown */
/***********/
