/****************************************************************************/
/***                                                                      ***/
/*** This file contains a routine to disassemble a buffer containing Z80  ***/
/*** opcodes.                                                             ***/
/***                                                                      ***/
/*** Copyright (C) Marcel de Kogel 1996,1997                              ***/
/***     You are not allowed to distribute this software commercially     ***/
/***     Please, notify me, if you make any changes to this file          ***/
/****************************************************************************/

#define RODATA_ATTR  __attribute__((section(".irom.text"))) __attribute__((aligned(4)))
#define ROMSTR_ATTR  __attribute__((section(".irom.text.romstr"))) __attribute__((aligned(4)))

#include "disasm.h"

static const char mnemonic_ed[] RODATA_ATTR = {
  #include "mnemonic_ed.data"
};

static const char mnemonic_main[] RODATA_ATTR = {
  #include "mnemonic_main.data"
};


static const char mnemonic_xx[] RODATA_ATTR = {
  #include "mnemonic_xx.data"
};

static const char mnemonic_cb[] RODATA_ATTR = {
  #include "mnemonic_cb.data"
};

static const char mnemonic_xx_cb[] RODATA_ATTR = {
  #include "mnemonic_xx_cb.data"
};


char *ICACHE_FLASH_ATTR strcat(char *to, const char *from) {
  while (*to++ != '\0')
    ;
  while ((*to++ = *from++) != '\0')
    ;
}


static char SignF(unsigned char a) { return (a & 128) ? '-' : '+'; }

static int AbsF(unsigned char a) {
  if (a & 128)
    return 256 - a;
  else
    return a;
}

/****************************************************************************/
/* Disassemble first opcode in buffer and return number of bytes it takes   */
/****************************************************************************/
int ICACHE_FLASH_ATTR Z80_Dasm(uint8_t *buffer, char *dest, uint16_t PC) {
  const char *S;
  char *r;
  int i, j, k;
  char buf[10];
  char Offset;
  i = Offset = 0;
  r = "INTERNAL PROGRAM ERROR";
  dest[0] = '\0';
  switch (buffer[i]) {
  case 0xCB:
    i++;
    S = &mnemonic_cb[buffer[i++]];
    break;
  case 0xED:
    i++;
    S = &mnemonic_ed[buffer[i++]];
    break;
  case 0xDD:
    i++;
    r = "ix";
    switch (buffer[i]) {
    case 0xcb:
      i++;
      Offset = buffer[i++];
      S = &mnemonic_xx_cb[buffer[i++]];
      break;
    default:
      S = &mnemonic_xx[buffer[i++]];
      break;
    }
    break;
  case 0xFD:
    i++;
    r = "iy";
    switch (buffer[i]) {
    case 0xcb:
      i++;
      Offset = buffer[i++];
      S = &mnemonic_xx_cb[buffer[i++]];
      break;
    default:
      S = &mnemonic_xx[buffer[i++]];
      break;
    }
    break;
    default:
    S = &mnemonic_main[buffer[i++]];
    break;
  }
  for (j = 0; S[j]; ++j) {
    switch (S[j]) {
    case 'B':
      ets_sprintf(buf, "$%02x", buffer[i++]);
      strcat(dest, buf);
      break;
    case 'R':
      ets_sprintf(buf, "$%04x", (PC + 2 + (signed char)buffer[i]) & 0xFFFF);
      i++;
      strcat(dest, buf);
      break;
    case 'W':
      ets_sprintf(buf, "$%04x", buffer[i] + buffer[i + 1] * 256);
      i += 2;
      strcat(dest, buf);
      break;
    case 'X':
      ets_sprintf(buf, "(%s%c$%02x)", r, SignF(buffer[i]), AbsF(buffer[i]));
      i++;
      strcat(dest, buf);
      break;
    case 'Y':
      ets_sprintf(buf, "(%s%c$%02x)", r, SignF(Offset), AbsF(Offset));
      strcat(dest, buf);
      break;
    case 'I':
      strcat(dest, r);
      break;
    case '!':
      ets_sprintf(dest, "db     $ed,$%02x", buffer[1]);
      return 2;
    case '@':
      ets_sprintf(dest, "db     $%02x", buffer[0]);
      return 1;
    case '#':
      ets_sprintf(dest, "db     $%02x,$cb,$%02x", buffer[0], buffer[2]);
      return 2;
    case ' ':
      k = ets_strlen(dest);
      if (k < 6)
        k = 7 - k;
      else
        k = 1;
      ets_memset(buf, ' ', k);
      buf[k] = '\0';
      strcat(dest, buf);
      break;
    default:
      buf[0] = S[j];
      buf[1] = '\0';
      strcat(dest, buf);
      break;
    }
  }
  return i;
}

