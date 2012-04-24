#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/property.h>

#include <type.h>

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

/* random number generation */

float rnd()
{
	/* use 30 bits of randomness */
	return ldexp((float)random(1 << 30), -30);
}

/* object */

object query_common_container(object a, object b)
{
	mapping la, ra;
	object le, re;
	
	le = a;
	re = b;
	
	la = ([ ]);
	ra = ([ ]);
	
	while (le && re) {
		la[le] = 1;
		ra[re] = 1;

		if (la[re]) {
			return re;
		}

		if (ra[le]) {
			return le;
		}

		le = le->_F_query_environment();
		re = re->_F_query_environment();
	}

	while (le) {
		if (ra[le]) {
			return le;
		}

		le = le->_F_query_environment();
	}

	while (re) {
		if (la[re]) {
			return re;
		}

		re = re->_F_query_environment();
	}
}
