#ifndef FS_HPP
#define FS_HPP

#include <common.hpp>
#include "structures.hpp"
#include <unordered_map>
#include <string>
#include <abstract.hpp>

extern std::UUID uuid;
extern uint8_t* buffer;

extern Inode rootInode;
extern LBA rootLBA;
extern size_t rootSize;

bool readLBA(LBA lba);
uint8_t* readBytes(LBA start, size_t sz, uint8_t* hint=nullptr);
bool readPVD();

size_t findInode(const char* name);
Directory* getFileDirectory(Inode inode);
LBA getFileLBA(Inode inode);
size_t getFileSize(Inode inode);

std::unordered_map<std::string, File> list(Inode inode);

#endif
