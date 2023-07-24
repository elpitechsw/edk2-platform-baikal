/** @file
  Support for PMBUS standard.

  Copyright (c) 2023, Elpitech. All rights reserved.<BR>
**/
#ifndef PMBUS_H_
#define PMBUS_H_

#define PMB_CMD_PAGE 0x00
#define PMB_CMD_OPERATION 0x01
#define PMB_CMD_ON_OFF_CONFIG 0x02
#define PMB_CMD_CLEAR_FAULTS 0x03
#define PMB_CMD_CLEAR_PHASE 0x04
#define PMB_CMD_PAGE_PLUS_WRITE 0x05
#define PMB_CMD_PAGE_PLUS_READ 0x06
#define PMB_CMD_ZONE_CONFIG 0x07
#define PMB_CMD_ZONE_ACTIVE 0x08

#define PMB_CMD_WRITE_PROTECT 0x10
#define PMB_CMD_STORE_DEFAULT_ALL 0x11
#define PMB_CMD_RESTORE_DEFAULT_ALL 0x12
#define PMB_CMD_STORE_USER_ALL 0x15
#define PMB_CMD_RESTORE_USER_ALL 0x16
#define PMB_CMD_STORE_USER_CODE 0x17
#define PMB_CMD_RESTORE_USER_CODE 0x18
#define PMB_CMD_CAPABILITY 0x19
#define PMB_CMD_QUERY 0x1a
#define PMB_CMD_SMBALERT_MASK 0x1b

#define PMB_CMD_VOUT_MODE 0x20
#define PMB_CMD_VOUT_COMMAND 0x21
#define PMB_CMD_VOUT_TRIM 0x22
#define PMB_CMD_VOUT_CAL_OFFSET 0x23
#define PMB_CMD_VOUT_MAX 0x24
#define PMB_CMD_VOUT_MARGIN_HIGH 0x25
#define PMB_CMD_VOUT_MARGIN_LOW 0x26
#define PMB_CMD_VOUT_DROOP 0x28
#define PMB_CMD_VOUT_SCALE_LOOP 0x29
#define PMB_CMD_VOUT_SCALE_MONITOR 0x2a
#define PMB_CMD_VOUT_MIN 0x2b

#define PMB_CMD_COEFFICIENTS 0x30
#define PMB_CMD_POUT_MAX 0x31
#define PMB_CMD_MAX_DUTY 0x32
#define PMB_CMD_FREQUENCY_SWITCH 0x33
#define PMB_CMD_POWER_MODE 0x34
#define PMB_CMD_VIN_ON 0x35
#define PMB_CMD_VIN_OFF 0x36
#define PMB_CMD_INTERLEAVE 0x37
#define PMB_CMD_IOUT_CAL_GAIN 0x38
#define PMB_CMD_IOUT_CAL_OFFSET 0x39
#define PMB_CMD_FAN_CONFIG_1_2 0x3a
#define PMB_CMD_FAN_COMMAND_1 0x3b
#define PMB_CMD_FAN_COMMAND_2 0x3c
#define PMB_CMD_FAN_CONFIG_3_4 0x3d
#define PMB_CMD_FAN_COMMAND_3 0x3e
#define PMB_CMD_FAN_COMMAND_4 0x3f

#define PMB_CMD_VOUT_OV_FAULT_LIMIT 0x40
#define PMB_CMD_VOUT_OV_FAULT_RESPONSE 0x41
#define PMB_CMD_VOUT_OV_WARN_LIMIT 0x42
#define PMB_CMD_VOUT_UV_WARN_LIMIT 0x43
#define PMB_CMD_VOUT_UV_FAULT_LIMIT 0x44
#define PMB_CMD_VOUT_UV_FAULT_RESPONSE 0x45
#define PMB_CMD_IOUT_OC_FAULT_LIMIT 0x46
#define PMB_CMD_IOUT_OC_FAULT_RESPONSE 0x47
#define PMB_CMD_IOUT_OC_LV_FAULT_LIMIT 0x48
#define PMB_CMD_IOUT_OC_LV_FAULT_RESPONSE 0x49
#define PMB_CMD_IOUT_OC_WARN_LIMIT 0x4a
#define PMB_CMD_IOUT_UC_FAULT_LIMIT 0x4b
#define PMB_CMD_IOUT_UC_FAULT_RESPONSE 0x4c

