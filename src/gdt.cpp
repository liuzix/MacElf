#include "gdt.hpp"

#define DEFAULT_GDT_SIZE 4096

GDT::GDT() : size(DEFAULT_GDT_SIZE) {
    memRegion = (char*)valloc(DEFAULT_GDT_SIZE);
    memset(memRegion, 0, DEFAULT_GDT_SIZE);
}

void GDT::addEntry(GDTEntry entry) {
    entries.push_back(entry);
}

void GDT::writeToMemory() {
    size_t offset = 8;
    for (GDTEntry& entry: entries) {
        size_t len = entry.isTSS ? 16 : 8;
        void* ptr = nullptr;
        if (entry.isTSS) {
            ptr = new RawGDTEntry128(entry.makeLong());
        } else {
            ptr = new RawGDTEntry64(entry.makeShort());
        }
        memcpy(memRegion + offset, ptr, len);
        offset += len;
        if (offset >= DEFAULT_GDT_SIZE) {
            throw "GDT space not enough!";
        }
    }
}

GDT::~GDT() {
    free(memRegion);
}

RawGDTEntry64 GDTEntry::makeShort() {
    RawGDTEntry64 ret = {
        .limit = (uint16_t)limit,
        .base_lo = (uint16_t)(base & 0xFFFF),
        .base_mid = (uint8_t)((base & 0xFF0000) >> 16),
        .attrs = (uint16_t)(access & 0xFF),
        .base_hi = (uint8_t)((base & 0xFF000000) >> 24),
    };
    return ret;
}

RawGDTEntry128 GDTEntry::makeLong() {
    RawGDTEntry128 ret = {
        .limit = (uint16_t)limit,
        .base_lo = (uint16_t)(base & 0xFFFF),
        .base_mid = (uint8_t)((base & 0xFF0000) >> 16),
        .attrs = (uint16_t)access,
        .base_hi = (uint8_t)((base & 0xFF000000) >> 24),
        .base_uhi = (uint32_t)(base >> 32L)
    };
    return ret;
}

GDTEntry GDTEntry::getCode(int priv) {
    GDTEntry ret = {
        .base = 0,
        .limit = 0,
        .access = 0,
        .isTSS = false
    };
    ret.access |= (1 << 1);    // read/writable
    ret.access |= (0b11 << 3); // type = code
    ret.access |= (priv << 5); // privilege
    ret.access |= (1 << 7);    // present
    ret.access |= (1 << 13);   // long mode
    
    return ret;
}

GDTEntry GDTEntry::getData(int priv) {
    GDTEntry ret = {
        .base = 0,
        .limit = 0,
        .access = 0,
        .isTSS = false
    };
    ret.access |= (1 << 1);    // read/writable
    ret.access |= (0b10 << 3); // type = code
    ret.access |= (priv << 5); // privilege
    ret.access |= (1 << 7);    // present
    
    return ret;
}
