#include "fs.hpp"
#include <cstdio>
#include <string>

static Inode nextInode = 0;
static Inode enterDirectory(const std::string& name) {
	for(auto const& x : list(nextInode))
		if(x.f == name)
			return nextInode = x.s.inode;
	return 0;
}

// Given a path, find its "inode" (just Directory structure location)
size_t findInode(const char* cname) {
	// Get root extend
	nextInode = rootInode;

	std::string name(cname);
	// From here onwards, it is assumed the path is well formed
	// (VFS is responsable)
	// Expected something like: boot/term
	// No trailing slash!

	size_t ret = 0;

	for(auto const& x : name.split('/')) {
		ret = enterDirectory(x);
		if(!ret)
			return 0;
	}

	return ret;
}
