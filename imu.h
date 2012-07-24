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
* Intertial Measurement Unit API
*   
* by Richard Sheperd
* based on InvenSense MPU-6050 6-axis MEMS Driver by Humphrey Hu
*
* v 0.1
*
*                   
* Notes:
* - This module is meant to provide an open API for pulling data from any IMU chip
* - Currently supports:
* 	 - InvenSense MPU-6050
*    - InvenSense MPU-6000
*
*/

#ifndef __IMU_H
#define __IMU_H

// Intialize the device
void init(void);

// 
void imuRunCalib(unsigned int count);

// Set sleep mode
void imuSetSleep(unsigned char mode);

/* Gyro Functions */

// Get all gyro values
// @returns 3 ints
void imuGetGyro(int* buff);

// Get X-value of gyro
// @returns 1 int
void imuGetGyroX(int buff);

// Get Y-value of gyro
// @returns 1 int
void imuGetGyroY(int buff);

// Get Z-value of gyro
// @returns 1 int
void imuGetGyroZ(int buff);

/* Accelerometer Functions */

// Get all accelerometer values 
// @returns 3 ints
void imuGetAccel(int* buff)

// Get X-value of accelerometer
// @returns 1 int
void imuGetAccelX(int buff);

// Get Y-value of accelerometer
// @returns 1 int
void imuGetAccelX(int buff);

// Get Z-value of accelerometer
// @returns 1 int
void imuGetAccelX(int buff);

// Get the temperature of the IMU 
// @returns 1 int
void imuGetTemp(int* buff);

#endif