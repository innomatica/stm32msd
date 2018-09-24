#ifndef __SFLASH_H
#define __SFLASH_H
/**
 * 	\file
 *	\author	<a href="http://www.innomatic.ca">innomatic</a>
 *	\brief	Serial FLASH memory interface
 * 	\copyright <a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/"><img alt="Creative Commons Licence" style="border-width:0" src="https://i.creativecommons.org/l/by-nc/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/">Creative Commons Attribution-NonCommercial 4.0 International License</a>.
 *
 *	\warning If there is no device in the system or if the device is not
 *	functional then any of the write functions may hang the system.
 */

#include <stdbool.h>
#include <stdint.h>
#include "board.h"

/// Serial FLASH command definitions
#define SFLASH_WRITE_STATUS			0x01
#define SFLASH_WRITE_DATA			0x02
#define SFLASH_READ_DATA1			0x03
#define SFLASH_READ_DATA2			0x0b
#define SFLASH_WRITE_DISABLE		0x04
#define SFLASH_READ_STATUS			0x05
#define SFLASH_WRITE_ENABLE			0x06
#define SFLASH_ERASE_4KB			0x20
#define SFLASH_ERASE_SECT			SFLASH_ERASE_4KB
#define SFLASH_ERASE_32KB			0x52
#define SFLASH_ERASE_CHIP			0x60
#define SFLASH_READ_JEDEC			0x9f
#define SFLASH_NOP					0xff

/// most serial FLASH devices support fast version of read command
#ifndef SFLASH_READ_DATA
#define SFLASH_READ_DATA			SFLASH_READ_DATA2
#endif

/// most devices receives multiple bytes in one WRITE transaction
#ifndef SFLASH_WRITE_PAGE
#define SFLASH_WRITE_PAGE			SFLASH_WRITE_DATA
#endif

/// concept of page is not universal
#define SFLASH_PAGE_ORD				(8)
#define SFLASH_PAGE_SIZE			(1<<(SFLASH_PAGE_ORD))		// 256

/// however most devices support 4KB erase
#define SFLASH_SECTOR_ORD			(12)
#define SFLASH_SECTOR_SIZE			(1<<(SFLASH_SECTOR_ORD))	// 4KB
#define SECT_TO_ADDR(x)				((x)<<(SFLASH_SECTOR_ORD))
#define ADDR_TO_SECT(x)				((x)>>(SFLASH_SECTOR_ORD))

/// Initialize device
void SFlash_Init(bool flag);
/// Check if the device is busy
bool SFlash_IsBusy(void);
/// Checi if the device is writable
bool SFlash_IsWritable(void);
/// Read the status byte
uint8_t SFlash_ReadStatus(void);
/// Read JEDEC ID
uint32_t SFlash_ReadJedecID(void);
/// Erase the chip
void SFlash_EraseChip(bool wait);
/// Erase one sector
void SFlash_EraseSector(uint32_t sector);
/// Read data
void SFlash_Read(uint32_t addr, uint8_t *buff, uint16_t len);
/// Write data
void SFlash_Write(uint32_t addr, uint8_t *buff, uint16_t len) __attribute((weak));
/// Read data by sector unit
void SFlash_ReadSector(uint32_t sector, uint8_t *buff);
/// Write data by sector unit
void SFlash_WriteSector(uint32_t sector, uint8_t *buff);
/// Unit test
void SFlash_UnitTest(void);

#endif // __SFLASH_H
