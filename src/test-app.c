#include "reb-host.h"

char *script =
	"print {Hello Rebol!}\n"
	"print [{Now is:} now]\n"
	"list-dir\n"
;
int main(int argc, char **argv) {
	RL_Init(argc, argv);        // Initialize Core
	RL_Start(0, 0, 0);          // Loads all mezzanine code
	RL_Do_String(script, 0, 0); // Does the script
	return 0;
}

