/*
 * \brief  Test for Gpio with zedcam on zynq board
 * \author Mark Albers
 * \date   2015-03-30
 */

/*
 * Copyright (C) 2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <gpio_session/zynq/connection.h>
#include <timer_session/connection.h>
#include <base/printf.h>

using namespace Genode;

int main()
{
    printf("--- Test Gpio ---\n");

	static Timer::Connection timer;

    /*
     * Int argument for Gpio::Connection is number of
     * gpio device configured in gpio_test.run from zero.
     */
    static Gpio::Connection gpio_trigger(0);
    static Gpio::Connection gpio_leds(1);
    static Gpio::Connection gpio_switches(2);

	uint8_t value;

    /*
     * First argument in write (Genode::uint32_t data)
     * is the value written to gpio, second
     * argument (bool) means writing to channel 1 (false)
     * or channel 2 (true) of gpio (default channel 1).
     */
    gpio_trigger.write(1);
    gpio_leds.write(1);
    PINF("Write value 0x1.");
    timer.msleep(3000);

    gpio_trigger.write(0);
    gpio_leds.write(2);
    PINF("Write value 0x0.");
    timer.msleep(3000);

    gpio_trigger.write(1);
    gpio_leds.write(4);
    PINF("Write value 0x1.");
    timer.msleep(3000);

    gpio_trigger.write(0);
    gpio_leds.write(0);
    PINF("Write value 0x0.");
    timer.msleep(1000);

    /*
     * Argument in read (bool) means reading
     * from channel 1 (false)or channel 2 (true)
     * of gpio (default channel 1).
     */
    for (unsigned i = 0; i<100; i++)
    {
        value = gpio_switches.read();
        PINF("Read value : 0x%x.", value);
        timer.msleep(100);
    }

    PINF("Gpio Test: done");

	return 0;
}

