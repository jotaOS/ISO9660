#include "fs.hpp"
#include <rpc>
#include <shared_memory>
#include <userspace/block.hpp>
#include "structures.hpp"
#include <algorithm>

std::PID block = 0;

uint8_t* buffer = nullptr;;
bool readLBA(LBA lba) {
	if(!block) {
		block = std::resolve("block");
		if(!block)
			return false;

		std::SMID smid = std::smMake();
		buffer = (uint8_t*)std::smMap(smid);
		std::smAllow(smid, block);
		bool result = std::rpc(block, std::block::CONNECT, smid);
		if(!result)
			return false;
	}

	return std::rpc(block,
					std::block::READ,
					uuid.a,
					uuid.b,
					lba,
					SECTOR_SIZE);
}

uint8_t* readBytes(LBA lba, size_t sz, uint8_t* hint) {
	uint8_t* ret = hint ? hint : new uint8_t[sz];
	uint8_t* cur = ret;

	size_t sectors = (sz + SECTOR_SIZE - 1) / SECTOR_SIZE;
	while(sectors--) {
		if(!readLBA(lba)) {
			delete [] ret;
			return nullptr;
		}

		size_t copied = std::min(sz, (size_t)SECTOR_SIZE);
		memcpy(cur, buffer, copied);

		cur += copied;
		sz -= copied;
		++lba;
	}

	return ret;
}
