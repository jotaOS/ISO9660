#include "fs.hpp"
#include <rpc>
#include <shared_memory>
#include <userspace/block.hpp>
#include "structures.hpp"
#include <algorithm>

std::PID block = 0;

bool readLBAs(std::SMID smid, LBA lba, size_t n) {
	if(!block) {
		block = std::resolve("block");
		if(!block)
			return false;

		bool ret = std::rpc(block,
							std::block::SELECT,
							uuid.a,
							uuid.b);
		if(!ret) {
			block = 0;
			return false;
		}
	}

	std::smAllow(smid, block);
	bool ret = std::rpc(block,
						std::block::READ,
						smid,
						lba,
						n * SECTOR_SIZE);

	return ret;
}

bool readBytes(std::SMID smid, LBA lba, size_t sz) {
	size_t sectors = (sz + SECTOR_SIZE - 1) / SECTOR_SIZE;
	return readLBAs(smid, lba, sectors);
}
