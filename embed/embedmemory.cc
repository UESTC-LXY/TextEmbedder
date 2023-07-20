// embedmanager.cc
// 4/26/2014 jichi

#include "config.h"
#include "embed/embedmemory.h" 

#define DEBUG "embedmemory"
#include "sakurakit/skdebug.h"
 
EmbedMemory_MMAP::EmbedMemory_MMAP() { 
	SECURITY_ATTRIBUTES allAccess = std::invoke([] // allows non-admin processes to access kernel objects made by admin processes
		{
			static SECURITY_DESCRIPTOR sd = {};
			InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
			SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
			return SECURITY_ATTRIBUTES{ sizeof(SECURITY_ATTRIBUTES), &sd, FALSE };
		});
	filemap = CreateFileMappingW(INVALID_HANDLE_VALUE, &allAccess, PAGE_EXECUTE_READWRITE, 0, 4096, (LUNASHAREDMEM + std::to_wstring(GetCurrentProcessId())).c_str());
	sharedcell = (Cell*) MapViewOfFile(filemap, FILE_MAP_ALL_ACCESS | FILE_MAP_EXECUTE, 0, 0, 4096);
	memset(sharedcell, 0, 4096); 
	 
}
EmbedMemory_MMAP::~EmbedMemory_MMAP() {
	UnmapViewOfFile(filemap);
}
int EmbedMemory_MMAP::cellCount() const {
	return 1;
}
bool EmbedMemory_MMAP::isAttached() const {
	return sharedcell->connect==1;
}
#include"stringutil.h"
std::wstring EmbedMemory_MMAP::dataLanguage(int i) const {
	std::string s= sharedcell->language;
	return StringToWideString(s, CP_ACP);
}
std::wstring EmbedMemory_MMAP::dataText(int i) const {
	return sharedcell->text;
}
UINT64 EmbedMemory_MMAP::dataHash(int i) const {
	return sharedcell->hash;
}
UINT8 EmbedMemory_MMAP::dataRole(int i) const {
	return sharedcell->role;
}
UINT8 EmbedMemory_MMAP::dataStatus(int i) const {
	return sharedcell->status;
}

// EOF
