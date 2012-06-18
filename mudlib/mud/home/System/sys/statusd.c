#include <kernel/user.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <status.h>

inherit wiz "~/closed/lib/wiztool_gate";
inherit man LIB_MANAGER;
inherit user LIB_USER;
inherit SECOND_AUTO;

string message;
mapping connections;
float interval;
int callout;
/* ([ connobj : ({ delay, trust, callout }) ]) */

private void schedule();

static void create()
{
	wiz::create(0);
	man::create();
	user::create();

	connections = ([ ]);
	interval = 0.0;
	schedule();
}

void set_interval(float new_interval)
{
	interval = new_interval;

	schedule();
}

void disable()
{
	ACCESS_CHECK(KADMIN() || SYSTEM());

	PORTD->set_binary_manager(2, nil);
}

void enable()
{
	ACCESS_CHECK(KADMIN() || SYSTEM());

	PORTD->set_binary_manager(2, this_object());
}

private float swap_used_ratio()
{
	return (float)status(ST_SWAPUSED) / (float)status(ST_SWAPSIZE);
}

private void schedule()
{
	if (callout) {
		remove_call_out(callout);
	}

	if (interval > 0.0) {
		callout = call_out("report", interval);
	} else {
		callout = 0;
	}
}

mixed message(string msg)
{
	switch (previous_program()) {
	case LIB_WIZTOOL:
		message = msg;
		return nil;
	default:
		error(previous_program() + " is harassing me!");
	}
}

string status_message()
{
	cmd_status(nil, nil, nil);
	return message;
}

/* I/O stuff */

string query_banner(object conn)
{
	ACCESS_CHECK(previous_program() == PORTD);

	return nil;
}

int query_timeout(object conn)
{
	ACCESS_CHECK(previous_program() == PORTD);

	connection(conn);
	redirect(this_object(), nil);

	return 0;
}

object select(string input)
{
	ACCESS_CHECK(previous_program() == PORTD);

	return this_object();
}

private void redraw(object conn)
{
	conn->message("\033c\033[2J");
	conn->message("\033[14;1H\033[1m-----------------------------------------");
	conn->message("\033[21;1H-----------------------------------------");
	conn->message("\033[21m\033[15;20r\033[15;1H");
}

private void prompt(object conn)
{
	conn->message("[\033[1;34mstatus\033[21;37m] ");
}

int login(string str)
{
	object base_conn, conn;

	int trusted;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	ASSERT(str == nil);

	conn = previous_object();
	base_conn = conn;

	while (base_conn && base_conn <- LIB_USER) {
		base_conn = base_conn->query_conn();
	}

	switch(query_ip_number(base_conn)) {
	case "::1":
	case "127.0.0.1":
		trusted = 1;
		break;
	}

	connections[conn] = ({
		trusted ? 0.05 : 15.0,
		trusted,
		call_out("report", 0, conn)
	});

	redraw(conn);
	prompt(conn);

	return MODE_NOCHANGE;
}

private int printstatus(object conn)
{
	if (conn) {
		return conn->message("\0337\033[1;1H" + status_message() + "\n\0338");
	} else {
		LOGD->post_message("status", LOG_INFO, status_message());
		return 0;
	}
}

int receive_message(string str)
{
	object conn;
	string *params;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (!str) {
		catch(error("suspicious"));
		return MODE_NOCHANGE;
	}

	conn = previous_object();

	params = explode(str, " ") - ({ "" });
	
	if (sizeof(params)) switch(params[0]) {
	case "redraw":
		redraw(conn);
		break;
	case "quit":
		remove_call_out(connections[conn][2]);
		conn->message("\033c");
		return MODE_DISCONNECT;
	case "interval":
		if (sizeof(params) < 1) {
			conn->message("Usage: interval <interval>\n");
			break;
		} else {
			float interval;
			
			sscanf(params[1], "%f", interval);
			
			if (interval < 15.0 && !connections[conn][1]) {
				conn->message("Intervals less than 15 seconds\nare only allowed for local connections.\n");
				break;
			}

			remove_call_out(connections[conn][2]);

			connections[conn][0] = interval;
			connections[conn][2] = call_out("report", interval, conn);
		}
	case "":
		break;
	default:
		conn->message("Commands: interval, quit, redraw\n");
		break;
	} else {
		printstatus(conn);
	}
	
	prompt(conn);
	
	return MODE_NOCHANGE;
}

static void report(varargs object conn)
{
	int status;
	
	status = printstatus(conn);

	if (conn) {
		if (status) {
			connections[conn][2] = call_out("report",
				connections[conn][0], conn
			);
		} else {
			connections[conn][2] = call_out("report",
				1, conn
			);
		}
	} else {
		schedule();
	}
}