#define PMB_CMD_OT_FAULT_LIMIT 0x4f
#define PMB_CMD_OT_FAULT_RESPONSE 0x50
#define PMB_CMD_OT_WARN_LIMIT 0x51
#define PMB_CMD_UT_WARN_LIMIT 0x52
#define PMB_CMD_UT_FAULT_LIMIT 0x53
#define PMB_CMD_UT_FAULT_RESPONSE 0x54
#define PMB_CMD_VIN_OV_FAULT_LIMIT 0x55
#define PMB_CMD_VIN_OV_FAULT_RESPONSE 0x56
#define PMB_CMD_VIN_OV_WARN_LIMIT 0x57
#define PMB_CMD_VIN_UV_WARN_LIMIT 0x58
#define PMB_CMD_VIN_UV_FAULT_LIMIT 0x59
#define PMB_CMD_VIN_UV_FAULT_RESPONSE 0x5a
#define PMB_CMD_IIN_OC_FAULT_LIMIT 0x5b
#define PMB_CMD_IIN_OC_FAULT_RESPONSE 0x5c
#define PMB_CMD_IIN_OC_WARN_LIMIT 0x5d
#define PMB_CMD_POWER_GOOD_ON 0x5e
#define PMB_CMD_POWER_GOOD_OFF 0x5f

#define PMB_CMD_TON_DELAY 0x60
#define PMB_CMD_TON_RISE 0x61
#define PMB_CMD_TON_MAX_FAULT_LIMIT 0x62
#define PMB_CMD_TON_MAX_FAULT_RESPONSE 0x63
#define PMB_CMD_TOFF_DELAY 0x64
#define PMB_CMD_TOFF_FALL 0x65
#define PMB_CMD_TOFF_MAX_WARN_LIMIT 0x66

#define PMB_CMD_POUT_OP_FAULT_LIMIT 0x68
#define PMB_CMD_POUT_OP_FAULT_RESPONSE 0x69
#define PMB_CMD_POUT_OP_WARN_LIMIT 0x6a
#define PMB_CMD_PIN_OP_WARN_LIMIT 0x6b

#define PMB_CMD_STATUS_BYTE 0x78
#define PMB_CMD_STATUS_WORD 0x79
#define PMB_CMD_STATUS_VOUT 0x7a
#define PMB_CMD_STATUS_IOUT 0x7b
#define PMB_CMD_STATUS_INPUT 0x7c
#define PMB_CMD_STATUS_TEMPERATURE 0x7d
#define PMB_CMD_STATUS_CML 0x7e
#define PMB_CMD_STATUS_OTHER 0x7f

#define PMB_CMD_STATUS_MFR_SPECIFIC 0x80
#define PMB_CMD_STATUS_FANS_1_2 0x81
#define PMB_CMD_STATUS_FANS_3_4 0x82
#define PMB_CMD_READ_KWH_IN 0x83
#define PMB_CMD_READ_KWH_OUT 0x84
#define PMB_CMD_READ_KWH_CONFIG 0x85
#define PMB_CMD_READ_EIN 0x86
#define PMB_CMD_READ_EOUT 0x87
#define PMB_CMD_READ_VIN 0x88
#define PMB_CMD_READ_IIN 0x89
#define PMB_CMD_READ_VCAP 0x8a
#define PMB_CMD_READ_VOUT 0x8b
#define PMB_CMD_READ_IOUT 0x8c
#define PMB_CMD_READ_TEMPERATURE_1 0x8d
#define PMB_CMD_READ_TEMPERATURE_2 0x8e
#define PMB_CMD_READ_TEMPERATURE_3 0x8f

#define PMB_CMD_READ_FAN_SPEED_1 0x90
#define PMB_CMD_READ_FAN_SPEED_2 0x91
#define PMB_CMD_READ_FAN_SPEED_3 0x92
#define PMB_CMD_READ_FAN_SPEED_4 0x93
#define PMB_CMD_READ_DUTY_CYCLE 0x94
#define PMB_CMD_READ_FREQUENCY 0x95
#define PMB_CMD_READ_POUT 0x96
#define PMB_CMD_READ_PIN 0x97
#define PMB_CMD_PMBUS_REVISION 0x98
#define PMB_CMD_MFR_ID 0x99
#define PMB_CMD_MFR_MODEL 0x9a
#define PMB_CMD_MFR_REVISION 0x9b
#define PMB_CMD_MFR_LOCATION 0x9c
#define PMB_CMD_MFR_DATE 0x9d
#define PMB_CMD_MFR_SERIAL 0x9e
#define PMB_CMD_APP_PROFILE_SUPPORT 0x9f

