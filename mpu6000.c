/*
* Copyright (c) 2012, Regents of the University of California
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* - Redistributions of source code must retain the above copyright notice,
*   this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
* - Neither the name of the University of California, Berkeley nor the names
*   of its contributors may be used to endorse or promote products derived
*   from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* InvenSense MPU-6000 6-axis MEMS Driver
*   
* by Richard J. Sheperd
* based on MPU-6050 Driver by Humphrey Hu
*
* v alpha
*
* Revisions:
* 
*                      
* Notes:
*  - This module uses an SPI port for communicating with the chip
*/

#include "mpu6000.h"

#include "spi.h"
#include "spi_controller.h"
#include "utils.h"
#include <string.h>

// Registers
#define MPU_REG_RATEDIV		(25)
#define MPU_REG_CONFIG		(26)
#define MPU_REG_GYROCONFIG	(27)
#define MPU_REG_XLCONFIG	(28)
#define MPU_REG_FIFOEN		(35)
#define MPU_REG_I2CMASTCON	(36)
#define MPU_REG_I2CMASTSTAT	(54)
#define MPU_REG_INTENABLE	(56)
#define MPU_REG_INTSTAT		(57)
#define MPU_REG_XLBASE		(59)
#define MPU_XLLEN			(6)
#define MPU_REG_TEMPBASE	(65)
#define MPU_TEMPLEN			(2)
#define MPU_REG_GYROBASE	(67)
#define MPU_GYROLEN			(6)
#define MPU_REG_USERCON		(106)
#define MPU_REG_PMGT1		(107)
#define MPU_REG_PMGT2		(108)
#define MPU_REG_FIFOCNTH	(114)	    // Not sure if high or low
#define MPU_REG_FIFOCNTL	(115)
#define MPU_REG_FIFORW		(116)
#define MPU_REG_WHOAMI		(117)

// Read/Write Access
#define READ				(128) 
#define WRITE				(0)

// More parameters
#define GYRO_SCALE_BASE		(0.000133231241)	// Lowest gyro range (250 degrees/sec)
#define XL_SCALE_BASE		(5.98550415E-4)		// Lowest accelerometer range (+-2 g)
#define TEMP_SCALE			(0.00294117647)		

#define DEFAULT_TEMP_OFFSET	(521)			
#define UPDATE_SIZE			(14)
#define UPDATE_TIMEOUT		(1000)

/*-----------------------------------------------------------------------------
*          Static Variables
-----------------------------------------------------------------------------*/
// Internal data buffer
static struct {
    int xl_data[3];
    int gyro_data[3];
    int temp;
} mpu_data;

static struct {
    int xl_offset[3]; 		// Not yet implemented!
    int gyro_offset[3]; 	// Not yet implemented!
    int temp_offset;		// Not yet implemented!
    float xl_scale;		// For m/s^2
    float gyro_scale;	// For rad/s
    float temp_scale;	// For celsius
} mpu_params;

/*-----------------------------------------------------------------------------
*          Declaration of static functions
-----------------------------------------------------------------------------*/
static void writeReg(unsigned char regaddr, unsigned char data );
static unsigned char readReg(unsigned char regaddr);

//TODO: Implement
static inline unsigned int readString(unsigned int length, unsigned char * data,
unsigned int data_wait);
//TODO: Implement
static inline unsigned int writeString(unsigned int length, unsigned char* data);								   
//TODO: Implement
static inline void sendByte( unsigned char byte );
//TODO: Implement
static inline unsigned char receiveByte(void);
static inline void setupI2C(void);

/*-----------------------------------------------------------------------------
*          Public functions
-----------------------------------------------------------------------------*/

// Note to self: FIFO State change requires power cycle!

