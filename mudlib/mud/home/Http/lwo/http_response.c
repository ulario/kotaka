/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2014  Raymond Jennings
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

int status_code;
string status_message;
string content_type;

static void create(int clone)
{
}

void set_status(int code, string message)
{
	status_code = code;
	status_message = message;
}

string generate_header()
{
	string buffer;
	int close;
	string *headers;

	if (!status_code) {
		error("Status code not set");
	}

	buffer = "HTTP/1.1 " + status_code + " " + status_message + "\r\n";
	buffer += "Content-Type: " + content_type + "\r\n";

	switch(status_code / 100) {
	case 3:
	case 4:
	case 5:
		buffer += "Cache-Control: no-cache\r\n";
	}

	buffer += "Connection: close\r\n";
	buffer += "\r\n";

	return buffer;
}
