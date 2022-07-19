#include <fs/fs.hpp>

void marshallList(Inode inode, uint8_t*& raw, size_t& sz) {
	// How much space do I need?
	sz = 0;

	auto l = list(inode);
	sz += sizeof(File) * l.size();
	for(auto const& x : l)
		sz += x.f.size() + 1; // + 1 because of null termination

	// Get that buffer
	raw = new uint8_t[sz + sizeof(Inode)];

	// And start copying
	uint8_t* cur = raw;
	for(auto& x : l) {
		// Copy file
		memcpy(cur, &(x.s), sizeof(File));
		cur += sizeof(File);
		// Copy name, null terminated
		memcpy(cur, x.f.c_str(), x.s.namesz + 1);
		cur += x.s.namesz + 1;
	}

	// I want last inode zeroed
	for(size_t i=0; i<sizeof(Inode); ++i)
		raw[sz+i] = 0;
}
