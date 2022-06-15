#include <cstdio>
#include <common.hpp>
#include <fs/fs.hpp>

bool setup(std::PID client, uint64_t uuida, uint64_t uuidb);

extern "C" void _start(size_t isBootstrapping) {
	exportProcedures();
	std::enableRPC();

	/*
		There is one instance of this service per filesystem.

		In bootstrapping, that is, when setting an usable environment from the
		Live CD, this process is exported so that VFS can connect to it.

		Further instances are not published, because VFS spawns them on demand,
		  and therefore knows the PID.
	*/
	if(isBootstrapping)
		std::publish("_bsISO");

	std::halt();
}
