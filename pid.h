/*
 * Copyright (c) 2010, Regents of the University of California
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
 * Generalized integer PID module
 *
 * by Andrew Pullin
 *
 * v.beta
 *
 * Revisions:
 *  Andrew Pullin    2012-06-03    Initial release
 *
 * Notes:
 */

#ifndef __PID_H
#define __PID_H

//Select DSP core PID
#define PID_HARDWARE

#ifdef PID_SOFTWARE
#define DEFAULT_KP  200
#define DEFAULT_KI  5
#define DEFAULT_KD  0
#define DEFAULT_KAW 5
#define DEFAULT_FF  0

#elif defined PID_HARDWARE
#include <dsp.h>
#define DEFAULT_KP  15000
#define DEFAULT_KI  500
#define DEFAULT_KD  150
#define DEFAULT_KAW 0
#define DEFAULT_FF  0
#define MOTOR_PID_ERR_SCALER 32
#endif

#define PID_ON 1
#define PID_OFF 0

//Structures and enums
//PID Continer structure

typedef struct {
    int input;
    long dState, iState, preSat, p, i, d;
    int Kp, Ki, Kd, Kaw, y_old, output;
    unsigned char N;
    char onoff; //boolean
    long error;
    unsigned long run_time;
    unsigned long start_time;
    int inputOffset;
    int feedforward;
    int maxVal, minVal;
    int satVal;
#ifdef PID_HARDWARE
    tPID dspPID;
#endif
} pidObj;

//Functions
void pidSetup();
void pidUpdate(pidObj *pid, int y);
void pidInitPIDObj(pidObj *pid, int Kp, int Ki, int Kd, int Kaw, int ff);
void pidSetInput(pidObj *pid, int feedback);
void pidSetGains(pidObj *pid, int Kp, int Ki, int Kd, int Kaw, int ff);
void pidOnOff(pidObj *pid, unsigned char state);

#endif