void mpuSetup(void) {
    
    // setup SPI port
    setupSPI();

    unsigned char reg;

    writeReg(MPU_REG_PMGT1, 0x03);				// Set clock to PLL Z Gyro	
    reg = readReg(MPU_REG_PMGT1);

    writeReg(MPU_REG_RATEDIV, DEFAULT_RATEDIV);	// Set rate divider
    
    writeReg(MPU_REG_GYROCONFIG, 0b00010000);	// Set gyro scale 1000 DPS (4x)
    mpu_params.gyro_scale = GYRO_SCALE_BASE*4;
    
    writeReg(MPU_REG_XLCONFIG, 0b00010000);		// Set xl scale 8g (4x)
    mpu_params.xl_scale = XL_SCALE_BASE*4;		
    
    mpu_params.temp_scale = TEMP_SCALE;
    mpu_params.temp_offset = DEFAULT_TEMP_OFFSET;
    
    writeReg(MPU_REG_INTENABLE, 0);				// Disable interrupts
    writeReg(MPU_REG_USERCON, 0b00000100);		// Disable FIFO, I2C Master mode
    writeReg(MPU_REG_CONFIG, 0);				// Set frame sync and DLPF off
    writeReg(MPU_REG_PMGT2, 0b00000000);		// Activate all sensors

    mpuRunCalib(1000);
}

// TODO: Implement!
void mpuRunCalib(unsigned int count){

    unsigned int i;
    long gx, gy, gz;

    for(i = 0; i < count; i++) {
        mpuUpdate();
        gx += mpu_data.gyro_data[0];
        gy += mpu_data.gyro_data[1];
        gz += mpu_data.gyro_data[2];
    }
    mpu_params.gyro_offset[0] = -gx/count;
    mpu_params.gyro_offset[1] = -gy/count;
    mpu_params.gyro_offset[2] = -gz/count;

}

// TODO: Implement
void mpuSetSleep(unsigned char mode) {

    return;

}

void mpuGetGyro(int* buff) {
    buff[0] = mpu_data.gyro_data[0] + mpu_params.gyro_offset[0];
    buff[1] = mpu_data.gyro_data[1] + mpu_params.gyro_offset[1];
    buff[2] = mpu_data.gyro_data[2] + mpu_params.gyro_offset[2];
}

float mpuGetGyroScale(void) {
    return mpu_params.gyro_scale;
}

void mpuGetXl(int* buff) {
    buff[0] = mpu_data.xl_data[0] + mpu_params.xl_offset[0];
    buff[1] = mpu_data.xl_data[1] + mpu_params.xl_offset[1];
    buff[2] = mpu_data.xl_data[2] + mpu_params.xl_offset[2];
}

float mpuGetXlScale(void) {
    return mpu_params.xl_scale;
}

void mpuGetTemp(int* buff) {    
    *buff = mpu_data.temp + mpu_params.temp_offset;
}    

float mpuGetTempScale(void) {
    return mpu_params.temp_scale;
}

void mpuUpdate(void) {

    unsigned char buff[UPDATE_SIZE], rev[UPDATE_SIZE], i;

    startTx();
    sendByte(MPU_ADDR_WR);
    sendByte(MPU_REG_XLBASE);
    endTx();
    startTx();
    sendByte(MPU_ADDR_RD);
    readString(UPDATE_SIZE, buff, UPDATE_TIMEOUT);
    sendNACK();
    endTx();

    // Order is XL[6] TEMP[2] GYRO[6]
    // Reverse data
    for(i = 0; i < UPDATE_SIZE; i++) {
        rev[i] = buff[UPDATE_SIZE - i - 1];
    }
    
    // Order is now GYRO[6] TEMP[2] XL[6]
    // Copy into buffers
    memcpy(mpu_data.gyro_data, rev, 6);
    memcpy(&mpu_data.temp, rev + 6, 2);
    memcpy(mpu_data.xl_data, rev + 8, 6);
    
}


/*-----------------------------------------------------------------------------
* ----------------------------------------------------------------------------
* The functions below are intended for internal use, i.e., private methods.
* Users are recommended to use functions defined above.
* ----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/

/*****************************************************************************
* Function Name : writeReg
* Description   : Write a data to a register
* Parameters    : regaddr - address of register
*                 data - value to be written to the register
* Return Value  : None
*****************************************************************************/
static void writeReg(unsigned char regaddr, unsigned char data ){
    spic2BeginTransaction();
    spic2Transmit(regaddr);
    spic2Transmit(data);
    spic2EndTransaction();
}

