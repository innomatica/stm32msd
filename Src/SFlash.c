#include "SFlash.h"

// sector dirty flag map
#define SIZE_SECTOR_MAP				(SFLASH_NUM_SECTORS >> 5)
uint32_t sector_map[SIZE_SECTOR_MAP];
// macros for handling bits in the sector map
#define SET_SECT_FLG(k)				( sector_map[(k/32)] |= (1<<(k%32)) )
#define CLR_SECT_FLG(k)				( sector_map[(k/32)] &= ~(1<<(k%32)) )
#define TST_SECT_FLG(k)				( sector_map[(k/32)] &  (1<<(k%32)) )

/** Call this function to perform either chip erase or just set dirty flags.
 *	Setting dirty flag ensures calling EraseSector function before each
 *	WriteSector function.
 *
 *	\param flag if true EraseChip will be called, otherwise set dirty flag
 */
void SFlash_Init(bool flag)
{
	if(flag)
	{
		// whole chip erase will take some time so here it retuns immediately
		// waiting for the completion
		SFlash_EraseChip(false);
	}
	else
	{
		// set all flags to dirty
		for(int i = 0; i < SIZE_SECTOR_MAP; i++)
		{
			sector_map[i] = 0xffffffff;
		}
	}
}

/** \return true if the device is busy
 */
