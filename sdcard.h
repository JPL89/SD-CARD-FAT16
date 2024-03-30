// @File		FAT16.H	 		
// @Author		JOSIMAR PEREIRA LEITE
// @country		Brazil
// @Date		29/03/24
//
//
// Copyright (C) 2024 JOSIMAR PEREIRA LEITE
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
#ifndef FAT16_H
#define FAT16_H

#define MMC_CLK PORTCbits.RC0
#define MMC_DI PORTCbits.RC1
#define MMC_DO PORTCbits.RC2
#define MMC_CS PORTCbits.RC3

#define MMC_CLK_TRIS TRISCbits.RC0
#define MMC_DI_TRIS TRISCbits.RC1
#define MMC_DO_TRIS TRISCbits.RC2
#define MMC_CS_TRIS TRISCbits.RC3

#define CMD0	(0x40)	// 0
#define CMD1	(0x41)	// 1
#define CMD2	(0x42)	// 2
#define CMD3	(0x43)	// 3
#define CMD4	(0x44)	// 4
#define CMD5	(0x45)	// 5
#define CMD6	(0x46)	// 6
#define CMD7	(0x47)	// 7
#define CMD8	(0x48)	// 8
#define CMD9	(0x49)	// 9
#define CMD10	(0x4A)	// 10
#define CMD11	(0x4B)	// 11
#define CMD12	(0x4C)	// 12
#define CMD13	(0x4D)	// 13
#define CMD14	(0x4E)	// 14
#define CMD15	(0x4F)	// 15
#define CMD16	(0x50)	// 16
#define CMD17	(0x51)	// 17
#define CMD18	(0x52)	// 18
#define CMD19	(0x53)	// 19
#define CMD20	(0x54)	// 20
#define CMD21	(0x55)	// 21
#define CMD22	(0x56)	// 22
#define CMD23	(0x57)	// 23
#define CMD24	(0x58)	// 24
#define CMD25	(0x59)	// 25
#define CMD26	(0x5A)	// 26
#define CMD27	(0x5B)	// 27
#define CMD28	(0x5C)	// 28
#define CMD29	(0x5D)	// 29
#define CMD30	(0x5E)	// 30
#define CMD31	(0x5F)	// 31
#define CMD32	(0x60)	// 32
#define CMD33	(0x61)	// 33
#define CMD34	(0x62)	// 34
#define CMD35	(0x63)	// 35
#define CMD36	(0x64)	// 36
#define CMD37	(0x65)	// 37
#define CMD38	(0x66)	// 38
#define CMD39	(0x67)	// 39
#define CMD40	(0x68)	// 40
#define CMD41	(0xE9)  // 41
#define CMD42	(0x6A)	// 42
#define CMD43	(0x6B)	// 43
#define CMD44	(0x6C)	// 44
#define CMD45	(0x6D)	// 45
#define CMD46	(0x6E)	// 46
#define CMD47	(0x6F)	// 47
#define CMD48	(0x70)	// 48
#define CMD49	(0x71)	// 49
#define CMD50	(0x72)	// 50
#define CMD51	(0x73)	// 51
#define CMD52	(0x74)	// 52
#define CMD53	(0x75)	// 53
#define CMD54	(0x76)	// 54
#define CMD55	(0x77)	// 55
#define CMD56	(0x78)	// 56
#define CMD57	(0x79)	// 57
#define CMD58	(0x7A)	// 58
#define CMD59	(0x7B)	// 59

void SPI_write(unsigned char data)
{
    unsigned char mask = 0x80;
    
    for(mask = 0x80; mask; mask >>= 1)
    {
        if(data & mask) MMC_DI = 1; else MMC_DI = 0;
            
        __asm__ __volatile__("nop");
        MMC_CLK = 1;
        __asm__ __volatile__("nop");
        MMC_CLK = 0;
        __asm__ __volatile__("nop");
    }
}

unsigned char SPI_read(void)
{ 
    unsigned char mask = 0;
    unsigned char data = 0;
    
    for(mask = 0x80; mask; mask >>= 1) {
        
        data <<= 1;           
        if(MMC_DO) data |= 0x01;
        
        __asm__ __volatile__("nop");
        MMC_CLK = 1;
        __asm__ __volatile__("nop");
        MMC_CLK = 0;
        __asm__ __volatile__("nop");
    }    
    return (data);
}

unsigned char SD_command(unsigned char cmd, unsigned long arg)
{      
    unsigned char res = 0;
    
    SPI_write(cmd);
    SPI_write((unsigned char)(arg >> 24));
    SPI_write((unsigned char)(arg >> 16));
    SPI_write((unsigned char)(arg >> 8));
    SPI_write((unsigned char)(arg));
    
    if (cmd == CMD0) 
        SPI_write( 0x95 );
    else if (cmd == CMD8) 
        SPI_write( 0x87 );
    
    while( ( res = SPI_read() ) == 0xFF);
    
    return res;
}

void read_single_block ( unsigned long block_address, unsigned char *buf )
{
    unsigned int i;

    block_address = block_address << 9;

    MMC_CS = 0;
   
    // CMD17: READ SIGLE BLOCK
    while ( 0x00 != SD_command ( CMD17, block_address ) ); 
    while ( 0xfe != SPI_read());
    for ( i = 0; i < 512; i ++ ) buf [ i ] = SPI_read();
    
    //SPI_read();
    //SPI_read();
    
    MMC_CS = 1;
}

void write_single_block ( unsigned long block_address, unsigned char *buf )
{
	int i;
	unsigned char r1;

	block_address = block_address << 9;

	MMC_CS = 0;
        
    // CMD24: WRITE
	while ( (SD_command ( CMD24, block_address )) != 0 ){};
	
    SPI_write( 0xfe );
	
    for ( i=0; i < 512; i++ ) SPI_write( buf [ i ] );
    
    SPI_write( 0xff );
	SPI_write( 0xff );

	r1 = SPI_read();

	if( ( r1 & 0x1f) != 0x05 )  	   
	{                                  
		MMC_CS = 1;           	   
		return;
	}else;

	while ( ! SPI_read() ); 				

	MMC_CS = 1;

	SPI_write( 0xff );   					

	MMC_CS = 0;
	while ( ! SPI_read() );
	MMC_CS = 1;

	return;
}

unsigned char sdsc_init ( void )
{
    unsigned char res = 0;
    
    for(unsigned char i=0; i<80; i++) SPI_write(0xFF); // SEND 80 CLOCK PUSLSES
    
    MMC_CS = 0;
    
    res = SD_command(CMD0, 0);    
    if(res != 0x01) return 0xFF;    
    res  = SD_command ( CMD8, 0x1AA );
    
    MMC_CS = 1;
    
    return ( res );
}

// ---------------------------------------------------

#endif	


