// Adapted from the analogous file in the RISC-V proxy kernel
// in riscv-tools/riscv-pk
#include "devicetree.h"
#include "encoding.h"
#include "util.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define ntohl(x) __builtin_bswap32(x)

// Global values set by parse_device_tree
uintptr_t mem_size;
uint32_t num_harts;
static uintptr_t max_hart_id;

static uint64_t fdt_read_uint64(uint32_t* addr) {
  return ((uint64_t)ntohl(addr[0]) << 32) | ntohl(addr[1]);
}

static void fdt_handle_cpu(const char* isa, uint32_t* reg_addr, uint32_t reg_len)
{
  int xlen = sizeof(long) * 8;

  uintptr_t* base_addr = (uintptr_t*)(uintptr_t)fdt_read_uint64(reg_addr);
  uintptr_t hart_id = *(uintptr_t*)(base_addr + CSR_MHARTID);
  num_harts++;
  max_hart_id = MAX(max_hart_id, hart_id);
}

static void fdt_handle_mem(uint32_t* reg_addr, uint32_t reg_len)
{
  uint64_t base = fdt_read_uint64(reg_addr);
  uint64_t size = fdt_read_uint64(reg_addr+2);
  mem_size = size;
}

// This code makes the following assumptions about FDTs:
// - They are trusted and don't need to be sanitized
// - All addresses and sizes are 64 bits (we don't parse #address-cells etc)

static uint32_t* parse_node(uint32_t* token, char* strings)
{
  const char* nodename = (const char*)token, *s, *dev_type = 0, *isa = 0;
  uint32_t reg_len = 0, *reg_addr = 0;
  token = (uint32_t*)nodename + strlen(nodename)/4+1;

  while (1) switch (ntohl(*token)) {
    case FDT_PROP: {
      token++;
      uint32_t len = ntohl(*token++);
      const char* name = strings + ntohl(*token++);
      if (strcmp(name, "device_type") == 0) {
        dev_type = (char*)token;
      } else if (strcmp(name, "isa") == 0) {
        isa = (char*)token;
      } else if (strcmp(name, "reg") == 0) {
        reg_len = len;
        reg_addr = token;
      }
      token += (len+3)/4;
      continue;
    }
    case FDT_BEGIN_NODE:
      token = parse_node(token+1, strings);
      continue;
    case FDT_END_NODE:
      goto out;
    case FDT_NOP:
      continue;
    default:
      ;
  }

out:
  if (dev_type && strcmp(dev_type, "cpu") == 0)
    fdt_handle_cpu(isa, reg_addr, reg_len);
  else if (dev_type && strcmp(dev_type, "memory") == 0)
    fdt_handle_mem(reg_addr, reg_len);

  return token+1;
}

void parse_device_tree(int cid)
{
  static volatile int done = 0;

  if (cid == 0) {
    struct fdt_header* hdr = (struct fdt_header*)read_csr(miobase);
    char* strings = (char*)hdr + ntohl(hdr->off_dt_strings);
    uint32_t* root = (uint32_t*)((char*)hdr + ntohl(hdr->off_dt_struct));
    while (ntohl(*root++) != FDT_BEGIN_NODE);
    parse_node(root, strings);
    done = 1;
  } else {
    while (!done)
      ;
  }

  __sync_synchronize();
}