bool SFlash_IsBusy(void)
{
	uint8_t tx_data[2] = {SFLASH_READ_STATUS, SFLASH_NOP};
	uint8_t rx_data[2] = {0xff,};

	SFLASH_CS_LOW;
	SFLASH_TXRX(tx_data, rx_data, 2);
	SFLASH_CS_HIGH;

	if(rx_data[1] & 0x01)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/** \return true if the device is writable
 */
bool SFlash_IsWritable(void)
{
	uint8_t tx_data[2] = {SFLASH_READ_STATUS, SFLASH_NOP};
	uint8_t rx_data[2] = {0xff,};

	SFLASH_CS_LOW;
	SFLASH_TXRX(tx_data, rx_data, 2);
	SFLASH_CS_HIGH;

	if(rx_data[1] & 0x02)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/** \return status byte
 */
uint8_t SFlash_ReadStatus(void)
{
	uint8_t tx_data[2] = {SFLASH_READ_STATUS, SFLASH_NOP};
	uint8_t rx_data[2] = {0xff,};

	SFLASH_CS_LOW;
	SFLASH_TXRX(tx_data, rx_data, 2);
	SFLASH_CS_HIGH;

	return rx_data[1];
}


/** \return JEDEC bytes packed and right aligned in a uint32_t variable
 */
uint32_t SFlash_ReadJedecID()
{
	uint8_t tx_data[4] = {SFLASH_READ_JEDEC, SFLASH_NOP, SFLASH_NOP, SFLASH_NOP};
	uint8_t rx_data[4] = {0xff,};

	while(SFlash_IsBusy());

	SFLASH_CS_LOW;
	SFLASH_TXRX(tx_data, rx_data, 4);
	SFLASH_CS_HIGH;

	return ((uint32_t)(rx_data[1]<<16) + (uint32_t)(rx_data[2]<<8) + \
	        (uint32_t)(rx_data[3]));
}


/** This is a blocking call if wait is true, otherwise it returns without
 *	waiting for the completion.
 */
void SFlash_EraseChip(bool wait)
{
	int i;
	uint8_t tx_data[1];

	while(SFlash_IsBusy());

	// enable write
	tx_data[0] = SFLASH_WRITE_ENABLE;
	SFLASH_CS_LOW;
	SFLASH_TX(tx_data, 1);
	SFLASH_CS_HIGH;

	// wait until the operation is completed
	while(SFlash_IsBusy());

	// erase chip
	tx_data[0] = SFLASH_ERASE_CHIP;
	SFLASH_CS_LOW;
	SFLASH_TX(tx_data, 1);
	SFLASH_CS_HIGH;

	// initialize sector map
	for(i = 0; i < SIZE_SECTOR_MAP; i++)
	{
		sector_map[i] = 0x0;
	}

	if(wait)
	{
		// wait until the operation is completed
		while(SFlash_IsBusy());
	}
}


/**
 *	\param sector sector number to be erased
 */
void SFlash_EraseSector(uint32_t sector)
{
	uint8_t tx_data[4];
	uint32_t addr = SECT_TO_ADDR(sector);

	while(SFlash_IsBusy());

	// enable write
	tx_data[0] = SFLASH_WRITE_ENABLE;
	SFLASH_CS_LOW;
	SFLASH_TX(tx_data, 1);
	SFLASH_CS_HIGH;

	// wait until the operation is done
	while(SFlash_IsBusy());

	// erase the sector
	tx_data[0] = SFLASH_ERASE_SECT;
	tx_data[1] = (uint8_t)(addr >> 16);
	tx_data[2] = (uint8_t)(addr >> 8);
	tx_data[3] = (uint8_t)(addr >> 0);

	SFLASH_CS_LOW;
	SFLASH_TX(tx_data, 4);
	SFLASH_CS_HIGH;

	// clear dirty flag for the sector
	CLR_SECT_FLG(sector);

	// wait until the operation is completed
	while(SFlash_IsBusy());
}


/** By default, it will use fast read command (0x0b) unless user defines
 *  SFLASH_READ_DATA otherwise.
 *
 *	\param addr starting address
 *	\param buff	buffer where the output data will be stored
 *	\param len the number of bytes to read
 */
void SFlash_Read(uint32_t addr, uint8_t *buff, uint16_t len)
{
	uint8_t tx_data[5] = {SFLASH_READ_DATA,
		(uint8_t)(addr>>16), (uint8_t)(addr>>8), (uint8_t)(addr), SFLASH_NOP };

	while(SFlash_IsBusy());

	// type 1 read (normal read)
	if(SFLASH_READ_DATA == SFLASH_READ_DATA1)
	{
		SFLASH_CS_LOW;
		SFLASH_TX(tx_data, 4);
		SFLASH_RX(buff, len);
		SFLASH_CS_HIGH;
	}
	// type 2 read (fast read)
	else
	{
		SFLASH_CS_LOW;
		SFLASH_TX(tx_data, 5);
		SFLASH_RX(buff, len);
		SFLASH_CS_HIGH;
	}
}

/** It is assumed that the device supports multiple byte programming with the
 *	usual write command (0x02). If the device does not support it, then this
 *	function should be overriden.
 *
 *	This routine divides the writing operation into multiple transactions, so
 *	that in one transaction, the operation does not cross the page boundary.
 *	User can override this function as well.
 *
 *	\param addr starting address
 *	\param buff pointer for the data buffer
 *	\param len the number of bytes to be written
 */
__attribute((weak)) void SFlash_Write(uint32_t addr, uint8_t *buff, uint16_t len)
{
	uint8_t tx_data[4];
	uint32_t page_bdry;
	int wr_size;
	bool end_of_data;

	while(1)
	{
		// compute next page boundary
		page_bdry = (addr + 0x100) & 0xffffff00;

		// data write is going to pass the page boundary
		if( (addr + len) > page_bdry )
		{
			// only write until the boundary met
			wr_size = page_bdry - addr;
			// we are not done
			end_of_data = false;
		}
		else
		{
			// write all the remaining data
			wr_size = len;
			// we are done
			end_of_data = true;
		}

		while(SFlash_IsBusy());

		// enable write
		tx_data[0] = SFLASH_WRITE_ENABLE;
		SFLASH_CS_LOW;
		SFLASH_TX(tx_data, 1);
		SFLASH_CS_HIGH;

		// wait until the operation is done
		while(SFlash_IsBusy());

		// write data
		tx_data[0] = SFLASH_WRITE_DATA;
		tx_data[1] = (uint8_t)(addr>>16);
		tx_data[2] = (uint8_t)(addr>>8);
		tx_data[3] = (uint8_t)(addr);

		SFLASH_CS_LOW;
		SFLASH_TX(tx_data, 4);
		SFLASH_TX(buff, wr_size);
		SFLASH_CS_HIGH;

		// set flag
		SET_SECT_FLG(ADDR_TO_SECT(addr));

		// wait until the operation is done
		while(SFlash_IsBusy());

		if(end_of_data)
		{
			break;
		}
		else
		{
			// proceed buffer pointer
			buff = buff + wr_size;
			// proceed address
			addr = addr + wr_size;
			// decrease data size
			len = len - wr_size;
		}
	}
}

/** One sector of data is to be read
 *	\param sector the sector number to read
 *	\param buff pointer for the data buffer
 */
void SFlash_ReadSector(uint32_t sector, uint8_t *buff)
{
	SFlash_Read(SECT_TO_ADDR(sector), buff, SFLASH_SECTOR_SIZE);
}

/** One sector of data to be written
 *  \param sector the sector number to write
 *  \param buff pointer for the data buffer
 */
void SFlash_WriteSector(uint32_t sector, uint8_t *buff)
{
	// if the sector is not empty
	if(TST_SECT_FLG(sector))
	{
		// erase first
		SFlash_EraseSector(sector);
	}
	// write data
	SFlash_Write(SECT_TO_ADDR(sector), buff, SFLASH_SECTOR_SIZE);
	// set flag
	SET_SECT_FLG(sector);
}

#if UNIT_TEST

void Print_SectorMap(void)
{
	uint8_t buffer[33] = {0};

	DbgPrintf("\n\r\t\t<<sector map>>");
	for(int i = 0; i < SIZE_SECTOR_MAP; i++)
	{
		for(int j = 0; j < 32; j++)
		{
			if((sector_map[i]>>j) & 0x1)
			{
				buffer[j] = 'x';
			}
			else
			{
				buffer[j] = '.';
			}
		}

		DbgPrintf("\r\n\t%s", buffer);
	}
}

void SFlash_UnitTest(void)
{
	int i;
	bool match;
	uint8_t u8val;
	uint32_t u32val;
	uint32_t addr1 = 0x000, addr2 = 0x7812;
	uint8_t u8wbuf[SFLASH_SECTOR_SIZE];
	uint8_t u8rbuf[SFLASH_SECTOR_SIZE] = {0xff,};

	for(i = 0; i < sizeof(u8wbuf); i++)
	{
		u8wbuf[i] = (uint8_t)i;
	}

	DbgPrintf("\r\nSflash_UnitTest\r\n");

	u32val = SFlash_ReadJedecID();
	DbgPrintf("\r\n\tSFlash_ReadJedecID: %02x.%02x.%02x",
	          (uint8_t)(u32val>>16),(uint8_t)(u32val>>8),(uint8_t)(u32val));

	u8val = SFlash_ReadStatus();
	DbgPrintf("\r\n\tSFlash_ReadStatus: %02x", u8val);
	DbgPrintf("\r\n\tSFlash_IsBusy: %d", SFlash_IsBusy());
	DbgPrintf("\r\n\tSFlash_IsWritable: %d", SFlash_IsWritable());

	for(i = 0; i < 10; i++)
	{
		DbgPrintf("\r\n\tSFlash_EraseSector:%d",i);
		SFlash_EraseSector(i);
	}
	Print_SectorMap();

	// read/write test: on sector boundary
	DbgPrintf("\r\n\tSFlash_Write at: %d", addr1);
	SFlash_Write(addr1, u8wbuf, sizeof(u8wbuf));
	DbgPrintf("\r\n\tSFlash_Read at: %d", addr1);
	SFlash_Read(addr1, u8rbuf, sizeof(u8rbuf));

	match = true;
	for(i = 0; i < sizeof(u8rbuf); i++)
	{
		if(u8rbuf[i] != u8wbuf[i])
		{
			match = false;
			break;
		}
	}

	if(match)
	{
		DbgPrintf("\r\n\tData match");
	}
	else
	{
		DbgPrintf("\r\n\tData does not match!!!");
	}
	Print_SectorMap();

	// erase chip
	DbgPrintf("\r\n\tSFlash_EraseChip...");
	SFlash_EraseChip(true);
	DbgPrintf("Done");
	Print_SectorMap();

	// confirm erasure
	DbgPrintf("\r\n\tSFlash_Read at: %d", addr2);
	SFlash_Read(addr2, u8rbuf, sizeof(u8rbuf));

	match = true;
	for(i = 0; i < sizeof(u8rbuf); i++)
	{
		if(u8rbuf[i] != 0xff)
		{
			match = false;
			break;
		}
	}

	if(match)
	{
		DbgPrintf("\r\n\tData erased");
	}
	else
	{
		DbgPrintf("\r\n\tData is not erased!!!");
	}

	// read/write test: off sector
	DbgPrintf("\r\n\tSFlash_Write at: %d", addr2);
	SFlash_Write(addr2, u8wbuf, sizeof(u8wbuf));
	DbgPrintf("\r\n\tSFlash_Read at: %d", addr2);
	SFlash_Read(addr2, u8rbuf, sizeof(u8rbuf));

	match = true;
	for(i = 0; i < sizeof(u8rbuf); i++)
	{
		if(u8rbuf[i] != u8wbuf[i])
		{
			match = false;
			break;
		}
	}

	if(match)
	{
		DbgPrintf("\r\n\tData match");
	}
	else
	{
		DbgPrintf("\r\n\tData does not match!!!");
	}
	Print_SectorMap();

	// read/write sector test
	DbgPrintf("\r\n\tSFlash_WriteSector at unused sector: %d", 12);
	SFlash_WriteSector(12, u8wbuf);
	DbgPrintf("\r\n\tSFlash_ReadSector at: %d", 12);
	SFlash_ReadSector(12, u8rbuf);

	match = true;
	for(i = 0; i < sizeof(u8rbuf); i++)
	{
		if(u8rbuf[i] != u8wbuf[i])
		{
			DbgPrintf("mismatch:(%d) %d - %d",i,u8rbuf[i],u8wbuf[i]);
			match = false;
			break;
		}
	}

	if(match)
	{
		DbgPrintf("\r\n\tData match");
	}
	else
	{
		DbgPrintf("\r\n\tData does not match!!!");
	}
	Print_SectorMap();

	// read/write sector test
	DbgPrintf("\r\n\tSFlash_WriteSector at used sector: %d", 7);
	SFlash_WriteSector(7, u8wbuf);
	DbgPrintf("\r\n\tSFlash_ReadSector at: %d", 7);
	SFlash_ReadSector(7, u8rbuf);

	match = true;
	for(i = 0; i < sizeof(u8rbuf); i++)
	{
		if(u8rbuf[i] != u8wbuf[i])
		{
			DbgPrintf("mismatch:(%d) %d - %d",i,u8rbuf[i],u8wbuf[i]);
			match = false;
			break;
		}
	}

	if(match)
	{
		DbgPrintf("\r\n\tData match");
	}
	else
	{
		DbgPrintf("\r\n\tData does not match!!!");
	}
	Print_SectorMap();

	DbgPrintf("\r\n\tErasing used sectors...");
	for(i = 0; i < SFLASH_NUM_SECTORS; i++)
	{
		if(TST_SECT_FLG(i))
		{
			DbgPrintf("%d,",i);
			CLR_SECT_FLG(i);
		}
	}
	DbgPrintf("...Done");
	Print_SectorMap();

	DbgPrintf("\r\nSFlash_UnitTest Done... Get back to work");

}

#else

void SFlash_UnitTest()
{
}

#endif
