#ifndef X86_64_H
#define X86_64_H

/* Legacy prefix | REX prefix | opcode | ModeRM | SIB | displacement | immediate
 */

/* opcode format:
 *   66h: operand size prefix
 *   67h: address prefix
 *   0fh: two-byte opcode // no use
 *
 * two-byte opcode: // no use
 *   0f3ah
 */


typedef struct { // for 64-bit compatible
} rex_prefix_t;

typedef struct {
} opcode_t;

/* existence of moderm byte depends on opcode
 * mod:
 *    00: no address offset
 *    01: 8 bit address offset
 *    10: 16 or 32 bits address offset, decided by prefix
 *    11: use register only, reg field is reg index
 * rm:
 *    when mod != 11, rm is the register index
 *    otherwise:
 *      000: *(eax)      001: *(ecx)
 *      010: *(edx)      011: *(ebx)
 *      100: SIB         101: DISP
 *      110: *(esi)      111: *(edi)
 * */
typedef struct {
  uint8_t rm   : 3; // reg or memory pointed by reg
  uint8_t reg  : 3; // reg or opcode ex
  uint8_t mode : 2;
} moderm_t;

/* express special addressing mode:
 *   REG[Base] + REG[Index] * 2^scala
 */
typedef struct {
  uint8_t base  : 3;
  uint8_t scale : 3;
  uint8_t inex  : 2;
} SIB_t;

/* instruction translation
 *
 * or rd, rs, rt
 *   or
 *
 *
 *
 *
 *
 *
 *
 */

#endif
