//
// Created by woegi on 08.03.22.
//

#ifndef CPUSIM_BIOSSIM_H
#define CPUSIM_BIOSSIM_H

#endif //CPUSIM_BIOSSIM_H
unsigned char CheckForBIOSInterrupt();
unsigned char GetInterruptArg(unsigned char INTCODE);
void HandleCPUInterrupt(unsigned char INTCODE, unsigned char ACC);