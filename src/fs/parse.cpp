#include "fs.hpp"
#include <shared_memory>

union {
	char raw[SECTOR_SIZE];
	VD::PVD pvd;
} pvd;

Inode rootInode = 0;
LBA rootLBA = 0;
size_t rootSize = 0;

bool readPVD() {
	std::SMID smid = std::smMake();
	uint8_t* buffer = (uint8_t*)std::smMap(smid);

	// Start looking through volume descriptors
	size_t lba = 0x10;
	while(true) {
		if(!readLBAs(smid, lba, 1)) {
			std::munmap(buffer);
			std::smDrop(smid);
			return false;
		}

		uint8_t type = *buffer; // First byte is type
		switch(type) {
		case VD::Types::PRIMARY_VOLUME_DESCRIPTOR:
			// Nice
			memcpy(pvd.raw, buffer, SECTOR_SIZE);

			rootInode = lba * SECTOR_SIZE + PVD_ROOT_OFFSET;
			rootLBA = pvd.pvd.root.extend;
			rootSize = pvd.pvd.root.extlen;

			std::munmap(buffer);
			std::smDrop(smid);
			return true;
		case VD::Types::TERMINATOR:
			// Tough luck
			std::munmap(buffer);
			std::smDrop(smid);
			return false;
		};

		++lba;
	}
}
