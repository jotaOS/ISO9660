#ifndef ABSTRACT_HPP
#define ABSTRACT_HPP

#include <types>

typedef uint64_t Inode;

struct File {
	Inode inode;
	uint64_t date;
	uint32_t size;
	uint8_t flags;
	uint8_t namesz = 0;
} __attribute__((packed));

#endif
