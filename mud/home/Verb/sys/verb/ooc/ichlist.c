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
#include <kotaka/paths/intermud.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;
inherit "~System/lib/string/char";
inherit "~System/lib/string/align";
inherit "/lib/string/format";

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string name;
	string *list;
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You have insufficient access to list i3 channels.\n");
		return;
	}

	if (!find_object(INTERMUDD)) {
		send_out("IntermudD is offline.\n");
		return;
	}

	list = INTERMUDD->query_channels();

	if (!list) {
		send_out("Intermud is down.\n");
		return;
	}

	switch(roles["raw"]) {
	case "-m":
		{
			int i;
			int sz;
			mapping muds;
			string *mlist;

			muds = ([ ]);
			sz = sizeof(list);

			for (i = 0; i < sz; i++) {
				string channel;
				string mud;
				int filter;

				channel = list[i];
				({ mud, filter }) = INTERMUDD->query_channel(channel);

				if (!muds[mud]) {
					muds[mud] = ({ channel });
				} else {
					muds[mud] += ({ channel });
				}
			}

			mlist = map_indices(muds);
			sz = sizeof(mlist);

			for (i = 0; i < sz; i++) {
				string mud;
				string *clist;
				int j, sz2;

				mud = mlist[i];
				send_out(mud + ":\n");

				clist = muds[mud];
				sz2 = sizeof(clist);

				for (j = 0; j < sz2; j++) {
					send_out("    " + clist[j] + "\n");
				}
			}
		}
		break;

	case "-v":
		{
			int i;
			int sz;
			int max;

			sz = sizeof(list);

			for (i = 0; i < sz; i++) {
				string channel;

				channel = list[i];

				if (strlen(channel) > max) {
					max = strlen(channel);
				}
			}

			send_out(lalign("Channel", max) + "   Owner" + "\n");
			send_out(lalign("-------", max) + "   -----" + "\n");

			for (i = 0; i < sz; i++) {
				string channel;
				string mud;
				mixed dummy;

				channel = list[i];
				({ mud, dummy }) = INTERMUDD->query_channel(channel);

				send_out(lalign(channel, max) + " - " + mud + "\n");
			}
		}
		break;

	default:
		{
			object telnet;
			int width;

			telnet = user->query_telnet_obj();

			width = 80;

			if (telnet) {
				if (telnet->query_naws_active()) {
					width = telnet->query_naws_width();
				}
			}

			send_out(wordwrap(implode(list, ", "), width) + "\n");
		}

		break;
	}
}
