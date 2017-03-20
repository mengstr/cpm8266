#ifndef _ESPINCLUDES_H
#define _ESPINCLUDES_H

#include "ets_sys.h"
#include "stdint.h"

#define RODATA_ATTR __attribute__((section(".irom.text"))) __attribute__((aligned(4)))
#define ROMSTR_ATTR __attribute__((section(".irom.text.romstr"))) __attribute__((aligned(4)))

typedef struct espconn espconn;

// void *ets_memcpy(void *dest, const void *src, size_t n);
void uart_div_modify(int no, unsigned int freq);
//int os_random();
char *ets_strcpy(char *dest, const char *src);
char *ets_strncpy(char *dest, const char *src, size_t n);
char *ets_strstr(const char *haystack, const char *needle);
int atoi(const char *nptr);
int ets_memcmp(const void *s1, const void *s2, size_t n);
int ets_printf(const char *fmt, ...);
int ets_sprintf(char *str, const char *format, ...) __attribute__((format(printf, 2, 3)));
int ets_str2macaddr(void *, void *);
int ets_strcmp(const char *s1, const char *s2);
int ets_strncmp(const char *s1, const char *s2, int len);
int os_printf_plus(const char *format, ...) __attribute__((format(printf, 1, 2)));
int os_printf(const char *format, ...) __attribute__((format(printf, 1, 2)));
int os_snprintf(char *str, size_t size, const char *format, ...)  __attribute__((format(printf, 3, 4)));
int rand(void);
size_t ets_strlen(const char *s);
uint32 system_get_time();
uint8 wifi_get_opmode(void);

//void *ets_memcpy(void *dest, const void *src, size_t n);
void *ets_memset(void *s, int c, size_t n);

void ets_bzero(void *s, size_t n);
void ets_delay_us(int ms);
void ets_install_putc1(void *routine);
void ets_isr_attach(int intr, void *handler, void *arg);
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);
void ets_timer_arm_new(ETSTimer *a, int b, int c, int isMstimer);
//void ets_timer_arm_new(volatile os_timer_t *a, int b, int c, int isMstimer);
void ets_timer_disarm(ETSTimer *a);
//void ets_timer_disarm(volatile os_timer_t *a);
void ets_timer_setfn(ETSTimer *t, ETSTimerFunc *fn, void *parg);
//void ets_timer_setfn(volatile os_timer_t *t, ETSTimerFunc *fn, void *parg);

//void ets_update_cpu_frequency(int freqmhz);

void *pvPortMalloc(size_t xWantedSize);
void *pvPortZalloc(size_t);
void *vPortMalloc(size_t xWantedSize);
void pvPortFree(void *ptr);
void vPortFree(void *ptr);

#endif
