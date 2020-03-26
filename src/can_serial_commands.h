/**
 * @brief CAN over serial command definitions
 * 
 * @file can_serial_commands.h
 */

#ifndef CAN_SERIAL_COMMANDS_H
#define CAN_SERIAL_COMMANDS_H

/* Includes -------------------------------------------- */

/* Defines --------------------------------------------- */

/* Serial port command functions ----------------------- */
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
