#ifndef FS_HPP
#define FS_HPP

#include <common.hpp>
#include "structures.hpp"
#include <unordered_map>
#include <string>
#include <abstract.hpp>

extern std::UUID uuid;

extern Inode rootInode;
extern LBA rootLBA;
extern size_t rootSize;

bool readLBAs(std::SMID smid, LBA lba, size_t n);
bool readBytes(std::SMID smid, LBA start, size_t sz);
bool readPVD();

size_t findInode(const char* name);
Directory* getFileDirectory(Inode inode);
LBA getFileLBA(Inode inode);
size_t getFileSize(Inode inode);

std::unordered_map<std::string, File> list(Inode inode);

#endif
