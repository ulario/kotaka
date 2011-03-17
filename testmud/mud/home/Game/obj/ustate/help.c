#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;

string *args;

private string *collect_topics(string key);

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

void set_args(string *new_args)
{
	args = new_args;
}

private void prompt()
{
	if (!introed) {
		send_out(read_file("~/data/doc/help_welcome"));
		introed = 1;
	}
	
	send_out("UlarioMUD help service: ");
}

static void do_help(string topic)
{
	string buffer;
	string *topics;
	string *aliases;
	object pager;
	int sz;
	
	string *filtered_topics;
	string *raw_topics;
	
	topic = STRINGD->replace(topic, " ", "_");
	
	raw_topics = GAME_HELPD->fetch_topics(topic);
	filtered_topics = GAME_HELPD->filter_topics(raw_topics);
	
	topics = filtered_topics;
	aliases = raw_topics - filtered_topics;
	
	if (topic != "") {
		topic += "/index";
	} else {
		topic = "index";
	}
	
	raw_topics = GAME_HELPD->fetch_topics(topic);
	filtered_topics = GAME_HELPD->filter_topics(raw_topics);

	topics |= filtered_topics;
	aliases |= raw_topics - filtered_topics;
	
	sz = sizeof(topics);

	if (sz == 0) {
		buffer = "No such topic.\n";
	} else if (sz > 1) {
		int i;
		int asz;
		
		buffer = "Ambiguous topic.\n\nChoices:\n\n\033[35m";

		for (i = 0; i < sz; i++) {
			buffer += "\t" + topics[i] + "\n";
		}
		
		buffer += "\033[0m\n";
		
		if (asz = sizeof(aliases)) {
			int i;
			
			buffer += "Aliases:\n\n\033[35m";
			
			for (i = 0; i < asz; i++) {
				buffer += "\t" + aliases[i] + "\n";
			}
			
			buffer += "\033[0m\n";
		}
		buffer += "\n";
	} else {
		string topic;
		int asz;
		
		topic = topics[0];
		buffer = "\033[1;32m" + topic + "\033[0m\n\n";
		
		if (asz = sizeof(aliases)) {
			int i;
			
			buffer += "Aliases:\n\033[35m";
			
			for (i = 0; i < asz; i++) {
				buffer += aliases[i] + "\n";
			}
			
			buffer += "\033[0m\n";
		}
		
		catch {
			buffer += read_file("~/data/help/" + topic + ".hlp");
		} : {
			buffer += "Error reading: " + topic + ".hlp";
		}
	}

	pager = clone_object("~Kotaka/obj/ustate/page");
	pager->set_text(buffer);
	swap_state(pager);
}

void begin()
{
	string topic;

	ACCESS_CHECK(previous_object() == query_user());

	if (!args) {
		args = ({ "index" });
	}

	topic = args[0];
	
	do_help(topic);
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

private string *collect_topics(string key)
{
	return GAME_HELPD->filter_topics(
		GAME_HELPD->fetch_topics(key)
	);
}
