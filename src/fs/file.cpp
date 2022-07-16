#include "fs.hpp"
#include <shared_memory>

static std::SMID smid = 0;
static uint8_t* buffer = nullptr;
Directory* getFileDirectory(Inode inode) {
	if(!smid) {
		smid = std::smMake();
		buffer = (uint8_t*)std::smMap(smid);
	}

	if(!readLBAs(smid, inode / SECTOR_SIZE, 1))
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
