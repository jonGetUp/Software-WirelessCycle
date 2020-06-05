/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef BLE_SECURITY_CONFIG_H
#define BLE_SECURITY_CONFIG_H

#include <ble_gap.h>

// ------------------------------------------------------------------------------------------------------------------
// Security parameters
#if (USE_MITM_BONDING == 0) &&  (USE_JUST_WORKS_BONDING == 0)                                                           ///< \brief No security at all
    #define SEC_PARAM_BOND                      false                                                                   ///< \brief Perform bonding.
    #define SEC_PARAM_MITM                      false                                                                   ///< \brief Man In The Middle protection not required.
    #define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                                                    ///< \brief I/O capabilities.
    #define SEC_PARAM_OOB                       false                                                                   ///< \brief Out Of Band data not available.

#elif (USE_MITM_BONDING != 0)                                                                                           ///< \brief Passkey bonding with display capabilities
    #define SEC_PARAM_BOND                      true                                                                    ///< \brief Perform bonding.
    #define SEC_PARAM_MITM                      true                                                                    ///< \brief Man In The Middle protection not required.
    #define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_DISPLAY_ONLY                                            ///< \brief I/O capabilities. 
    #define SEC_PARAM_OOB                       false                                                                   ///< \brief Out Of Band data not available.

#elif (USE_JUST_WORKS_BONDING != 0)
    #define SEC_PARAM_BOND                      true                                                                    ///< \brief Perform bonding.
    #define SEC_PARAM_MITM                      false                                                                   ///< \brief Man In The Middle protection not required.
    #define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                                                    ///< \brief No I/O capabilities.
    #define SEC_PARAM_OOB                       false                                                                   ///< \brief Out Of Band data not available.

#endif

#define SEC_PARAM_LESC                          0                                                                       ///< \brief LE Secure Connections not enabled.
#define SEC_PARAM_KEYPRESS                      0                                                                       ///< \brief Keypress notifications not enabled.
#define SEC_PARAM_MIN_KEY_SIZE                  7                                                                       ///< \brief Minimum encryption key size.
#define SEC_PARAM_MAX_KEY_SIZE                  16                                                                      ///< \brief Maximum encryption key size.

#define APP_FEATURE_NOT_SUPPORTED               BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2                                    ///< \brief Reply when unsupported features are requested.

// ------------------------------------------------------------------------------------------------------------------
// Security behaviour when wrong PIN or key missong
#define DISCONNECT_ON_ERROR_PIN_OR_KEY_MISSING  1                                                                       ///< \brief If set the device will close the connection when bonding information has been deleted on the current device and the key are wrong or missing at the next connection. 

    #endif // BLE_SECURITY_CONFIG_H
