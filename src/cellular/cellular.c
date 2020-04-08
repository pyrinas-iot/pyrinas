#include "FreeRTOS.h"

#include "cellular_cfg_hw.h"
#include "cellular_cfg_module.h"
#include "cellular_cfg_sw.h"
#include "cellular_port.h"
#include "cellular_port_os.h"

#include "cellular_ctrl.h"
#include "cellular_port_clib.h"
#include "cellular_port_debug.h"
#include "cellular_port_uart.h"
#include "cellular_sock.h" // For cellularSockGetHostByName()

#include "boards.h"
#include "cellular.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"

// Used to check if we're initialized
static bool m_initialized = false;

// UART queue.
static CellularPortQueueHandle_t gQueueHandle = NULL;

uint32_t cellular_init(void)
{

    int32_t errorCode;

    // Buffer power/enable
    nrf_gpio_cfg_output(UB_BUF_PWR);
    nrf_gpio_pin_set(UB_BUF_PWR);

    // Reset pin
    nrf_gpio_cfg_output(UB_RST);
    nrf_gpio_pin_set(UB_RST);

    // RTS must be low at all times
    nrf_gpio_cfg_output(UB_RTS);
    nrf_gpio_pin_clear(UB_RTS);

    // Init function (which does nothing)
    errorCode = cellularPortInit();

    if (errorCode == NRF_SUCCESS)
    {
        errorCode = cellularPortUartInit(UB_TX,
                                         UB_RX,
                                         UB_CTS,
                                         UB_RTS,
                                         CELLULAR_BAUD,
                                         CELLULAR_UNUSED,
                                         CELLULAR_UARTE,
                                         &gQueueHandle);
        if (errorCode == 0)
        {
            errorCode = cellularCtrlInit(CELLULAR_UNUSED,
                                         UB_PWR_ON,
                                         UB_VINT,
                                         false,
                                         CELLULAR_UARTE,
                                         gQueueHandle);
            if (errorCode == 0)
            {
                m_initialized = true;
                NRF_LOG_RAW_INFO("CELLULAR: initialised.\n");
            }
            else
            {
                NRF_LOG_RAW_INFO("CELLULAR: cellularCtrlInit() failed (%d).\n",
                                 errorCode);
            }
        }
        else
        {
            NRF_LOG_RAW_INFO("CELLULAR: cellularPortUartInit() failed (%d).\n",
                             errorCode);
        }
    }
    else
    {
        NRF_LOG_RAW_INFO("CELLULAR: cellularPortInit() failed (%d).\n",
                         errorCode);
    }

    // Power it on!
    if (m_initialized && (cellularCtrlPowerOn(NULL) == 0))
    {
        cellularPortLog("CELLULAR: WIFI_On() cellular powered on.\n");
        errorCode = NRF_SUCCESS;
    }

    return errorCode;
}