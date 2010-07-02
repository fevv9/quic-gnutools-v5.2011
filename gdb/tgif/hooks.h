
#ifndef __HOOKS_H
#define __HOOKS_H
/*
 *
 * File: hooks.h
 *
 * Purpose: This file contains headers for the hook functions.
 *
 */
extern size_t SIZEOFBUFFER;
extern char * tcl_status_buffer;
extern char * inset_into_buffer;

//void Tgif_puts_hook(const char * linebuffer, struct ui_file * stream); [ams]

int tgif_query_hook(char *ctlstr, va_list args);
#endif
