/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <stdio.h>

/* Custom includes */
#include <codegen.h>

static const char* regs[8] = {
	"AX",	/* 0 */
	"CX",	/* 1 */
	"DX",	/* 2 */
	"BX",	/* 3 */
	"SP",	/* 4 */
	"BP",	/* 5 */
	"SI",	/* 6 */
	"DI"	/* 7 */
};

static const char* byte_regs[8] = {
	"AL",	/* 0 */
	"BL",	/* 1 */
	"CL",	/* 2 */
	"DL",	/* 3 */
	"AH",	/* 4 */
	"BH",	/* 5 */
	"CH",	/* 6 */
	"DH"	/* 7 */
};

/* Swap endianness */
uint16_t swap(uint16_t word)
{
	return (word >> 8) | (word << 8);
}

uint16_t read_word(CompileTarget* c, int offset)
{
	uint8_t low = c->code[offset + 1] & 0xFF;
	uint8_t high = c->code[offset] & 0xFF;
	uint16_t swapped = ((high << 8) + low) & 0xFFFF;
	return swap(swapped);
}

int disassemble_instruction(CompileTarget* c, int offset)
{
	printf("0x%04X  ", offset + LOAD);	/* Print our current offset */

	uint8_t byte = c->code[offset];
	switch (byte) {
		case 0x03: {	/* ADD r16, r16 */
			uint8_t modrm = c->code[offset + 1];
			printf("%02X%02X          ", byte, modrm);
			int dst = (modrm & 0x38) >> 3;
			int src = modrm & 7;
			printf("ADD %s, %s\n", regs[dst], regs[src]);
			offset += 2;
			break;
		}
		case 0x0B: {	/* OR r16, [imm16] */
			uint8_t modrm = c->code[offset + 1];
			uint16_t addr = read_word(c, offset + 2);
			printf("%02X%02X%04X      ", byte, modrm, swap(addr));
			int dst = (modrm & 0x38) >> 3;
			printf("OR %s, [0x%04X]\n", regs[dst], addr);
			offset += 4;
			break;
		}
		case 0x0F: {	/* Various Jcc rel16 */
			uint8_t op = c->code[offset + 1];
			uint16_t rel = read_word(c, offset + 2);
			uint16_t addr = offset + 4 + rel + LOAD;
			printf("%02X%02X%04X      ", byte, op, swap(rel));
			switch (op) {
				case 0x84: {
					printf("JZ  near  0x%04X\n", addr);
					break;
				}
				case 0x85: {
					printf("JNE near  0x%04X\n", addr);
					break;
				}
				case 0x8D: {
					printf("JGE near  0x%04X\n", addr);
				}
			}
			offset += 4;
			break;
		}
		case 0x25: {	/* AND AX, imm16 */
			uint16_t imm = read_word(c, offset + 1);
			printf("%02X%04X        ", byte, swap(imm));
			printf("AND AX, %d (0x%04X)\n", imm, imm);
			offset += 3;
			break;
		}
		case 0x2B: {	/* SUB r16, r16 */
			uint8_t modrm = c->code[offset + 1];
			printf("%02X%02X          ", byte, modrm);
			int dst = (modrm & 0x38) >> 3;
			int src = modrm & 7;
			printf("SUB %s, %s\n", regs[dst], regs[src]);
			offset += 2;
			break;
		}
		case 0x33: {	/* XOR r16, r16 */
			uint8_t modrm = c->code[offset + 1];
			printf("%02X%02X          ", byte, modrm);
			int dst = (modrm & 0x38) >> 3;
			int src = modrm & 7;
			printf("XOR %s, %s\n", regs[dst], regs[src]);
			offset += 2;
			break;
		}
		case 0x3B: {	/* CMP r16, r16 */
			uint8_t modrm = c->code[offset + 1];
			printf("%02X%02X          ", byte, modrm);
			int dst = (modrm & 0x38) >> 3;
			int src = modrm & 7;
			printf("CMP %s, %s\n", regs[dst], regs[src]);
			offset += 2;
			break;
		}
		case 0x3D: {	/* CMP AX, imm16 */
			uint16_t imm = read_word(c, offset + 1);
			printf("%02X%04X        ", byte, swap(imm));
			printf("CMP AX, %d (0x%04X)\n", imm, imm);
			offset += 3;
			break;
		}
		/* Man I love C */
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57: {	/* PUSH r16 */
			printf("%02X            ", byte);
			printf("PUSH %s\n", regs[byte - 0x50]);
			offset += 1;
			break;
		}
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F: {	/* POP r16 */
			printf("%02X            ", byte);
			printf("POP %s\n", regs[byte - 0x58]);
			offset += 1;
			break;
		}
		case 0x72: {	/* JC rel8 */
			int8_t rel = c->code[offset + 1];
			uint16_t target = offset + 2 + rel + LOAD;
			printf("%02X%02X          ", byte, (uint8_t)rel);
			printf("JC  short 0x%04X\n", target);
			offset += 2;
			break;
		}
		case 0x74: {	/* JZ rel8 */
			int8_t rel = c->code[offset + 1];
			uint16_t target = offset + 2 + rel + LOAD;
			printf("%02X%02X          ", byte, (uint8_t)rel);
			printf("JZ  short 0x%04X\n", target);
			offset += 2;
			break;
		}
		case 0x75: {	/* JNE rel8 */
			int8_t rel = c->code[offset + 1];
			uint16_t target = offset + 2 + rel + LOAD;
			printf("%02X%02X          ", byte, (uint8_t)rel);
			printf("JNE short 0x%04X\n", target);
			offset += 2;
			break;
		}
		case 0x7F: {	/* JG rel8 */
			int8_t rel = c->code[offset + 1];
			uint16_t target = offset + 2 + rel + LOAD;
			printf("%02X%02X          ", byte, (uint8_t)rel);
			printf("JG  short 0x%04X\n", target);
			offset += 2;
			break;
		}
		case 0x85: {	/* TEST r16, r16 */
			uint8_t modrm = c->code[offset + 1];
			printf("%02X%02X          ", byte, modrm);
			int dst = (modrm & 0x38) >> 3;
			int src = modrm & 7;
			printf("TEST %s, %s\n", regs[dst], regs[src]);
			offset += 2;
			break;
		}
		case 0x87: {	/* XCHG r16, r16 */
			uint8_t modrm = c->code[offset + 1];
			printf("%02X%02X          ", byte, modrm);
			int dst = (modrm & 0x38) >> 3;
			int src = modrm & 7;
			printf("XCHG %s, %s\n", regs[dst], regs[src]);
			offset += 2;
			break;
		}
		case 0x89: {	/* MOV [r/m16], r16 */
			uint8_t modrm = c->code[offset + 1];
			int src = (modrm & 0x38) >> 3;

			/* Loading into [BX] */
			if ((modrm & 0xC0) == 0 && (modrm & 7) == 7) {
				printf("%02X%02X          ", byte, modrm);
				printf("MOV [BX], %s\n", regs[src]);
				offset += 2;
				break;
			}

			/* We are loading into immediate address */
			uint16_t addr = read_word(c, offset + 2);
			printf("%02X%02X%04X      ", byte, modrm, swap(addr));
			printf("MOV [0x%04X], %s\n", addr, regs[src]);
			offset += 4;
			break;
		}
		case 0x8B: {	/* MOV r16, r16 or MOV r16, [r/m16] */
			uint8_t modrm = c->code[offset + 1];
			int dst = (modrm & 0x38) >> 3;

			/* It is reg to reg */
			if (modrm & 0xC0) {
				printf("%02X%02X          ", byte, modrm);
				int src = modrm & 7;
				printf("MOV %s, %s\n", regs[dst], regs[src]);
				offset += 2;
				break;
			}

			/* It is loading from [BX] */
			if ((modrm & 7) == 7) {
				printf("%02X%02X          ", byte, modrm);
				printf("MOV %s, [BX]\n", regs[dst]);
				offset += 2;
				break;
			}

			/* Or from immediate value */
			uint16_t addr = read_word(c, offset + 2);
			printf("%02X%02X%04X      ", byte, modrm, swap(addr));
			printf("MOV %s, [0x%04X]\n", regs[dst], addr);

			offset += 4;
			break;
		}
		case 0x90: {	/* NOP */
			printf("%02X            ", byte);
			printf("NOP\n");
			offset += 1;
			break;
		}
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97: {	/* XCHG AX, r16 */
			printf("%02X            ", byte);
			printf("XCHG AX, %s\n", regs[byte - 0x90]);
			offset += 1;
			break;
		}
		case 0xAA: {	/* STOSB */
			printf("%02X            ", byte);
			printf("STOSB\n");
			offset += 1;
			break;
		}
		case 0xAB: {	/* STOSW */
			printf("%02X            ", byte);
			printf("STOSW\n");
			offset += 1;
			break;
		}
		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7: {	/* MOV r/m8, imm8 */
			int reg = byte - 0xB0;
			uint8_t imm = c->code[offset + 1];
			printf("%02X%02X          ", byte, imm);
			printf("MOV %s, %d (0x%02X)\n", byte_regs[reg],
				imm, imm);
			offset += 2;
			break;
		}
		case 0xAC: {	/* LODSB */
			printf("%02X            ", byte);
			printf("LODSB\n");
			offset += 1;
			break;
		}
		case 0xC1: {	/* SHR r16, imm8 or SHL r16, imm8 */
			uint8_t modrm = c->code[offset + 1];
			uint8_t imm = c->code[offset + 2];
			int dst = modrm & 7;
			printf("%02X%02X%04X      ", byte, modrm, swap(imm));
			if ((modrm & 0x38) >> 3 == 4)
				printf("SHL %s, %d\n", regs[dst], imm);
			else
				printf("SHR %s, %d\n", regs[dst], imm);
			offset += 3;
			break;
		}
		case 0xC3: {	/* RET */
			printf("%02X            ", byte);
			printf("RET\n");
			offset += 1;
			break;
		}
		case 0xC7: {	/* MOV r/m16, imm16 */
			uint8_t modrm = c->code[offset + 1];
			int dst = modrm & 7;
			uint16_t imm = read_word(c, offset + 2);
			if ((modrm & 0xC0) == 0) {
				uint16_t val = read_word(c, offset + 4);
				printf("%02X%02X%04X%04X  ", byte, modrm,
					swap(imm), swap(val));
				printf("MOV [0x%04X], %d (0x%04X)\n", imm,
					val, val);
				offset += 6;
				break;
			}
			printf("%02X%02X%04X      ", byte, modrm, swap(imm));
			printf("MOV %s, %d (0x%04X)\n", regs[dst], imm, imm);
			offset += 4;
			break;
		}
		case 0xCD: {	/* INT n */
			uint8_t imm = c->code[offset + 1];
			printf("%02X%02X          ", byte, imm);
			printf("INT 0x%02X\n", imm);
			offset += 2;
			break;
		}
		case 0xE8: {	/* CALL rel16 */
			int16_t rel = read_word(c, offset + 1);
			uint16_t target = offset + 3 + rel + LOAD;
			printf("%02X%04X        ", byte, swap(rel));
			printf("CALL 0x%04X\n", target);
			offset += 3;
			break;
		}
		case 0xE9: {	/* JMP NEAR */
			int16_t rel = read_word(c, offset + 1);
			uint16_t target = offset + 3 + rel + LOAD;
			printf("%02X%04X        ", byte, swap(rel));
			printf("JMP near  0x%04X\n", target);
			offset += 3;
			break;
		}
		case 0xEB: {	/* JMP SHORT */
			int8_t rel = c->code[offset + 1];
			uint16_t target = offset + 2 + rel + LOAD;
			printf("%02X%02X          ", byte, (uint8_t)rel);
			printf("JMP short 0x%04X\n", target);
			offset += 2;
			break;
		}
		case 0xF3: {	/* REP prefix */
			printf("%02X            REP Prefix\n", byte);
			offset += 1;
			break;
		}
		case 0xF7: {	/* MUL r16 or DIV r16 */
			uint8_t modrm = c->code[offset + 1];
			printf("%02X%02X          ", byte, modrm);
			int src = modrm & 7;
			if ((modrm & 0x38) >> 3 == 4)
				printf("MUL %s\n", regs[src]);
			else
				printf("DIV %s\n", regs[src]);
			offset += 2;
			break;
		}
		case 0xFE: {	/* INC r/m8 */
			uint8_t modrm = c->code[offset + 1];

			int src = modrm & 7;
			printf("%02X%02X          ", byte, modrm);
			printf("INC %s\n", byte_regs[src]);

			offset += 2;
			break;
		}
		case 0xFF: {	/* INC r/m16 or DEC r/m16 or CALL r16 */
			uint8_t modrm = c->code[offset + 1];

			/* CALL r16 */
			if ((modrm & 0x38) >> 3 == 2) {
				int src = modrm & 7;
				printf("%02X%02X          ", byte, modrm);
				printf("CALL %s\n", regs[src]);
				offset += 2;
				break;
			}

			/* DEC r16 */
			if ((modrm & 0x38) >> 3 == 1) {
				int src = modrm & 7;
				printf("%02X%02X          ", byte, modrm);
				printf("DEC %s\n", regs[src]);
				offset += 2;
				break;
			}

			/* INC r16 */
			if ((modrm & 0xC0) != 0) {
				int src = modrm & 7;
				printf("%02X%02X          ", byte, modrm);
				printf("INC %s\n", regs[src]);
				offset += 2;
				break;
			}

			/* INC [mem16] */
			uint16_t addr = read_word(c, offset + 2);
			printf("%02X%02X%04X      ", byte, modrm, swap(addr));
			printf("INC [0x%04X]\n", addr);
			offset += 4;
			break;
		}
		default:
			printf("%02X            <Unknown instruction>\n", byte);
			offset++;
	}

	return offset;
}

void disassemble(CompileTarget* c)
{
	int strings_len = read_word(c, 1) + 3 - RUNTIMELEN;
	/* Instructions in x86 are variable length so we do it this way */
	for (int i = 0; i < c->length; /* nothing */) {
		if (i == 3) {	/* Runtime! Don't disassemble */
			printf("0x%04X  ", i + LOAD);
			printf("(* ==== BASIC RUNTIME ==== *)\n");
			i = RUNTIMELEN;
			continue;
		}
		if (i == RUNTIMELEN && strings_len != 0) { /* String table */
			printf("0x%04X  ", i + LOAD);
			printf("(* ==== STRINGS TABLE ==== *)\n");
			i = RUNTIMELEN + strings_len;	/* Where we jump */
			continue;
		}
		i = disassemble_instruction(c, i);
	}
}
