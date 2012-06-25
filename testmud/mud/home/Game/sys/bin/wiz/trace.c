#include <kernel/user.h>
#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object conn;

	send_out("Connection chain trace:\n");

	conn = query_user();

	while (conn) {
		send_out(object_name(conn) + "\n");

		if (conn <- LIB_USER) {
			conn = conn->query_conn();
		} else {
			break;
		}
	}
}
