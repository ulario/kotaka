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
#include <kernel/kernel.h>
#include <kernel/user.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit conn LIB_CONN;

object manager;
string buffer;
int flushing;

static void create(int clone)
{
	if (clone) {
		conn::create("telnet");
		buffer = "";
	}
}

void connect(string addr, int port)
{
	ACCESS_CHECK(SYSTEM());

	if (sscanf(addr, "%*d.%*d.%*d.%*d") != 4) {
		error("Malformed IP address");
	}

	if (BAND->check_siteban(addr)) {
		error("Cannot connect to sitebanned IP");
	}

	::connect(addr, port);
}

void set_manager(object new_manager)
{
	ACCESS_CHECK(SYSTEM());

	manager = new_manager;
}

object query_manager()
{
	ACCESS_CHECK(SYSTEM());

	return manager;
}

static void connect_failed(int refused)
{
	if (manager) {
		catch {
			manager->connect_failed(refused);
		}
	}

	destruct_object(this_object());
}

/*
 * NAME:	open()
 * DESCRIPTION:	open the connection
 */
static int open()
{
    ::open(allocate(DRIVER->query_tls_size()));
    return FALSE;
}

/*
 * NAME:	close()
 * DESCRIPTION:	close the connection
 */
static void close(int dest)
{
    ::close(allocate(DRIVER->query_tls_size()), dest);
}

/*
 * NAME:	timeout()
 * DESCRIPTION:	connection timed out
 */
static void timeout()
{
    ::timeout(allocate(DRIVER->query_tls_size()));
}

/*
 * NAME:	add_to_buffer()
 * DESCRIPTION:	do this where an error is allowed to happen
 */
private void add_to_buffer(mixed *tls, string str)
{
    catch {
	buffer += str;
    } : error("Binary connection buffer overflow");
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	forward a message to listeners
 */
static void receive_message(string str)
{
    int mode, len;
    string head, pre;
    mixed *tls;

    add_to_buffer(tls = allocate(DRIVER->query_tls_size()), str);

    while (this_object() &&
	   (mode=query_mode()) != MODE_BLOCK && mode != MODE_DISCONNECT) {
	if (mode != MODE_RAW) {
	    if (sscanf(buffer, "%s\r\n%s", str, buffer) != 0 ||
		sscanf(buffer, "%s\n%s", str, buffer) != 0) {
		while (sscanf(str, "%s\b%s", head, str) != 0) {
		    while (sscanf(head, "%s\x7f%s", pre, head) != 0) {
			len = strlen(pre);
			if (len != 0) {
			    head = pre[0 .. len - 2] + head;
			}
		    }
		    len = strlen(head);
		    if (len != 0) {
			str = head[0 .. len - 2] + str;
		    }
		}
		while (sscanf(str, "%s\x7f%s", head, str) != 0) {
		    len = strlen(head);
		    if (len != 0) {
			str = head[0 .. len - 2] + str;
		    }
		}

		::receive_message(tls, str);
	    } else {
		break;
	    }
	} else {
	    if (strlen(buffer) != 0) {
		str = buffer;
		buffer = "";
		::receive_message(tls, str);
	    }
	    break;
	}
    }
}

/*
 * NAME:	set_mode()
 * DESCRIPTION:	set the connection mode
 */
void set_mode(int mode)
{
    if (KERNEL() || SYSTEM() || previous_object() == manager) {
	::set_mode(mode);
	if (!flushing && mode == MODE_RAW && strlen(buffer) != 0) {
	    call_out("flush", 0);
	    flushing = TRUE;
	}
    } else {
	error("Access denied");
    }
}

/*
 * NAME:	flush()
 * DESCRIPTION:	flush the input buffer after a switch to binary mode
 */
static void flush()
{
    string str;

    flushing = FALSE;
    if (query_mode() == MODE_RAW && strlen(buffer) != 0) {
	str = buffer;
	buffer = "";
	::receive_message(allocate(DRIVER->query_tls_size()), str);
    }
}

/*
 * NAME:	message_done()
 * DESCRIPTION:	called when output is completed
 */
static void message_done()
{
	::message_done(allocate(DRIVER->query_tls_size()));
}
