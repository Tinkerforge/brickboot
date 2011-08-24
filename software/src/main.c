/* brickboot
 * Copyright (C) 2011 Olaf Lüke <olaf@tinkerforge.com>
 *
 * main.c: Bootloader based on Atmel example flash bootloader
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "applet.h"

#include <bricklib/drivers/board/board_lowlevel.h>
#include <pio/pio.h>
#include <wdt/wdt.h>
#include <flash/flashd.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define STACK_SIZE (0x100)

struct _Mailbox {
    uint32_t command;
    uint32_t status;

    union {
        struct {
            uint32_t comType;
            uint32_t traceLevel;
            uint32_t bank;
        } inputInit;

        struct {
            uint32_t memorySize;
            uint32_t bufferAddress;
            uint32_t bufferSize;
            struct {
                uint16_t lockRegionSize;
                uint16_t numbersLockBits;
            } memoryInfo;
        } outputInit;

        struct {
            uint32_t bufferAddr;
            uint32_t bufferSize;
            uint32_t memoryOffset;
        } inputWrite;

        struct {
            uint32_t bytesWritten;
        } outputWrite;

        struct {
            uint32_t bufferAddr;
            uint32_t bufferSize;
            uint32_t memoryOffset;
        } inputRead;

        struct {
            uint32_t bytesRead;
        } outputRead;

        struct {
            uint32_t sector;
        } inputLock;

        struct {
            uint32_t sector;
        } inputUnlock;

        struct {
            uint32_t action;
            uint32_t bitsOfNVM;
        } inputGPNVM;

        struct {
            uint32_t bufferAddr;
        } inputReadUniqueID;
    } argument;
};

// End of program space (code + data)
extern uint32_t end;

// Size of the buffer used for read/write operations in bytes
static uint32_t bufferSize;
// Communication type with SAM-BA GUI
static uint32_t comType;

static uint32_t flashBaseAddr;
static uint32_t flashBaseAddrInit;
static uint32_t flashSize;
static uint32_t flashPageSize;
static uint32_t flashNbLockBits;
static uint32_t flashLockRegionSize;

static Pin led_blue = PIN_LED_BLUE;
static Pin led_red = PIN_LED_RED;

int main(int argc, char **argv) {
    struct _Mailbox *pMailbox = (struct _Mailbox *) argv;

    uint32_t bytesToWrite, bufferAddr, memoryOffset;
    uint32_t l_start, l_end;
    uint32_t *pActualStart = NULL;
    uint32_t *pActualEnd = NULL;
    uint8_t error;

    // Disable watchdog
    WDT_Disable(WDT);
    PIO_Configure(&led_blue, 1);
    PIO_Configure(&led_red, 1);
    PIO_Set(&led_red);

    // INIT:
    if(pMailbox->command == APPLET_CMD_INIT) {
    	low_level_init();
        // Save communication link type
        comType = pMailbox->argument.inputInit.comType;

        flashBaseAddr       = IFLASH_ADDR;
        flashBaseAddrInit   = IFLASH_ADDR;
        flashSize           = IFLASH_SIZE;
        flashPageSize       = IFLASH_PAGE_SIZE;
        flashNbLockBits     = IFLASH_NB_OF_LOCK_BITS;
        flashLockRegionSize = IFLASH_LOCK_REGION_SIZE;

        // flash accesses must be 4 bytes aligned
        pMailbox->argument.outputInit.bufferAddress = ((uint32_t) &end);

        bufferSize = IRAM_SIZE                         // sram size
                     - (((uint32_t) &end) - IRAM_ADDR) // program size
                     - STACK_SIZE;                     // stack size at the end

        // integer number of pages can be contained in each buffer
        // operation is : buffersize -= bufferSize % flashPageSize
        // modulo can be done with a mask since flashpagesize is a power
        // of two integer
        bufferSize -= (bufferSize & (flashPageSize - 1));
        pMailbox->argument.outputInit.bufferSize = bufferSize;

        pMailbox->argument.outputInit.memorySize = flashSize;
        pMailbox->argument.outputInit.memoryInfo.lockRegionSize = flashLockRegionSize;
        pMailbox->argument.outputInit.memoryInfo.numbersLockBits = flashNbLockBits;

        pMailbox->status = APPLET_SUCCESS;
    }

    // WRITE:
    else if(pMailbox->command == APPLET_CMD_WRITE) {
    	PIO_Clear(&led_red);
        memoryOffset  = pMailbox->argument.inputWrite.memoryOffset;
        bufferAddr    = pMailbox->argument.inputWrite.bufferAddr;
        bytesToWrite  = pMailbox->argument.inputWrite.bufferSize;

        // Check the giving sector have been locked before.
        if(FLASHD_IsLocked(flashBaseAddr + memoryOffset,
                           flashBaseAddr + memoryOffset + bytesToWrite) != 0) {
            pMailbox->argument.outputWrite.bytesWritten = 0;
            pMailbox->status = APPLET_PROTECT_FAIL;
            goto exit;
        }

        // Write data
        if(FLASHD_Write(flashBaseAddr + memoryOffset,
                        (const void *)bufferAddr, bytesToWrite) != 0) {
            pMailbox->argument.outputWrite.bytesWritten = 0;
            pMailbox->status = APPLET_WRITE_FAIL;
            goto exit;
        }

        pMailbox->argument.outputWrite.bytesWritten = bytesToWrite;
        pMailbox->status = APPLET_SUCCESS;
        PIO_Set(&led_red);
    }

    // READ:
    else if(pMailbox->command == APPLET_CMD_READ) {
        memoryOffset = pMailbox->argument.inputRead.memoryOffset;
        bufferAddr   = pMailbox->argument.inputRead.bufferAddr;
        bufferSize   = pMailbox->argument.inputRead.bufferSize;

        // read data
        memcpy((void *)bufferAddr,
               (void *)(flashBaseAddr + memoryOffset),
               bufferSize);

        pMailbox->argument.outputRead.bytesRead = bufferSize;
        pMailbox->status = APPLET_SUCCESS;
    }

    // FULL ERASE:
    else if(pMailbox->command == APPLET_CMD_FULL_ERASE) {
        // Check if at least one page has been locked
        if(FLASHD_IsLocked(flashBaseAddr, flashBaseAddr + flashSize) != 0) {
            pMailbox->status = APPLET_PROTECT_FAIL;
            goto exit;
        }

        // Implement the erase all command
        if(FLASHD_Erase(flashBaseAddr) != 0) {
            pMailbox->status = APPLET_ERASE_FAIL;
            goto exit;
        }

        pMailbox->status = APPLET_SUCCESS;
    }

    // LOCK SECTOR:
    else if(pMailbox->command == APPLET_CMD_LOCK) {
        l_start = (pMailbox->argument.inputLock.sector * flashLockRegionSize) + flashBaseAddr;
        l_end = l_start + flashLockRegionSize;

        if(FLASHD_Lock(l_start, l_end, pActualStart, pActualEnd) != 0) {
            pMailbox->status = APPLET_PROTECT_FAIL;
            goto exit;
        }

        pMailbox->status = APPLET_SUCCESS;
    }

    // UNLOCK SECTOR:
    else if(pMailbox->command == APPLET_CMD_UNLOCK) {
        l_start = (pMailbox->argument.inputLock.sector * flashLockRegionSize) + flashBaseAddr;
        l_end = l_start + flashLockRegionSize;

        if(FLASHD_Unlock(l_start, l_end, pActualStart, pActualEnd) != 0) {
            pMailbox->status = APPLET_UNPROTECT_FAIL;
            goto exit;
        }

        pMailbox->status = APPLET_SUCCESS;
    }

    // GPNVM :
    else if(pMailbox->command == APPLET_CMD_GPNVM) {
        if(pMailbox->argument.inputGPNVM.action == 0) {
            error = FLASHD_ClearGPNVM(pMailbox->argument.inputGPNVM.bitsOfNVM);
        } else {
            error = FLASHD_SetGPNVM(pMailbox->argument.inputGPNVM.bitsOfNVM);
        }

        if(error != 0) {
            pMailbox->status = APPLET_FAIL;
            goto exit;
        }

        pMailbox->status = APPLET_SUCCESS;
        PIO_Set(&led_blue);
    }

    // READ UNIQUE ID :
    else if (pMailbox->command == APPLET_CMD_READ_UNIQUE_ID) {
        if(FLASHD_ReadUniqueID((uint32_t*)(pMailbox->argument.inputReadUniqueID.bufferAddr)) != 0) {
            pMailbox->status = APPLET_FAIL;
            goto exit;
        }

        pMailbox->status = APPLET_SUCCESS;
    }

exit:
    // Acknowledge the end of command
    // Notify the host application of the end of the command processing
    pMailbox->command = ~(pMailbox->command);

    return 0;
}

