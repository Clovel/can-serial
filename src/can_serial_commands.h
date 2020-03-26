/**
 * @brief CAN over serial command definitions
 * 
 * @file can_serial_commands.h
 */

#ifndef CAN_SERIAL_COMMANDS_H
#define CAN_SERIAL_COMMANDS_H

/* Includes -------------------------------------------- */

/* Defines --------------------------------------------- */

/**
 * @brief Maximum answer size (6 + 2 for margin)
 * 
 * @details According to the Lawicel CANUSB document,
 * it seems that the maximum size for a command's
 * answer is 26 bytes.
 */
#define CAN_SERIAL_ANSWER_MAX_SIZE      32U

/**
 * @brief Maximum command size (6 + 2 for margin)
 * 
 * @details According to the Lawicel CANUSB document,
 * it seems that the maximum size for a command's
 * answer is 6 bytes.
 */
#define CAN_SERIAL_CMD_MAX_SIZE         CAN_SERIAL_ANSWER_MAX_SIZE

#define CAN_SERIAL_END_OF_CMD           '\r'    /**< End-of-command delimiter */

#define CAN_SERIAL_CMD_BELL             "\x07"  /**< Indicates an error */
#define CAN_SERIAL_CMD_OK               "\r"    /**< "OK" answer */
#define CAN_SERIAL_CMD_OPEN             "O\r"   /**< Open CAN channel cmd */
#define CAN_SERIAL_CMD_CLOSE            "C\r"   /**< Close CAN channel cmd */
#define CAN_SERIAL_CMD_SN               "N\r"   /**< Get Serial Number cmd */
#define CAN_SERIAL_CMD_VN               "V\r"   /**< Get Version Number cmd */
#define CAN_SERIAL_CMD_STAUTS_FLAGS     "F\r"   /**< Read Status Flags cmd */

/* Serial port command functions ----------------------- */
/**
 * @brief Send a command to the CANUSB device
 * via the serial port
 * 
 * @param[in]   pID         The Id of the CAN driver
 * @param[in]   pCmd        The string representing the command
 * @param[out]  pAnswer     The answer received from the CAN device
 * 
 * $@details
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_sendCmd(const canSerialID_t pID,
    const char * const pCmd,
    char * const pAnswer);

/**
 * @brief Send the command to OPEN the CAN channel
 * to the CAN device.
 * 
 * @param[in]   pID     ID of the CANSerial module
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_sendOpenChannelCmd(const canSerialID_t pID);

/**
 * @brief Send the command to CLOSE the CAN channel
 * to the CAN device.
 * 
 * @param[in]   pID     ID of the CANSerial module
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_sendCloseChannelCmd(const canSerialID_t pID);

#endif /* CAN_SERIAL_COMMANDS_H */
