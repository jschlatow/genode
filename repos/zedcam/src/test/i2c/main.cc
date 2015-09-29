/*
 * \brief  Test for I2C with zedcam on zynq board
 * \author Mark Albers
 * \date   2015-03-12
 */

/*
 * Copyright (C) 2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <i2c_session/zynq/connection.h>
#include <timer_session/connection.h>
#include <base/printf.h>

using namespace Genode;

int main()
{
	printf("--- Test I2C ---\n");

	static Timer::Connection timer;
	static I2C::Connection i2c_0(0);
	static I2C::Connection i2c_1(1);

	uint8_t value;

	i2c_0.write_16bit_reg(0x10, 0x301b, 0xdc);
	PINF("Write value 0xdc to reg 0x301b on bus 0.");

	i2c_1.write_16bit_reg(0x10, 0x301b, 0xdc);
	PINF("Write value 0xdc to reg 0x301b on bus 1.");
	timer.msleep(1000);

	i2c_1.write_16bit_reg(0x10, 0x301b, 0x01);
	PINF("Write value 0x01 to reg 0x301b on bus 1.");
	timer.msleep(1000);

	i2c_1.write_16bit_reg(0x10, 0x301b, 0xdc);
	PINF("Write value 0xdc to reg 0x301b on bus 1.");
	timer.msleep(1000);

	i2c_1.read_byte_16bit_reg(0x10, 0x301b, &value);
	PINF("Read value from reg 0x301b on bus 1: 0x%x.", value);
	timer.msleep(1000);

	i2c_1.write_16bit_reg(0x10, 0x301b, 0x01);
	PINF("Write value 0x01 to reg 0x301b on bus 1.");
	timer.msleep(1000);

	i2c_0.write_16bit_reg(0x10, 0x301b, 0x01);
	PINF("Write value 0x01 to reg 0x301b on bus 0.");

	PINF("I2C Test: done");

	return 0;
}

