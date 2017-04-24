# BC -> IR

## Notes

- On entering function, `alloc_local %name, int64_t` once for each local variable that is not an arg (since each value is a static constant or pointer to Value which can fit in an int64_t)
- Locals referenced as %x
- Registers referenced as %0
- Number registers 1 through n, never re-use. let IR -> ASM phase handle allocating and recycling them
- All arithmetic is done only on regs, to do on local var you must `store_reg`, `add`, `store_local`

## Instructions

### alloc_local

- Args: [%local, size]
- Effects: allocates enough space on the stack for an object of size (eg. sizeof(int64_t)), and saves pointer to %local

### store_local

- Args: [%dest_local, ($src_const | %src_local | %src_reg)]
- Effects: Puts the value of $src_const or %src_local or %src_reg in %dest_local

### store_reg

- Args: [%dest_reg, ($src_const | %src_local | %src_reg)]
- Effects: Puts the value of $src_const or %src_local or %src_reg in %dest_reg

### add

- Args: [%dest_reg, %src_reg1, %src_reg2]