#define PMB_CMD_MFR_VIN_MIN 0xa0
#define PMB_CMD_MFR_VIN_MAX 0xa1
#define PMB_CMD_MFR_IIN_MAX 0xa2
#define PMB_CMD_MFR_PIN_MAX 0xa3
#define PMB_CMD_MFR_VOUT_MIN 0xa4
#define PMB_CMD_MFR_VOUT_MAX 0xa5
#define PMB_CMD_MFR_IOUT_MAX 0xa6
#define PMB_CMD_MFR_POUT_MAX 0xa7
#define PMB_CMD_MFR_TAMBIENT_MAX 0xa8
#define PMB_CMD_MFR_TAMBIENT_MIN 0xa9
#define PMB_CMD_MFR_EFFICIENCY_LL 0xaa
#define PMB_CMD_MFR_EFFICIENCY_HL 0xab
#define PMB_CMD_MFR_PIN_ACCURACY 0xac
#define PMB_CMD_IC_DEVICE_ID 0xad
#define PMB_CMD_IC_DEVICE_REV 0xae

#define PMB_CMD_MFR_CTRL_COMP 0xd0
#define PMB_CMD_MFR_CTRL_VOUT 0xd1
#define PMB_CMD_MFR_CTRL_OPS 0xd2
#define PMB_CMD_MFR_ADDR_PMBUS 0xd3
#define PMB_CMD_MFR_VOUT_OVP_FAULT_LIMIT 0xd4
#define PMB_CMD_MFR_OVP_NOCP_SET 0xd5
#define PMB_CMD_MFR_OT_OC_SET 0xd6
#define PMB_CMD_MFR_OC_PHASE_LIMIT 0xd7
#define PMB_CMD_MFR_HICCUP_ITV 0xd8
#define PMB_CMD_MFR_PGOOD_ON_OFF 0xd9
#define PMB_CMD_MRF_VOUT_STEP 0xda

#define PMB_CMD_MFR_LOW_POWER 0xe5
#define PMB_CMD_MFR_CTRL 0xea
#define PMB_CMD_MFR_DT_MTP 0xec

#define PMB_MAX_PAGE 0x1f

#define PMB_CAPABILITY_AVSBUS_SUPPORTED 0x04
#define PMB_CAPABILITY_NUMERIC_FORMAT_LINEAR 0x00
#define PMB_CAPABILITY_NUMERIC_FORMAT_FLOAT 0x08
#define PMB_CAPABILITY_SMBALERT_SUPPORTED 0x10
#define PMB_CAPABILITY_MAX_BUS_SPEED_100KHZ 0x00
#define PMB_CAPABILITY_MAX_BUS_SPEED_400KHZ 0x20
#define PMB_CAPABILITY_MAX_BUS_SPEED_1MHZ 0x40
#define PMB_CAPABILITY_MAX_BUS_SPEED_MASK 0x60
#define PMB_CAPABILITY_PEC_SUPPORTED 0x80

#define PMB_STATUS_WORD_VOUT_FAULT_OR_WARNING 0x80
#define PMB_STATUS_WORD_IOUT_FAULT_OR_WARNING 0x40
#define PMB_STATUS_WORD_INPUT_FAULT_OR_WARNING 0x20
#define PMB_STATUS_WORD_MFR_STATUS 0x10
#define PMB_STATUS_WORD_POWER_GOOD_NEGATED 0x08
#define PMB_STATUS_WORD_FAN_FAULT_OR_WARNING 0x04
#define PMB_STATUS_WORD_OTHER 0x02
#define PMB_STATUS_WORD_UNKNOWN_FAULT_OR_WARNING 0x01

#define PMB_STATUS_BYTE_UNIT_BUSY 0x80
#define PMB_STATUS_BYTE_UNIT_OFF 0x40
#define PMB_STATUS_BYTE_VOUT_OV_FAULT 0x20
#define PMB_STATUS_BYTE_IOUT_OC_FAULT 0x10
#define PMB_STATUS_BYTE_VIN_UV_FAULT 0x08
#define PMB_STATUS_BYTE_OT_FAULT_WARN 0x04
#define PMB_STATUS_BYTE_CML 0x02
#define PMB_STATUS_BYTE_NONE_OF_ABOVE 0x01

