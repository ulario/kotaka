#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object *users;
	string **lists;
	int sz, i;

	lists = ({ ({ }), ({ }), ({ }) });

	send_out("User list\n");
	send_out("---------\n");

	users = GAME_USERD->query_users();
	sz = sizeof(users);

	for (i = 0; i < sz; i++) {
		lists[3 - users[i]->query_class()]
			+= ({ users[i]->query_username() });
	}

	for (i = 0; i < 3; i++) {
		if (sizeof(lists[i])) {
			int j;
			string *list;
			
			list = lists[i];
			sz = sizeof(list);
			SUBD->qsort(list, 0, sz);

			switch(i) {
			case 0: send_out("Administrators:\n"); break;
			case 1: send_out("Wizards:\n"); break;
			case 2: send_out("Players:\n"); break;
			}

			for (j = 0; j < sz; j++) {
				send_out(STRINGD->to_title(list[j]) + "\n");
			}

			send_out("\n");
		}
	}
}
