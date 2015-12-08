/*
 * \brief  Test for VDMA with zedcam on zynq board
 * \author Mark Albers
 * \date   2015-04-13
 */

/*
 * Copyright (C) 2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <vdma_session/zynq/connection.h>
#include <timer_session/connection.h>
#include <base/printf.h>

using namespace Genode;

#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 960

#define IMG_BAY_L_ADDR 0x1FE00000

int main()
{
    printf("--- Test VDMA ---\n");

	static Timer::Connection timer;

    /*
     * Int argument for Vdma::Connection is number of
     * vdma device configured in vdma_test.run from zero.
     */
    static Vdma::Connection vdma_left_input(0);

    /*
     * VDMA starts working when image height is set.
     * Second param is S2MM for stream to memory
     * or MM2S for memory to stream direction.
     */

    // vdma s2mm channel
    vdma_left_input.setConfig(0x4, S2MM); //reset
    vdma_left_input.setConfig(0x01013, S2MM);
    vdma_left_input.setStride(FRAME_WIDTH, S2MM);
    vdma_left_input.setAddr(IMG_BAY_L_ADDR, S2MM);
    vdma_left_input.setWidth(FRAME_WIDTH, S2MM);
    vdma_left_input.setHeight(FRAME_HEIGHT, S2MM);

    timer.msleep(20000);
    vdma_left_input.setConfig(0x4, S2MM); //reset

    PINF("VDMA Test: done");

	return 0;
}

