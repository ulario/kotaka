#include <kotaka/paths.h>
#include <kotaka/privilege.h>

int execute_command(string cmd, string args)
{
	object ustate;
	object bin;

	ACCESS_CHECK((ustate = previous_object())<-LIB_USTATE);
	bin = find_object("~/sys/bin/" + cmd);

	if (!bin) {
		return FALSE;
	}

	TLSD->set_tls_value("Game", "ustate", ustate);

	bin->main(args);

	TLSD->set_tls_value("Game", "ustate", nil);

	return TRUE;
}
