
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "bmdread.h"
#include "openfile.h"

#include "addons/exportTexture.h"
#include "addons/export3ds.h"

using namespace std;

void setStartupText(const std::string& s)
{
	cout << s << endl;
}

void warn(const char* msg, ...)
{
	va_list argList;
	va_start(argList, msg);
	char buff[161];
	vsnprintf(buff, 161, msg, argList);
	va_end(argList);
	cout << buff << endl;
}

int main(int argc, char **argv) {
	if(argc < 2)
		return -1;

	OpenedFile* f = openFile(argv[1]);
	if(f == 0)
		return -2;

	BModel* mod = loadBmd(f->f);

	exportAs3ds(*mod, string(argv[1]) + string(".3ds"));
	delete mod;
}
