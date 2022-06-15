#include "fs.hpp"

Directory* getFileDirectory(Inode inode) {
	if(!readLBA(inode / SECTOR_SIZE))
		return nullptr;

	return (Directory*)(buffer + (inode % SECTOR_SIZE));
}

LBA getFileLBA(Inode inode) {
	Directory* dir = getFileDirectory(inode);
	if(!dir)
		return 0;
	return dir->extend;
}

size_t getFileSize(Inode inode) {
	Directory* dir = getFileDirectory(inode);
	if(!dir)
		return 0;
	return dir->extlen;
}
