inherit "thing";

static string query_ip();

static string object_text(object obj)
{
	string subbuffer;
	string buffer;
	string name;

	subbuffer = "<p>Object owner: " + obj->query_owner() + "<br />\n";

	if (name = obj->query_object_name()) {
		subbuffer += "Object name: " + name + "<br />\n";
	}

	if (query_ip() == "127.0.0.1") {
		subbuffer += "<a href=\"destruct.lpc?obj=" + object2string(obj) + "\">Destruct<br />\n";
	}

	buffer = oinfobox("Object", 1, subbuffer);

	if (obj <- "~Thing/lib/thing") {
		buffer += thing_text(obj);

		if (query_ip() == "127.0.0.1") {
			buffer += thing_form(obj);
		}
	}

	return buffer;
}
