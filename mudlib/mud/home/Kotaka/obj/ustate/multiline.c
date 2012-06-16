#include <kotaka/paths.h>

inherit LIB_USTATE;

string text;

static void create(int clone)
{
	if (clone) {
		::create();
		text = "";
	}
}

private void prompt()
{
	send_out(": ");
}

static void end()
{
	destruct_object(this_object());
}

static void receive_in(string str)
{
	if (str == ".") {
		send_in(text);
		text = "";
		return;
	} else if (strlen(str) >= 2 && str[0 .. 1] == "..") {
		str = str[1 ..];
	}

	text += str;
	text += "\n";
}

static void input_over()
{
	prompt();
}
