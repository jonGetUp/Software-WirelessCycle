/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Patrice Rudaz, Thomas Sterren
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "irqlinetouartmaster.h"
#include "gpio/gpiohal.h"

#include <boards.h>

using hal::IrqLineToUartMaster;

//------------------------------------------------------------------------------------------------------------
// static
void IrqLineToUartMaster::initialize()
{
    hal::GpioHal::cfgQuickOutput(IRQ_USR2_PIN);
}

//------------------------------------------------------------------------------------------------------------
// static
void IrqLineToUartMaster::set()
{
    hal::GpioHal::pinSet(IRQ_USR2_PIN);
}

//------------------------------------------------------------------------------------------------------------
// static
void IrqLineToUartMaster::clear()
{
    hal::GpioHal::pinClear(IRQ_USR2_PIN);
}