/*****************************************************************************
* Function Name : readReg
* Description   : Read a register
* Parameters    : regaddr - address of register
* Return Value  : register contents
*****************************************************************************/
static unsigned char readReg(unsigned char regaddr) {
    unsigned char c;
    
    spic2BeginTransaction();
    spic2Transmit(regaddr | READ);
    c = spic2Recieve();
    spic2EndTransaction();
    
    return c;
}

/*****************************************************************************
* Function Name : readString
* Description   : It reads predetermined data string length from the I2C bus.
* Parameters    : length is the string length to read
*                 data is the storage for received mpu data
*                 data_wait is the timeout value
* Return Value  : Number of bytes read before timeout.
*****************************************************************************/
static inline unsigned int readString(unsigned int length, unsigned char * data,
unsigned int data_wait) {
    return NULL;
}

/*****************************************************************************
* Function Name : mpuWriteString
* Description   : Writes a buffer of data to the I2C bus
* Parameters    : length is the string length to write
*                 data is a pointer to buffer containing data to write
* Return Value  : Error codes: -3 if collision, 0 if successful
*****************************************************************************/
static inline unsigned int writeString(unsigned int length, unsigned char * data) {
	return NULL;
}


/*****************************************************************************
* Function Name : sendByte
* Description   : Send a byte to mpuscope
* Parameters    : byte - a byte to send
* Return Value  : None
*****************************************************************************/
static inline void sendByte( unsigned char byte ) {
}

/*****************************************************************************
* Function Name : receiveByte
* Description   : Receive a byte from mpuscope
* Parameters    : None
* Return Value  : None
*****************************************************************************/
static inline unsigned char receiveByte(void) {
    return NULL;
}

/*****************************************************************************
* Function Name : sendNACK
* Description   : Send NACK to mpuscope
* Parameters    : None
* Return Value  : None
*****************************************************************************/
static inline void sendNACK(void){
    NotAckI2C2();
    while(I2C2CONbits.ACKEN);
}

/*****************************************************************************
* Function Name : startTx
* Description   : Start I2C transmission
* Parameters    : None
* Return Value  : None
*****************************************************************************/
static inline void startTx(void){
    StartI2C2();
    while(I2C2CONbits.SEN);
}

/*****************************************************************************
* Function Name : endTx
* Description   : End I2C transmission
* Parameters    : None
* Return Value  : None
*****************************************************************************/
static inline void endTx(void){
    StopI2C2();
    while(I2C2CONbits.PEN);
}

/*****************************************************************************
* Function Name : setupI2C
* Description   : Setup I2C for mpuscope
* Parameters    : None
* Return Value  : None
*****************************************************************************/
static inline void setupI2C(void) {
    unsigned int SPIConValue1, SPIConValue2, SPIConValue3;
    SPIConValue1 = NULL;
    
    SPIConValue2 = 
    	ENABLE_SCK_PIN // Internal SPI clock enabled
    	& ENABLE_SDO_PIN // CSDO pin is used by module
    	& SPI_MODE16_ON // Comm is byte wide   	
    	& SPI_SMP_???? // 
    	& SPI_CKE_ON // Tx happens on active to idle clock
    	& SLAVE_ENABLE_ON // Slave select enabled
    	& CLK_POL_ACTIVE_LOW // Idle state for clk is high, active is low
    	& MASTER_ENABLE_ON // Master mode
    	& SEC_PRESCAL_3_1 // Divide clock by 3
    	& PRI_PRESCAL_16_1; // Divide clock by 16
    	
    SPIConValue3 =     	
    	& FRAME_ENABLE_ON // Frame SPI support enable
    	& FRAME_SYNC_OUTPUT // Frame sync pulse Output (master) 
    	& FRAME_POL_ACTIVE_LOW // Frame sync is active-low
    	& FRAME_SYNC_EDGE_???? // coincides with first bit
    	& FIFO_BUFFER_DISABLE; // FIFO buffer disabled

    OpenSPI2(SPIConValue1, SPIConValue2, SPIConVale3);
}