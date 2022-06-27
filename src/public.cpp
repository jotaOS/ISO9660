#include <common.hpp>
#include <rpc>
#include <shared_memory>
#include "fs/fs.hpp"
#include <unordered_map>

static bool isSetup = false;
std::UUID uuid;
bool setup(std::PID client, uint64_t uuida, uint64_t uuidb) {
	IGNORE(client);

	if(isSetup)
		return false;

	uuid = {uuida, uuidb};
	if(!readPVD())
		return false;

	isSetup = true;
	return true;
}

static std::unordered_map<std::PID, uint8_t*> shared;

bool connect(std::PID client, std::SMID smid) {
	if(!isSetup)
		return false;

	// Already connected?
	if(!std::smRequest(client, smid))
		return false;

	auto ptr = std::smMap(smid);
	if(!ptr)
		return false;

	// TODO: unmap previous, release SMID
	shared[client] = (uint8_t*)ptr;
	return true;
}

Inode getRoot(std::PID client) {
	if(!isSetup)
		return 0;
	if(shared.find(client) == shared.end())
		return 0;

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
	if(!isSetup)
		return 0;
	if(shared.find(client) == shared.end())
		return 0;
	uint8_t* remote = shared[client];

	uint8_t* marshalled = nullptr;
	size_t npages = 0;
	marshallList(inode, marshalled, npages);

	if(page > npages)
		return npages; // No more pages left

	memcpy(remote, marshalled+page*PAGE_SIZE, PAGE_SIZE);
	return npages;
}

size_t pubread(std::PID client, Inode inode, size_t page) {
	if(!isSetup)
		return 0;
	if(shared.find(client) == shared.end())
		return 0;
	uint8_t* remote = shared[client];

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
