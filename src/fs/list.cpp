#include "fs.hpp"
#include <shared_memory>

// Given an inode, find all its files and subdirectories
std::unordered_map<std::string, File> list(Inode inode) {
	Directory* dir = getFileDirectory(inode);
	if(!dir)
		return {};

	LBA extend = dir->extend;
	size_t sz = dir->extlen;

	std::unordered_map<std::string, File> ret;

	size_t npages = NPAGES(sz);
	std::SMID smid = std::smMake(npages);
	uint8_t* buffer = (uint8_t*)std::smMap(smid);
	if(!readBytes(smid, extend, sz)) {
		std::munmap(buffer, npages);
		std::smDrop(smid);
		return {};
	}

	size_t offset = 0;
	while(true) {
		Directory* dir = (Directory*)(buffer + offset);
		if(!(dir->length))
			break; // That was all :)

		union {
				uint8_t date[7];
				uint64_t raw;
		} u;
		memcpy(u.date, dir->date, 7);

		File file;
		file.inode = extend * SECTOR_SIZE + offset;
		file.date = u.raw;
		file.size = dir->extlen;
		file.flags = dir->flags;

		size_t len = dir->lenId;
		if(len == 1 && dir->id[0] == '\0') {
			// '.' entry (rename)
			file.namesz = 1;
			ret["."] = file;
		} else if(len == 1 && dir->id[0] == '\1') {
			// '..' entry (rename)
			file.namesz = 2;
			ret[".."] = file;
		} else {
			// Regular entry
			// Do some shenanigans to get a std::string
			char* tmp = new char[len + 1];
			tmp[len] = '\0';
			memcpy(tmp, dir->id, len);
			std::string str(tmp);
			delete [] tmp;

			// If it's a file, it ends in ";1"
			str = str.split(';')[0];
			if(str.size() < len) {
				// It's definitely a file. In which case, it might not have
				//   an extension, which is mandatory. It will hence end in '.',
				//   so remove that last one.
				if(str.size() && str[str.size()-1] == '.')
					str.pop_back();
			}

			file.namesz = str.size();
			ret[str] = file;
		}

		offset += dir->length;
	}

	std::munmap(buffer, npages);
	std::smDrop(smid);
	return ret;
}