#define PMB_STATUS_INPUT_VIN_OV_FAULT 0x80
#define PMB_STATUS_INPUT_VIN_OV_WARN 0x40
#define PMB_STATUS_INPUT_VIN_UV_WARN 0x20
#define PMB_STATUS_INPUT_VIN_UV_FAULT 0x10
#define PMB_STATUS_INPUT_UNIT_OFF_UV 0x08
#define PMB_STATUS_INPUT_IIN_OC_FAULT 0x04
#define PMB_STATUS_INPUT_IIN_OC_WARN 0x02
#define PMB_STATUS_INPUT_PIN_OC_WARN 0x01

#define PMB_STATUS_VOUT_OV_FAULT 0x80
#define PMB_STATUS_VOUT_OV_WARN 0x40
#define PMB_STATUS_VOUT_UV_WARN 0x20
#define PMB_STATUS_VOUT_UV_FAULT 0x10
#define PMB_STATUS_VOUT_MAX_MIN_WARN 0x08
#define PMB_STATUS_TON_MAX_FAULT 0x04
#define PMB_STATUS_TOFF_MAX_WARN 0x02
#define PMB_STATUS_VOUT_TRACK_ERR 0x01

#define PMB_STATUS_IOUT_OC_FAULT 0x80
#define PMB_STATUS_IOUT_OC_FAULT_LV_SHUTDOWN 0x40
#define PMB_STATUS_IOUT_OC_WARN 0x20
#define PMB_STATUS_IOUT_UC_FAULT 0x10
#define PMB_STATUS_CURRENT_SHARE_FAULT 0x08
#define PMB_STATUS_IN_PWR_LIM_MODE 0x04
#define PMB_STATUS_POUT_OP_FAULT 0x02
#define PMB_STATUS_POUT_OP_WARN 0x01

#define PMB_STATUS_OT_FAULT 0x80
#define PMB_STATUS_OT_WARN 0x40
#define PMB_STATUS_UT_WARN 0x20
#define PMB_STATUS_UT_FAULT 0x10

#define PMB_CML_INVALID_CMD (1 << 7)
#define PMB_CML_INVALID_DATA (1 << 6)
#define PMB_CML_PACKET_ERR (1 << 5)
#define PMB_CML_MEMORY_FAULT (1 << 4)
#define PMB_CML_PROCESSOR_FAULT (1 << 3)
#define PMB_CML_COMM_FAULT (1 << 1)
#define PMB_CML_OTHER_FAULT (1 << 0)

/**
 * @brief Fan config flags
 */
#define PMB_FAN_CONFIG_INSTALLED (1 << 3)           // fan installed otherwise is not installed
#define PMB_FAN_CONFIG_RPM_COMMANDED (1 << 2)       // fan commanded via RPM otherwise via duty cycle

// Number of pulses per revolution
#define PMB_FAN_CONFIG_PULSES_MASK 0x03
#define PMB_FAN_CONFIG_PULSES(cfg) ((cfg) + 1)

#define PMB_FAN_CONFIG_BY_IDX(idx, cfg) \
	((cfg) << (((idx) % 2) ? 0 : 4))

#define PMB_FAN_STATUS_FAULT(idx) (1 << (6 + ((idx) % 2)))
#define PMB_FAN_STATUS_WARNING(idx) (1 << (4 + ((idx) % 2)))
#define PMB_FAN_STATUS_SPEED_OVERRIDDEN(idx) (1 << (2 + ((idx) % 2)))
#define PMB_FAN_STATUS_AIRFLOW_FAULT (1 << 1)
#define PMB_FAN_STATUS_AIRFLOW_WARNING (1 << 0)

#define PMB_IO_WRITE FALSE
#define PMB_IO_READ TRUE

#define PMB_IO_N_A 0
#define PMB_IO_SEND_BYTE 1
#define PMB_IO_WRITE_BYTE 2
#define PMB_IO_WRITE_WORD 3
#define PMB_IO_WRITE_32 4
#define PMB_IO_WRITE_64 5
#define PMB_IO_BLOCK_WRITE 6
#define PMB_IO_RECV_BYTE 7
#define PMB_IO_READ_BYTE 8
#define PMB_IO_READ_WORD 9
#define PMB_IO_READ_32 10
#define PMB_IO_READ_64 11
#define PMB_IO_BLOCK_READ 12
#define PMB_IO_PROCESS_CALL 13
#define PMB_IO_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL 14

