/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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

static mixed **blocks;

static void start_page()
{
	blocks = ({ });
}

static void add_title_block(string title, int level)
{
	blocks += ({ ({ "title", level, title }) });
}

static void add_text_block(string text)
{
	blocks += ({ ({ "text", text }) });
}

static string render_page()
{
	int i, sz;
	string buffer;

	sz = sizeof(blocks);
	buffer = "";

	for (i = 0; i <= sz; i++) {
		mixed *block;

		block = blocks[i];

		switch(block[0]) {
		case "title":
			{
				int level;
				string title;

				level = block[1];
				title = block[2];
				buffer += "<h" + level + ">" + title + "</h" + level + ">\n";
			}
			break;

		case "text":
			buffer += "<div class=\"infobox\">\n";
			buffer += block[1];
			buffer += "</div>\n";
		}
	}

	return buffer;
}
