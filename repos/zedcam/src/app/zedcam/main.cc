/*
 * \brief  Zedcam App on zynq board
 * \author Mark Albers
 * \date   2015-04-21
 */

/*
 * Copyright (C) 2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <gpio_session/zynq/connection.h>
#include <i2c_session/zynq/connection.h>
#include <vdma_session/zynq/connection.h>
#include <timer_session/connection.h>
#include <base/printf.h>

#include "image_mem.h"

using namespace Genode;

#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 960
#define BAYER_IMG_SIZE 0x200000
#define RGB_IMG_SIZE 0x400000

int main()
{
    printf("--- Zedcam App Test ---\n");

	static Timer::Connection timer;

    /* GPIOs */
    static Gpio::Connection gpio_trigger(0);
    static Gpio::Connection gpio_leds(1);
    static Gpio::Connection gpio_switches(2);

    /* I2Cs */
    static I2C::Connection i2c_0(0);
    static I2C::Connection i2c_1(1);

    /* VDMAs */
    static Vdma::Connection vdma_left_input(0);




    Image_mem img_bay_l(BAYER_IMG_SIZE);
    Image_mem img_bay_r(BAYER_IMG_SIZE);

    PINF("Phys addr img_bay_l: %p.", (void*)img_bay_l.phys_addr());
    PINF("Phys addr img_bay_r: %p.", (void*)img_bay_r.phys_addr());





    PINF("Data img_bay_l: %d.", *(uint8_t volatile *)(img_bay_l.img_base() + 0x100000));
    PINF("Data img_bay_r: %d.", *(uint8_t volatile *)(img_bay_r.img_base() + 0x100000));




    /* Start cams */
    i2c_0.write_16bit_reg(0x10, 0x301b, 0xdc);
    PINF("Write value 0xdc to reg 0x301b on bus 0.");
    i2c_1.write_16bit_reg(0x10, 0x301b, 0xdc);
    PINF("Write value 0xdc to reg 0x301b on bus 1.");

    /* Start VDMA */
    // vdma s2mm channel
    vdma_left_input.setConfig(0x4, S2MM); //reset
    vdma_left_input.setConfig(0x01013, S2MM);
    vdma_left_input.setStride(FRAME_WIDTH, S2MM);
    vdma_left_input.setAddr(img_bay_l.phys_addr(), S2MM);
    vdma_left_input.setWidth(FRAME_WIDTH, S2MM);
    vdma_left_input.setHeight(FRAME_HEIGHT, S2MM);

    timer.msleep(3000);


    PINF("Data img_bay_l: %d.", *(uint8_t volatile *)(img_bay_l.img_base() + 0x100000));
    PINF("Data img_bay_r: %d.", *(uint8_t volatile *)(img_bay_r.img_base() + 0x100000));




    /* Reset VDMA */
    vdma_left_input.setConfig(0x4, S2MM);

    /* Stop cams */
    i2c_0.write_16bit_reg(0x10, 0x301b, 0x01);
    PINF("Write value 0x01 to reg 0x301b on bus 0.");
    i2c_1.write_16bit_reg(0x10, 0x301b, 0x01);
    PINF("Write value 0x01 to reg 0x301b on bus 1.");

    /* Signalize that test is done */
    PINF("Zedcam App Test: done");





    /*gpio_trigger.write(1);
    gpio_leds.write(1);
    PINF("Write value 0x1.");
    timer.msleep(3000);*/

	return 0;
}