#define PMB_IO_SIZE_VARIABLE -1
#define PMB_IO_MAX_PKT_LEN 32

/**
	 _(command_code, command_name, write_trans_type, read_trans_type, data_bytes)
 **/
#define PMB_CMD_DEF(_)																									\
	_(0x00, PAGE, WRITE_BYTE, READ_BYTE, 1)																\
	_(0x01, OPERATION, WRITE_BYTE, READ_BYTE, 1)													\
	_(0x02, ON_OFF_CONFIG, WRITE_BYTE, READ_BYTE, 1)											\
	_(0x03, CLEAR_FAULTS, SEND_BYTE, N_A, 0)															\
	_(0x04, PHASE, WRITE_BYTE, READ_BYTE, 1)															\
	_(0x05, PAGE_PLUS_WRITE, BLOCK_WRITE, N_A, -1)												\
	_(0x06, PAGE_PLUS_READ, N_A, BLOCK_WRITE_BLOCK_READ_PROCESS_CALL, -1) \
	_(0x07, ZONE_CONFIG, WRITE_WORD, READ_WORD, 2)												\
	_(0x08, ZONE_ACTIVE, WRITE_WORD, READ_WORD, 2)												\
	_(0x10, WRITE_PROTECT, WRITE_BYTE, READ_BYTE, 1)											\
	_(0x11, STORE_DEFAULT_ALL, SEND_BYTE, N_A, 0)													\
	_(0x12, RESTORE_DEFAULT_ALL, SEND_BYTE, N_A, 0)												\
	_(0x13, STORE_DEFAULT_CODE, WRITE_BYTE, N_A, 1)												\
	_(0x14, RESTORE_DEFAULT_CODE, WRITE_BYTE, N_A, 1)											\
	_(0x15, STORE_USER_ALL, SEND_BYTE, N_A, 0)														\
	_(0x16, RESTORE_USER_ALL, SEND_BYTE, N_A, 0)													\
	_(0x17, STORE_USER_CODE, WRITE_BYTE, N_A, 1)													\
	_(0x18, RESTORE_USER_CODE, WRITE_BYTE, N_A, 1)												\
	_(0x19, CAPABILITY, N_A, READ_BYTE, 1)																\
	_(0x1A, QUERY, N_A, BLOCK_WRITE_BLOCK_READ_PROCESS_CALL, 1)						\
	_(0x1B, SMBALERT_MASK, WRITE_WORD, BLOCK_WRITE_BLOCK_READ_PROCESS_CALL, 2) \
	_(0x20, VOUT_MODE, WRITE_BYTE, READ_BYTE, 1)													\
	_(0x21, VOUT_COMMAND, WRITE_WORD, READ_WORD, 2)											  \
	_(0x22, VOUT_TRIM, WRITE_WORD, READ_WORD, 2)													\
	_(0x23, VOUT_CAL_OFFSET, WRITE_WORD, READ_WORD, 2)										\
	_(0x24, VOUT_MAX, WRITE_WORD, READ_WORD, 2)														\
	_(0x25, VOUT_MARGIN_HIGH, WRITE_WORD, READ_WORD, 2)										\
	_(0x26, VOUT_MARGIN_LOW, WRITE_WORD, READ_WORD, 2)										\
	_(0x27, VOUT_TRANSITION_RATE, WRITE_WORD, READ_WORD, 2)								\
	_(0x28, VOUT_DROOP, WRITE_WORD, READ_WORD, 2)													\
	_(0x29, VOUT_SCALE_LOOP, WRITE_WORD, READ_WORD, 2)										\
	_(0x2A, VOUT_SCALE_MONITOR, WRITE_WORD, READ_WORD, 2)									\
	_(0x2B, VOUT_MIN, WRITE_WORD, READ_WORD, 2)														\
	_(0x30, COEFFICIENTS, N_A, BLOCK_WRITE_BLOCK_READ_PROCESS_CALL, 5)		\
	_(0x31, POUT_MAX, WRITE_WORD, READ_WORD, 2)														\
	_(0x32, MAX_DUTY, WRITE_WORD, READ_WORD, 2)														\
	_(0x33, FREQUENCY_SWITCH, WRITE_WORD, READ_WORD, 2)										\
	_(0x34, POWER_MODE, WRITE_BYTE, READ_BYTE, 1)													\
	_(0x35, VIN_ON, WRITE_WORD, READ_WORD, 2)															\
	_(0x36, VIN_OFF, WRITE_WORD, READ_WORD, 2)														\
	_(0x37, INTERLEAVE, WRITE_WORD, READ_WORD, 2)													\
	_(0x38, IOUT_CAL_GAIN, WRITE_WORD, READ_WORD, 2)											\
	_(0x39, IOUT_CAL_OFFSET, WRITE_WORD, READ_WORD, 2)										\
	_(0x3A, FAN_CONFIG_1_2, WRITE_BYTE, READ_BYTE, 1)											\
	_(0x3B, FAN_COMMAND_1, WRITE_WORD, READ_WORD, 2)											\
	_(0x3C, FAN_COMMAND_2, WRITE_WORD, READ_WORD, 2)											\
	_(0x3D, FAN_CONFIG_3_4, WRITE_BYTE, READ_BYTE, 1)											\
	_(0x3E, FAN_COMMAND_3, WRITE_WORD, READ_WORD, 2)											\
	_(0x3F, FAN_COMMAND_4, WRITE_WORD, READ_WORD, 2)											\
	_(0x40, VOUT_OV_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)								\
	_(0x41, VOUT_OV_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)							\
	_(0x42, VOUT_OV_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x43, VOUT_UV_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x44, VOUT_UV_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)								\
	_(0x45, VOUT_UV_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)							\
	_(0x46, IOUT_OC_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)								\
	_(0x47, IOUT_OC_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)							\
	_(0x48, IOUT_OC_LV_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)							\
	_(0x49, IOUT_OC_LV_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)					\
	_(0x4A, IOUT_OC_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x4B, IOUT_UC_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)								\
	_(0x4C, IOUT_UC_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)							\
	_(0x4F, OT_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)											\
	_(0x50, OT_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)									\
	_(0x51, OT_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)											\
	_(0x52, UT_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)											\
	_(0x53, UT_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)										  \
	_(0x54, UT_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)								  \
	_(0x55, VIN_OV_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x56, VIN_OV_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)						  \
	_(0x57, VIN_OV_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x58, VIN_UV_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x59, VIN_UV_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x5A, VIN_UV_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)							\
	_(0x5B, IIN_OC_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x5C, IIN_OC_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)							\
	_(0x5D, IIN_OC_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x5E, POWER_GOOD_ON, WRITE_WORD, READ_WORD, 2)											\
	_(0x5F, POWER_GOOD_OFF, WRITE_WORD, READ_WORD, 2)											\
	_(0x60, TON_DELAY, WRITE_WORD, READ_WORD, 2)													\
	_(0x61, TON_RISE, WRITE_WORD, READ_WORD, 2)														\
	_(0x62, TON_MAX_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)								\
	_(0x63, TON_MAX_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)							\
	_(0x64, TOFF_DELAY, WRITE_WORD, READ_WORD, 2)													\
	_(0x65, TOFF_FALL, WRITE_WORD, READ_WORD, 2)													\
	_(0x66, TOFF_MAX_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)								\
	_(0x68, POUT_OP_FAULT_LIMIT, WRITE_WORD, READ_WORD, 2)								\
	_(0x69, POUT_OP_FAULT_RESPONSE, WRITE_BYTE, READ_BYTE, 1)							\
	_(0x6A, POUT_OP_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x6B, PIN_OP_WARN_LIMIT, WRITE_WORD, READ_WORD, 2)									\
	_(0x78, STATUS_BYTE, WRITE_BYTE, READ_BYTE, 1)												\
	_(0x79, STATUS_WORD, WRITE_WORD, READ_WORD, 2)												\
	_(0x7A, STATUS_VOUT, WRITE_BYTE, READ_BYTE, 1)												\
	_(0x7B, STATUS_IOUT, WRITE_BYTE, READ_BYTE, 1)												\
	_(0x7C, STATUS_INPUT, WRITE_BYTE, READ_BYTE, 1)												\
	_(0x7D, STATUS_TEMPERATURE, WRITE_BYTE, READ_BYTE, 1)									\
	_(0x7E, STATUS_CML, WRITE_BYTE, READ_BYTE, 1)													\
	_(0x7F, STATUS_OTHER, WRITE_BYTE, READ_BYTE, 1)												\
	_(0x80, STATUS_MFR_SPECIFIC, WRITE_BYTE, READ_BYTE, 1)							  \
	_(0x81, STATUS_FANS_1_2, WRITE_BYTE, READ_BYTE, 1)										\
	_(0x82, STATUS_FANS_3_4, WRITE_BYTE, READ_BYTE, 1)										\
	_(0x83, READ_KWH_IN, N_A, READ_32, 4)																	\
	_(0x84, READ_KWH_OUT, N_A, READ_32, 4)																\
	_(0x85, READ_KWH_CONFIG, WRITE_WORD, READ_WORD, 2)										\
	_(0x86, READ_EIN, N_A, BLOCK_READ, 5)																	\
	_(0x87, READ_EOUT, N_A, BLOCK_READ, 5)																\
	_(0x88, READ_VIN, N_A, READ_WORD, 2)																	\
	_(0x89, READ_IIN, N_A, READ_WORD, 2)																	\
	_(0x8A, READ_VCAP, N_A, READ_WORD, 2)																	\
	_(0x8B, READ_VOUT, N_A, READ_WORD, 2)																	\
	_(0x8C, READ_IOUT, N_A, READ_WORD, 2)																	\
	_(0x8D, READ_TEMPERATURE_1, N_A, READ_WORD, 2)												\
	_(0x8E, READ_TEMPERATURE_2, N_A, READ_WORD, 2)												\
	_(0x8F, READ_TEMPERATURE_3, N_A, READ_WORD, 2)												\
	_(0x90, READ_FAN_SPEED_1, N_A, READ_WORD, 2)													\
	_(0x91, READ_FAN_SPEED_2, N_A, READ_WORD, 2)													\
	_(0x92, READ_FAN_SPEED_3, N_A, READ_WORD, 2)													\
	_(0x93, READ_FAN_SPEED_4, N_A, READ_WORD, 2)													\
	_(0x94, READ_DUTY_CYCLE, N_A, READ_WORD, 2)														\
	_(0x95, READ_FREQUENCY, N_A, READ_WORD, 2)														\
	_(0x96, READ_POUT, N_A, READ_WORD, 2)																	\
	_(0x97, READ_PIN, N_A, READ_WORD, 2)																	\
	_(0x98, PMBUS_REVISION, N_A, READ_BYTE, 1)														\
	_(0x99, MFR_ID, BLOCK_WRITE, BLOCK_READ, -1)													\
	_(0x9A, MFR_MODEL, BLOCK_WRITE, BLOCK_READ, -1)												\
	_(0x9B, MFR_REVISION, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0x9C, MFR_LOCATION, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0x9D, MFR_DATE, BLOCK_WRITE, BLOCK_READ, -1)									  		\
	_(0x9E, MFR_SERIAL, BLOCK_WRITE, BLOCK_READ, -1)											\
	_(0x9F, APP_PROFILE_SUPPORT, N_A, BLOCK_READ, -1)											\
	_(0xA0, MFR_VIN_MIN, N_A, READ_WORD, 2)																\
	_(0xA1, MFR_VIN_MAX, N_A, READ_WORD, 2)																\
	_(0xA2, MFR_IIN_MAX, N_A, READ_WORD, 2)															  \
	_(0xA3, MFR_PIN_MAX, N_A, READ_WORD, 2)																\
	_(0xA4, MFR_VOUT_MIN, N_A, READ_WORD, 2)															\
	_(0xA5, MFR_VOUT_MAX, N_A, READ_WORD, 2)															\
	_(0xA6, MFR_IOUT_MAX, N_A, READ_WORD, 2)															\
	_(0xA7, MFR_POUT_MAX, N_A, READ_WORD, 2)															\
	_(0xA8, MFR_TAMBIENT_MAX, N_A, READ_WORD, 2)													\
	_(0xA9, MFR_TAMBIENT_MIN, N_A, READ_WORD, 2)													\
	_(0xAA, MFR_EFFICIENCY_LL, N_A, BLOCK_READ, 14)												\
	_(0xAB, MFR_EFFICIENCY_HL, N_A, BLOCK_READ, 14)												\
	_(0xAC, MFR_PIN_ACCURACY, N_A, READ_BYTE, 1)												  \
	_(0xAD, IC_DEVICE_ID, N_A, BLOCK_READ, -1)														\
	_(0xAE, IC_DEVICE_REV, N_A, BLOCK_READ, -1)														\
	_(0xB0, USER_DATA_00, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB1, USER_DATA_01, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB2, USER_DATA_02, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB3, USER_DATA_03, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB4, USER_DATA_04, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB5, USER_DATA_05, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB6, USER_DATA_06, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB7, USER_DATA_07, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB8, USER_DATA_08, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xB9, USER_DATA_09, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xBA, USER_DATA_10, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xBB, USER_DATA_11, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xBC, USER_DATA_12, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xBD, USER_DATA_13, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xBE, USER_DATA_14, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xBF, USER_DATA_15, BLOCK_WRITE, BLOCK_READ, -1)										\
	_(0xC0, MFR_MAX_TEMP_1, WRITE_WORD, READ_WORD, 2)								  		\
	_(0xC1, MFR_MAX_TEMP_2, WRITE_WORD, READ_WORD, 2)											\
	_(0xC2, MFR_MAX_TEMP_3, WRITE_WORD, READ_WORD, 2)

