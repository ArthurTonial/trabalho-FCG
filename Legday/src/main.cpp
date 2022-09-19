/*
for release use the not D version of dll in the linker
*/

#include "MainProgram.h"
#include "MainWindow.h"

int main() {
	MainProgram::init();
	MainProgram::run();
	return 0;
}
