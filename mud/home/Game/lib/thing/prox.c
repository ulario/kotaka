string environment_detail;
object prox;
string prep;

void set_environment_detail(string new_environment_detail)
{
	environment_detail = new_environment_detail;
}

string query_environment_detail()
{
	return environment_detail;
}

void set_prox(object new_prox)
{
	prox = new_prox;
}

object query_prox()
{
	return prox;
}

void set_prep(string new_prep)
{
	prep = new_prep;
}

string query_prep()
{
	return prep;
}

mapping prox_save()
{
	return ([
		"environment_detail": environment_detail,
		"prox": prox,
		"prep": prep
	]);
}

void prox_load(mapping data)
{
	if (data) {
		environment_detail = data["environment_detail"];
		prox = data["prox"];
		prep = data["prep"];
	} else {
		environment_detail = nil;
		prox = nil;
		prep = nil;
	}
}