#ifndef __DTS__

#define _PMB_CMD_CONST(code, name, send, recv, size)	\
  const UINT8 PMB_CMD_##name = code;
//PMB_CMD_DEF(_PMB_CMD_CONST)
#undef _PMB_CMD_CONST

typedef UINT8 PMB_CMD;
typedef UINT8 PMB_DAT;
typedef UINT8 PMB_LEN;

/**
	 The struct which describes PMBUS request
 **/
#pragma pack(1)
typedef union {
	/** Plain request data **/
	PMB_DAT Raw[PMB_IO_MAX_PKT_LEN];
	struct {
		/** Command code **/
		PMB_CMD Cmd;
		/** Request payload data **/
		union {
			/** Request payload plain data **/
			PMB_DAT Data[0];
			/** Request payload block data **/
			struct {
				/** Request block size **/
				PMB_LEN Size;
				PMB_DAT Byte[0];
			} Block;
		};
	};
} PMB_IO_REQ;
#pragma pack()

#define PMB_READ_BYTE(Res, Idx) ((Res)->Data[Idx])
// FIXME: currently little-endian machines is only supported
#define PMB_READ_WORD(Res, Idx) (ReadUnaligned16(&((CONST UINT16 *)(Res)->Data)[Idx]))
#define PMB_READ_32(Res, Idx) (ReadUnaligned32(&((CONST UINT32 *)(Res)->Data)[Idx]))
#define PMB_READ_64(Res, Idx) (ReadUnaligned64(&((CONST UINT64 *)(Res)->Data)[Idx]))

