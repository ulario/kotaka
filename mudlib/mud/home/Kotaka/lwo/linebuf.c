#include <kotaka/checkarg.h>

/* need to change this to a generic buffer */
/* need to be able to "charge up" the buffer */

string buf;

static void create(int clone)
{
	buf = "";
}

void load(string new_buf)
{
	CHECKARG(new_buf, 1, "load");
	buf = new_buf;
}

int empty()
{
	return buf == "";
}

string getline()
{
	string line;
	
	line = nil;
	
	if (sscanf(buf, "%s\n%s", line, buf)) {
		return line;
	} else if (buf != "") {
		line = buf;
		buf = "";
		return line;
	}
}
