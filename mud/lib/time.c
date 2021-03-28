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

int idelay(int stamp, int interval, int offset)
{
	int goal;

	goal = stamp;
	goal -= (goal + interval - offset) % interval;

	if (goal <= stamp) {
		goal += interval;
	}

	return goal - stamp;
}

string ptime(int time)
{
	string c;
	string hms;
	string w;
	string m;
	string d;
	string y;

	string dt;

	c = ctime(time);

	hms = c[11 .. 18];

	w = c[0 .. 2];
	m = c[4 .. 6];
	d = c[8 .. 9];
	y = c[20 .. 23];

	if (d[0] == ' ')
		d[0] = '0';

	dt = m + " " + d + ", " + y;
	return dt + " " + hms;
}

string pmtime(mixed *mtime)
{
	int msec;
	string ms;

	msec = (int) (mtime[1] * 1000.0);
	ms = "000" + (string) msec;
	ms = ms[strlen(ms) - 3..];
	return ptime(mtime[0]) + "." + ms;
}

static string timedesc(int sec, varargs int long)
{
	if (sec < 60) {
		return sec + (long ? (sec == 1 ? " second" : " seconds") : "s");
	} else if (sec < 3600 - 60) {
		sec += 59;
		sec /= 60;
		return sec + (long ? (sec == 1 ? " minute" : " minutes") : "m");
	} else if (sec < 86400 - 3600) {
		sec += 60 * 59;
		sec /= 60 * 60;
		return sec + (long ? (sec == 1 ? " hour" : " hours") : "h");
	} else if (sec < 86400 * 6) {
		sec += 23 * 3600;
		sec /= 24 * 3600;
		return sec + (long ? (sec == 1 ? " day" : " days") : "d");
	} else if (sec < 86400 * 7 * 4) {
		sec += 86400 * 6;
		sec /= 86400 * 7;
		return sec + (long ? (sec == 1 ? " week" : " weeks") : "w");
	} else if (sec < 86400 * 335) {
		sec += 86400 * 29;
		sec /= 86400 * 30;
		return sec + (long ? (sec == 1 ? " month" : " months") : "mo");
	} else {
		sec += 86400 * 335;
		sec /= 86400 * 365;
		return sec + (long ? (sec == 1 ? " year" : " years") : "y");
	}
}