#define PMB_WRITE_BYTE(Res, Idx, Val) ((Res)->Data[Idx] = Val)
// FIXME: currently little-endian machines is only supported
#define PMB_WRITE_WORD(Res, Idx, Val) (WriteUnaligned16(&((UINT16 *)(Res)->Data)[Idx], Val))
#define PMB_WRITE_32(Res, Idx, Val) (WriteUnaligned32(&((UINT32 *)(Res)->Data)[Idx], Val))
#define PMB_WRITE_64(Res, Idx, Val) (WriteUnaligned64(&((UINT64 *)(Res)->Data)[Idx], Val))

/**
	 The struct which describes PMBUS response
 **/
#pragma pack(1)
typedef struct {
	PMB_LEN Size;
	union {
		/** Plain response data **/
		PMB_DAT Raw[PMB_IO_MAX_PKT_LEN];
		/** Response payload data **/
		union {
			/** Response payload plain data **/
			PMB_DAT Data[0];
			/** Response payload block data **/
			struct {
				/** Response block size **/
				PMB_LEN Size;
				PMB_DAT Data[0];
			} Block;
		};
	};
} PMB_IO_RES;
#pragma pack()

/**
	 The struct which describes PMBUS transaction
 **/
typedef struct {
	PMB_IO_REQ Req;
	PMB_IO_RES Res;
} PMB_IO_TRANS;

typedef UINT16 PMB_LINEAR11;

#define PMB_LINEAR11_FROM_PARTS(mantissa, exponent)									\
	((PMB_LINEAR11)(((mantissa) & 0x7ff) | (((exponent) & 0x1f) << 11)))

typedef UINT16 PMB_IEEE754H;

#define PMB_IEEE754H_FROM_PARTS(mantissa, exponent)				       \
	((PMB_IEEE754H)(((((mantissa) < 0 ? -(mantissa) : (mantissa)) & 0x3ff) | \
		       ((((exponent) + 15) & 0x1f) << 10) |		       \
		       ((mantissa) < 0 ? (1 << 15) : 0))))

#endif /* __DTS__ */

#endif /* PMBUS_H_ */
