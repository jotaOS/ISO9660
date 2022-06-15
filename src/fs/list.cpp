#include "fs.hpp"

// Given an inode, find all its files and subdirectories
std::unordered_map<std::string, File> list(Inode inode) {
	Directory* dir = getFileDirectory(inode);
	if(!dir)
		return {};

	LBA extend = dir->extend;
	size_t sz = dir->extlen;

	std::unordered_map<std::string, File> ret;

	uint8_t* tmp = readBytes(extend, sz);
	size_t offset = 0;
	while(true) {
		Directory* dir = (Directory*)(tmp + offset);
		if(!(dir->length))
			break; // That was all :)

		size_t len = dir->lenId;
		if(len == 1 && dir->id[0] == '\0') {
			// '.' entry, ignore
		} else if(len == 1 && dir->id[0] == '\1') {
			// '..' entry, ignore
		} else {
			// Regular entry
			// Do some shenanigans to get a std::string
			char* tmpp = new char[len + 1];
			tmpp[len] = '\0';
			memcpy(tmpp, dir->id, len);
			std::string str(tmpp);
			delete [] tmpp;

			// If it's a file, it ends in ";1"
			str = str.split(';')[0];
			if(str.size() < len) {
				// It's definitely a file. In which case, it might not have
				//   an extension, which is mandatory. It will hence end in '.',
				//   so remove that last one.
				if(str.size() && str[str.size()-1] == '.')
					str.pop_back();
			}

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
			file.namesz = str.size();
			ret[str] = file;
		}

		offset += dir->length;
	}
	delete [] tmp;

	return ret;
}
