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

size_t publistSize(std::PID client, Inode inode) {
	IGNORE(client);

	setupLock.acquire();
	if(!isSetup) {
		setupLock.release();
		return 0;
	}
	setupLock.release();

	uint8_t* marshalled = nullptr;
	size_t npages = 0;
	marshallList(inode, marshalled, npages);
	return npages;
}

bool publist(std::PID client, std::SMID smid, Inode inode) {
	setupLock.acquire();
	if(!isSetup) {
		setupLock.release();
		return false;
	}
	setupLock.release();

	auto link = std::sm::link(client, smid);
	size_t npages = link.s;
	if(!npages)
		return false;
	uint8_t* buffer = link.f;

	uint8_t* marshalled = nullptr;
	size_t mnpages = 0;
	marshallList(inode, marshalled, mnpages);

	memcpy(buffer, marshalled, std::min(npages, mnpages) * PAGE_SIZE);
	std::sm::unlink(smid);
	return true;
}

size_t pubread(std::PID client, std::SMID smid, Inode inode, size_t page, size_t n) {
	setupLock.acquire();
	if(!isSetup) {
		setupLock.release();
		return 0;
	}
	setupLock.release();

	// About the shared memory
	auto link = std::sm::link(client, smid);
	if(!link.s)
		return false;

	// About the file
	LBA lba = getFileLBA(inode) + page * 2;

	auto ret = readBytes(smid, lba, n * PAGE_SIZE);
	std::sm::unlink(smid);
	return ret;
}

void exportProcedures() {
	std::exportProcedure((void*)setup, 2);
	std::exportProcedure((void*)getRoot, 0);
	std::exportProcedure((void*)publistSize, 1);
	std::exportProcedure((void*)publist, 2);
	std::exportProcedure((void*)pubread, 4);
}
