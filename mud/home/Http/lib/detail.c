inherit "support";
inherit "/lib/string/sprint";

private string detail_box(object obj, string detail)
{
	string buffer;
	string *descriptions;
	string *words;
	string combine;
	int sz;

	buffer = "";

	if (combine = obj->query_combine(detail)) {
		buffer += "<p>Archetype combine: " + combine + "</p>\n";
	}

	if (sizeof(words = obj->query_snouns(detail))) {
		buffer += "<p>Singular nouns: " + implode(words, ", ") + "</p>\n";
	}

	if (sizeof(words = obj->query_pnouns(detail))) {
		buffer += "<p>Plural nouns: " + implode(words, ", ") + "</p>\n";
	}

	if (sizeof(words = obj->query_adjectives(detail))) {
		buffer += "<p>Adjectives: " + implode(words, ", ") + "</p>\n";
	}

	descriptions = obj->query_descriptions(detail);
	sz = sizeof(descriptions);

	if (sz) {
		int i;

		for (i = 0; i < sz; i++) {
			string description;

			description = descriptions[i];

			buffer += "<p>" + description + ": " + obj->query_description(detail, description) + "</p>\n";
		}
	}

	if (detail) {
		return oinfobox(detail, 3, buffer);
	} else {
		return oinfobox("(default)", 3, buffer);
	}
}

static string detail_text(object obj, string *details)
{
	string buffer;
	int sz, i;

	sz = sizeof(details);

	buffer = "<h2>Details</h2>\n<br />";

	for (i = 0; i < sz; i++) {
		buffer += detail_box(obj, details[i]);
	}

	return buffer;
}
