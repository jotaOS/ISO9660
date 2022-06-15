#ifndef COMMON_HPP
#define COMMON_HPP

#include <types>
#include <abstract.hpp>

void marshallList(Inode inode, uint8_t*& raw, size_t& npages);
void exportProcedures();

#endif
