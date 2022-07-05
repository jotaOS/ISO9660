#include <common.hpp>
#include <rpc>
#include <shared_memory>
#include "fs/fs.hpp"
#include <unordered_map>
#include <mutex>

static std::mutex setupLock;
static bool isSetup = false;
std::UUID uuid;
bool setup(std::PID client, uint64_t uuida, uint64_t uuidb) {
	IGNORE(client);

	setupLock.acquire();
	if(isSetup) {
		setupLock.release();
		return false;
	}

	uuid = {uuida, uuidb};
	if(!readPVD()) {
		setupLock.release();
		return false;
	}

	isSetup = true;
	setupLock.release();
	return true;
}

bool connect(std::PID client, std::SMID smid) {
	setupLock.acquire();
	if(!isSetup) {
		setupLock.release();
		return false;
	}
	setupLock.release();

	return std::sm::connect(client, smid);
}

Inode getRoot(std::PID client) {
	IGNORE(client);

	setupLock.acquire();
	if(!isSetup) {
		setupLock.release();
		return 0;
	}
	setupLock.release();

	return rootInode;
}

/*Inode getInode(std::PID client) {
	if(!isSetup)
		return 0;
	if(shared.find(client) == shared.end())
		return 0;

	return findInode((char*)(shared[client]));
}*/

size_t publist(std::PID client, Inode inode, size_t page) {
	setupLock.acquire();
	if(!isSetup) {
		setupLock.release();
		return 0;
	}
	setupLock.release();

	uint8_t* remote = std::sm::get(client);
	if(!remote)
		return 0;

	uint8_t* marshalled = nullptr;
	size_t npages = 0;
	marshallList(inode, marshalled, npages);

	if(page > npages)
		return npages; // No more pages left

	memcpy(remote, marshalled+page*PAGE_SIZE, PAGE_SIZE);
	return npages;
}

size_t pubread(std::PID client, Inode inode, size_t page) {
	setupLock.acquire();
	if(!isSetup) {
		setupLock.release();
		return 0;
	}
	setupLock.release();

	uint8_t* remote = std::sm::get(client);
	if(!remote)
		return 0;

	// How big is that file?
	size_t fullsz = getFileSize(inode);
	size_t npages = (fullsz + PAGE_SIZE - 1) / PAGE_SIZE;
	if(page >= npages)
		return 0;
	// Page now is guaranteed to be in limits

	// What LBA is that?
	size_t nLBAs = (fullsz + SECTOR_SIZE - 1) / SECTOR_SIZE;
	LBA seqlba = page * 2; // Which one sequentially

	LBA startlba = getFileLBA(inode); // File start here
	LBA lba = startlba + seqlba; // Absolute LBA to read

	// Perform the read
	size_t toReadLBAs = 1;
	if(seqlba + 1 < nLBAs)
		++toReadLBAs; // Next one is in limits

	size_t hasRead = toReadLBAs * SECTOR_SIZE;
	readBytes(lba, hasRead, remote);

	// I have read "hasRead" bytes
	// However, might have read some zeros there
	// How much have I actually read?
	// This is way too verbose, but otherwise it's incomprehensible
	size_t ret = 0;
	if(toReadLBAs == 1) {
		// Case 1: only one LBA. Was it the last one?
		if(seqlba + 1 == nLBAs) {
			// Case 1.1: yep, last one; tell caller I've read the last bytes
			ret = fullsz % SECTOR_SIZE;
		} else {
			// Case 1.2: it wasn't the last one, so I've read it all
			ret = SECTOR_SIZE;
		}
	} else if(toReadLBAs == 2) {
		// Case 2: two LBAs
		if(seqlba + 2 < nLBAs) {
			// Case 2.1: second one is not the last, so I've read everything
			ret = 2 * SECTOR_SIZE;
		} else if(seqlba + 2 == nLBAs) {
			// Case 2.2: second one is the last
			ret = SECTOR_SIZE; // First one completely read
			ret += fullsz % SECTOR_SIZE;
		} else {
			// Case 2.3: first one is the last, impossible
			HALT_AND_CATCH_FIRE();
		}
	}

	// Clear the rest 👀
	memset(remote+ret, 0, PAGE_SIZE - ret);

	return ret;
}

void exportProcedures() {
	std::exportProcedure((void*)setup, 2);
	std::exportProcedure((void*)connect, 1);
	std::exportProcedure((void*)getRoot, 0);
	std::exportProcedure((void*)publist, 2);
	std::exportProcedure((void*)pubread, 2);
}
