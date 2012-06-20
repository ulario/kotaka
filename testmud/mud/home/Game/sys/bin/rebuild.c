#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	OBJECTD->klib_recompile();
	OBJECTD->global_recompile();
}
