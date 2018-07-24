#ifndef gdt_hpp
#define gdt_hpp

#include <cstdint>
#include <vector>


struct RawGDTEntry64 {
    uint16_t limit;
    uint16_t base_lo;
    uint8_t base_mid;
    uint16_t attrs;
    uint8_t base_hi;
} __attribute__((packed));

struct RawGDTEntry128 {
    uint16_t limit;
    uint16_t base_lo;
    uint8_t base_mid;
    uint16_t attrs;
    uint8_t base_hi;
    uint32_t base_uhi;
    uint32_t reseved;
} __attribute__((packed));


struct GDTEntry {
    uint64_t base = 0;
    uint32_t limit = 0;
    uint32_t access = 0;
    bool isTSS = false;
    
    RawGDTEntry64 makeShort();
    RawGDTEntry128 makeLong();
    
    static GDTEntry getCode(int priv);
    static GDTEntry getData(int priv);
};

class GDT {
private:
    char* memRegion;
    std::vector<GDTEntry> entries;
    size_t size;
public:
    GDT();
    void* getMemRegion() const
    { return memRegion; }
    size_t getSize() const
    { return size; }
    void addEntry(GDTEntry entry);
    void writeToMemory();
    ~GDT();
};






#endif /* gdt_hpp */
