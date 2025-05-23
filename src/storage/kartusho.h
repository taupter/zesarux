/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef KARTUSHO_H
#define KARTUSHO_H

#include "cpu.h"

#define KARTUSHO_SIZE (512*1024)

extern z80_bit kartusho_enabled;

extern void kartusho_enable(void);
extern void kartusho_disable(void);

extern char kartusho_rom_file_name[];

extern z80_byte kartusho_active_bank;
extern void kartusho_press_button(void);

extern z80_byte *kartusho_memory_pointer;

#endif
