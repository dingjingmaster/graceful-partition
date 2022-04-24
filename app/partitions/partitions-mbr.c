//
// Created by dingjing on 4/22/22.
//

#include "partitions-mbr.h"

static inline uint32_t __dos_assemble_4le(const unsigned char *p);
static inline void __dos_store_4le(unsigned char *p, unsigned int val);

struct dos_partition *mbr_get_partition(unsigned char *mbr, int i)
{
    return (struct dos_partition *)
            (mbr + MBR_PT_OFFSET + (i * sizeof(struct dos_partition)));
}

/* assemble badly aligned little endian integer */
static inline uint32_t __dos_assemble_4le(const unsigned char *p)
{
    uint32_t last_byte = p[3];

    return p[0] | (p[1] << 8) | (p[2] << 16) | (last_byte << 24);
}

static inline void __dos_store_4le(unsigned char *p, unsigned int val)
{
    assert(!(p == NULL));
    p[0] = (val & 0xff);
    p[1] = ((val >> 8) & 0xff);
    p[2] = ((val >> 16) & 0xff);
    p[3] = ((val >> 24) & 0xff);
}

unsigned int dos_partition_get_start(DosPartition* p)
{
    return __dos_assemble_4le(&(p->start_sect[0]));
}

void dos_partition_set_start(DosPartition* p, unsigned int n)
{
    __dos_store_4le(p->start_sect, n);
}

unsigned int dos_partition_get_size(DosPartition* p)
{
    return __dos_assemble_4le(&(p->nr_sects[0]));
}

void dos_partition_set_size(DosPartition* p, unsigned int n)
{
    __dos_store_4le(p->nr_sects, n);
}

void dos_partition_sync_chs(DosPartition* p, unsigned long long int part_offset, unsigned int geom_sectors, unsigned int geom_heads)
{
    unsigned long long int start = part_offset + dos_partition_get_start(p);
    unsigned long long int stop = start + dos_partition_get_size(p) - 1;
    unsigned int spc = geom_heads * geom_sectors;

    if (start / spc > 1023)
        start = spc * 1024 - 1;
    if (stop / spc > 1023)
        stop = spc * 1024 - 1;

    p->bc = (start / spc) & 0xff;
    p->bh = (start / geom_sectors) % geom_heads;
    p->bs = ((start % geom_sectors + 1) & 0x3f) |
            (((start / spc) >> 2) & 0xc0);

    p->ec = (stop / spc) & 0xff;
    p->eh = (stop / geom_sectors) % geom_heads;
    p->es = ((stop % geom_sectors + 1) & 0x3f) |
            (((stop / spc) >> 2) & 0xc0);
}

int mbr_is_valid_magic(const unsigned char *mbr)
{
    return mbr[510] == 0x55 && mbr[511] == 0xaa ? 1 : 0;
}

void mbr_set_magic(unsigned char *b)
{
    b[510] = 0x55;
    b[511] = 0xaa;
}

unsigned int mbr_get_id(const unsigned char *mbr)
{
    return __dos_assemble_4le(&mbr[440]);
}

void mbr_set_id(unsigned char *b, unsigned int id)
{
    __dos_store_4le(&b[440], id);
}

