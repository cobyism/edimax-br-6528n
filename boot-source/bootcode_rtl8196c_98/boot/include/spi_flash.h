/* SPI Flash driver
 *
 * Written by sam (sam@realtek.com)
 * 2010-05-01
 *
 */
// Init
void spi_pio_init(void);
// Probe
void spi_cp_probe(unsigned int uiChip, unsigned int uiPrnFlag);
// Read
unsigned int spi_reader(unsigned int uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
// Write
unsigned int spi_write(unsigned int uiChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer);
// Erase
unsigned int spi_sector_erase(unsigned int uiChip, unsigned int uiAddr);
unsigned int spi_block_erase(unsigned int uiChip, unsigned int uiAddr);
unsigned int spi_erase_chip(unsigned int chip);
// print unsigned char
void prnUChar(char* pcName, unsigned char* pucBuffer, unsigned int uiLen);

/************************************ for old interface ************************************/
// Init
void spi_pio_init_8198(void);
// Probe
void spi_probe();
// Read
unsigned int spi_read(unsigned int uiChip, unsigned int uiAddr, unsigned int* puiDataOut);
int flashread (unsigned long dst, unsigned int src, unsigned long length);
int flashwrite(unsigned long dst, unsigned long src, unsigned long length);
// Write
int spi_flw_image(unsigned int chip, unsigned int flash_addr_offset ,unsigned char *image_addr, unsigned int image_size);
int spi_flw_image_mio_8198(unsigned int cnt, unsigned int flash_addr_offset , unsigned char *image_addr, unsigned int image_size);
// Erase
unsigned int do_spi_block_erase(unsigned int addr);
