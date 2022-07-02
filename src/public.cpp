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
	LBA lba = getFileLBA(inode);
	lba += page / 2;

	// Perform the read
	size_t toReadLBAs = 1;
	if(lba + 1 < nLBAs)
		++toReadLBAs; // Next one is in limits

	size_t hasRead = toReadLBAs * SECTOR_SIZE;
	readBytes(lba, hasRead, remote);

	// I have read "hasRead" bytes
	// However, might have read some zeros there
	// How much have I actually read?
	size_t ret = 0;
	if(toReadLBAs == 2)
		ret += SECTOR_SIZE; // First one is surely full
	if(lba + toReadLBAs < nLBAs)
		ret += SECTOR_SIZE; // Second one was full too
	else
		ret += fullsz % SECTOR_SIZE;

	return ret;
}

void exportProcedures() {
	std::exportProcedure((void*)setup, 2);
	std::exportProcedure((void*)connect, 1);
	std::exportProcedure((void*)getRoot, 0);
	std::exportProcedure((void*)publist, 2);
	std::exportProcedure((void*)pubread, 2);
}
