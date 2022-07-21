#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <types>

#define SECTOR_SIZE 2048
#define MIN_SIZE_DIRECTORY_ENTRY 32

typedef uint32_t LBA;

union Directory {
	struct {
		uint8_t length;
		uint8_t extLength;
		LBA extend;
		uint32_t extendBE; // Big-endian version
		uint32_t extlen;
		uint32_t extlenBE; // Big-endian
		char date[7];
		uint8_t flags;
		uint8_t interleaveUnitSize;
		uint8_t interleaveGap;
		uint16_t volSeq;
		uint16_t volSeqBE; // Big-endian
		uint8_t lenId;
		char id[];
	} __attribute__((packed));
	uint8_t padding[256];
};

#define PVD_ROOT_OFFSET 156

// Volume Descriptors
namespace VD {
	struct Types {
		enum {
			BOOT_RECORD,
			PRIMARY_VOLUME_DESCRIPTOR,
			// Don't care about the rest
			TERMINATOR = 255,
		};
	};

	union PVD {
		struct {
			uint8_t type; // Always 0x01
			char identifier[5]; // Always "CD001"
			uint8_t version; // Always 0x01
			uint8_t unused0;
			char sysIdentifier[32];
			char volIdentifier[32];
			uint64_t unused1;
			uint64_t volSpacesize; // LBAs in which the volume is recorded
			uint8_t unused2[32];
			uint32_t volSetSize;
			uint32_t volSeqNumber;
			uint32_t lbaSize; // Should be 2KiB, but it might not be
			uint64_t pathTableSize; // Don't care
			uint32_t typeLPathTable; // Don't care
			uint32_t optTypeLPathTable; // Don't care
			uint32_t typeMPathTable; // Don't care
			uint32_t optTypeMPathTable; // Don't care
			Directory root; // Do care, and very much
			/*char volSetIdentifier[128];
			char pubIdentifier[128];
			char dataPrepIdentifier[128];
			char appIdentifier[128];*/
		} __attribute__((packed));
		uint8_t padding[2048];
	};
};

#endif
