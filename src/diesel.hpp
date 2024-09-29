#pragma once
#include "generated_game_information.h"
#include <string>
#include <stdint.h>


class DieselGenericAllocator {
public:
	virtual ~DieselGenericAllocator() {}
	virtual __int64 allocate(unsigned long long* size) { return 0xDEADBEEF; }
	virtual void allocate_aligned() {}
	virtual void deallocate(void*) {}
	virtual void allocated_size() {}
	virtual void size() {}
	virtual void unk0() {}
	virtual void unk1() {}
};

namespace dsl {
	class Buffer {
	public:
#ifdef RAIDWW2
		std::uintptr_t _allocator;
		//DieselGenericAllocator* _allocator;
	  long long size;
		char* data;
#endif
	};
	unsigned long long hash64(
		char* k,     /* the key */
		unsigned long long length, /* the length of the key */
		unsigned long long level  /* the previous hash, or an arbitrary value */
	);
	unsigned long long hash64(const std::string& s);

	class idstring {
	public:
		unsigned long long _id;
		idstring() : _id(0) {}
		idstring(unsigned long long _id) : _id(_id) {}
		idstring(const dsl::idstring& other);
		idstring(const std::string& source);
		idstring(const char* source);

		bool operator<(const dsl::idstring& b) const;
		bool operator==(const dsl::idstring& b) const;
	};

	class ResourceID {
	public:
		dsl::idstring type;
		dsl::idstring name;
	};
}

std::string attempt_to_find_source_idstring_from_hashlist(dsl::idstring str, bool file_printable = true);

void load_hashlist();