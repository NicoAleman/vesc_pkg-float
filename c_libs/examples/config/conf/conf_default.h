// This file is autogenerated by VESC Tool

#ifndef CONF_DEFAULT_H_
#define CONF_DEFAULT_H_

// PID Mode
#ifndef APPCONF_BALANCE_PID_MODE
#define APPCONF_BALANCE_PID_MODE 0
#endif

// Angle P
#ifndef APPCONF_BALANCE_KP
#define APPCONF_BALANCE_KP 0
#endif

// Angle I
#ifndef APPCONF_BALANCE_KI
#define APPCONF_BALANCE_KI 0
#endif

// Angle D
#ifndef APPCONF_BALANCE_KD
#define APPCONF_BALANCE_KD 0
#endif

// Rate P
#ifndef APPCONF_BALANCE_KP2
#define APPCONF_BALANCE_KP2 0
#endif

// Rate I
#ifndef APPCONF_BALANCE_KI2
#define APPCONF_BALANCE_KI2 0
#endif

// Rate D
#ifndef APPCONF_BALANCE_KD2
#define APPCONF_BALANCE_KD2 0
#endif

// Loop Hertz
#ifndef APPCONF_BALANCE_HERTZ
#define APPCONF_BALANCE_HERTZ 1000
#endif

// Loop Time Correction Filter
#ifndef APPCONF_BALANCE_LOOP_TIME_FILTER
#define APPCONF_BALANCE_LOOP_TIME_FILTER 0
#endif

// Pitch Axis Fault Cutoff
#ifndef APPCONF_BALANCE_FAULT_PITCH
#define APPCONF_BALANCE_FAULT_PITCH 20
#endif

// Roll Axis Fault Cutoff
#ifndef APPCONF_BALANCE_FAULT_ROLL
#define APPCONF_BALANCE_FAULT_ROLL 45
#endif

// Duty Cycle Fault Cutoff
#ifndef APPCONF_BALANCE_FAULT_DUTY
#define APPCONF_BALANCE_FAULT_DUTY 0.9
#endif

// ADC1 Switch Voltage
#ifndef APPCONF_BALANCE_FAULT_ADC1
#define APPCONF_BALANCE_FAULT_ADC1 0
#endif

// ADC2 Switch Voltage
#ifndef APPCONF_BALANCE_FAULT_ADC2
#define APPCONF_BALANCE_FAULT_ADC2 0
#endif

// Pitch Fault Delay
#ifndef APPCONF_BALANCE_FAULT_DELAY_PITCH
#define APPCONF_BALANCE_FAULT_DELAY_PITCH 0
#endif

// Roll Fault Delay
#ifndef APPCONF_BALANCE_FAULT_DELAY_ROLL
#define APPCONF_BALANCE_FAULT_DELAY_ROLL 0
#endif

// Duty Fault Delay
#ifndef APPCONF_BALANCE_FAULT_DELAY_DUTY
#define APPCONF_BALANCE_FAULT_DELAY_DUTY 0
#endif

// Half Switch Fault Delay
#ifndef APPCONF_BALANCE_FAULT_DELAY_SWITCH_HALF
#define APPCONF_BALANCE_FAULT_DELAY_SWITCH_HALF 0
#endif

// Full Switch Fault Delay
#ifndef APPCONF_BALANCE_FAULT_DELAY_SWITCH_FULL
#define APPCONF_BALANCE_FAULT_DELAY_SWITCH_FULL 0
#endif

// ADC Half State Fault ERPM
#ifndef APPCONF_BALANCE_FAULT_ADC_HALF_ERPM
#define APPCONF_BALANCE_FAULT_ADC_HALF_ERPM 1000
#endif

// Treat both sensors as one
#ifndef APPCONF_BALANCE_FAULT_IS_DUAL_SWITCH
#define APPCONF_BALANCE_FAULT_IS_DUAL_SWITCH 0
#endif

// Angle
#ifndef APPCONF_BALANCE_TILTBACK_DUTY_ANGLE
#define APPCONF_BALANCE_TILTBACK_DUTY_ANGLE 10
#endif

// Speed
#ifndef APPCONF_BALANCE_TILTBACK_DUTY_SPEED
#define APPCONF_BALANCE_TILTBACK_DUTY_SPEED 3
#endif

// Duty Cycle
#ifndef APPCONF_BALANCE_TILTBACK_DUTY
#define APPCONF_BALANCE_TILTBACK_DUTY 0.75
#endif

// Angle
#ifndef APPCONF_BALANCE_TILTBACK_HV_ANGLE
#define APPCONF_BALANCE_TILTBACK_HV_ANGLE 10
#endif

// Speed
#ifndef APPCONF_BALANCE_TILTBACK_HV_SPEED
#define APPCONF_BALANCE_TILTBACK_HV_SPEED 3
#endif

// High Voltage
#ifndef APPCONF_BALANCE_TILTBACK_HV
#define APPCONF_BALANCE_TILTBACK_HV 100
#endif

// Angle
#ifndef APPCONF_BALANCE_TILTBACK_LV_ANGLE
#define APPCONF_BALANCE_TILTBACK_LV_ANGLE 10
#endif

// Speed
#ifndef APPCONF_BALANCE_TILTBACK_LV_SPEED
#define APPCONF_BALANCE_TILTBACK_LV_SPEED 3
#endif

// Low Voltage
#ifndef APPCONF_BALANCE_TILTBACK_LV
#define APPCONF_BALANCE_TILTBACK_LV 0
#endif

// Return To Level Speed
#ifndef APPCONF_BALANCE_TILTBACK_RETURN_SPEED
#define APPCONF_BALANCE_TILTBACK_RETURN_SPEED 1
#endif

// Constant Tiltback
#ifndef APPCONF_BALANCE_TILTBACK_CONSTANT
#define APPCONF_BALANCE_TILTBACK_CONSTANT 0
#endif

// Constant Tiltback ERPM
#ifndef APPCONF_BALANCE_TILTBACK_CONSTANT_ERPM
#define APPCONF_BALANCE_TILTBACK_CONSTANT_ERPM 500
#endif

// Variable Tiltback
#ifndef APPCONF_BALANCE_TILTBACK_VARIABLE
#define APPCONF_BALANCE_TILTBACK_VARIABLE 0
#endif

// Variable Tiltback Maximum
#ifndef APPCONF_BALANCE_TILTBACK_VARIABLE_MAX
#define APPCONF_BALANCE_TILTBACK_VARIABLE_MAX 0
#endif

// Nose Angling Speed
#ifndef APPCONF_BALANCE_NOSEANGLING_SPEED
#define APPCONF_BALANCE_NOSEANGLING_SPEED 5
#endif

// Startup Pitch Axis Angle Tolerance
#ifndef APPCONF_BALANCE_STARTUP_PITCH_TOLERANCE
#define APPCONF_BALANCE_STARTUP_PITCH_TOLERANCE 20
#endif

// Startup Roll Axis Angle Tolerance
#ifndef APPCONF_BALANCE_STARTUP_ROLL_TOLERANCE
#define APPCONF_BALANCE_STARTUP_ROLL_TOLERANCE 8
#endif

// Startup Centering Speed
#ifndef APPCONF_BALANCE_STARTUP_SPEED
#define APPCONF_BALANCE_STARTUP_SPEED 30
#endif

// Deadzone
#ifndef APPCONF_BALANCE_DEADZONE
#define APPCONF_BALANCE_DEADZONE 0
#endif

// Multiple VESCs Over CAN
#ifndef APPCONF_BALANCE_MULTI_ESC
#define APPCONF_BALANCE_MULTI_ESC 0
#endif

// Yaw P
#ifndef APPCONF_BALANCE_YAW_KP
#define APPCONF_BALANCE_YAW_KP 0
#endif

// Yaw I
#ifndef APPCONF_BALANCE_YAW_KI
#define APPCONF_BALANCE_YAW_KI 0
#endif

// Yaw D
#ifndef APPCONF_BALANCE_YAW_KD
#define APPCONF_BALANCE_YAW_KD 0
#endif

// Roll Steer KP
#ifndef APPCONF_BALANCE_ROLL_STEER_KP
#define APPCONF_BALANCE_ROLL_STEER_KP 0
#endif

// Roll Steer ERPM KP
#ifndef APPCONF_BALANCE_ROLL_STEER_ERPM_KP
#define APPCONF_BALANCE_ROLL_STEER_ERPM_KP 0
#endif

// Brake Current
#ifndef APPCONF_BALANCE_BRAKE_CURRENT
#define APPCONF_BALANCE_BRAKE_CURRENT 0
#endif

// Brake Timeout
#ifndef APPCONF_BALANCE_BRAKE_TIMEOUT
#define APPCONF_BALANCE_BRAKE_TIMEOUT 10
#endif

// Yaw Current Clamp
#ifndef APPCONF_BALANCE_YAW_CURRENT_CLAMP
#define APPCONF_BALANCE_YAW_CURRENT_CLAMP 0
#endif

// D term PT1 Low Pass Filter
#ifndef APPCONF_BALANCE_KD_PT1_LOWPASS_FREQUENCY
#define APPCONF_BALANCE_KD_PT1_LOWPASS_FREQUENCY 0
#endif

// D term PT1 High Pass Filter
#ifndef APPCONF_BALANCE_KD_PT1_HIGHPASS_FREQUENCY
#define APPCONF_BALANCE_KD_PT1_HIGHPASS_FREQUENCY 0
#endif

// D term Biquad Low Pass Filter
#ifndef APPCONF_BALANCE_KD_BIQUAD_LOWPASS
#define APPCONF_BALANCE_KD_BIQUAD_LOWPASS 0
#endif

// D term Biquad High Pass Filter
#ifndef APPCONF_BALANCE_KD_BIQUAD_HIGHPASS
#define APPCONF_BALANCE_KD_BIQUAD_HIGHPASS 0
#endif

// Start Angle
#ifndef APPCONF_BALANCE_BOOSTER_ANGLE
#define APPCONF_BALANCE_BOOSTER_ANGLE 8
#endif

// Ramp Up
#ifndef APPCONF_BALANCE_BOOSTER_RAMP
#define APPCONF_BALANCE_BOOSTER_RAMP 1
#endif

// Current Boost
#ifndef APPCONF_BALANCE_BOOSTER_CURRENT
#define APPCONF_BALANCE_BOOSTER_CURRENT 0
#endif

// Start Current Threshold
#ifndef APPCONF_BALANCE_TORQUETILT_START_CURRENT
#define APPCONF_BALANCE_TORQUETILT_START_CURRENT 10
#endif

// Tiltback Angle Limit
#ifndef APPCONF_BALANCE_TORQUETILT_ANGLE_LIMIT
#define APPCONF_BALANCE_TORQUETILT_ANGLE_LIMIT 5
#endif

// Max Tiltback Speed
#ifndef APPCONF_BALANCE_TORQUETILT_ON_SPEED
#define APPCONF_BALANCE_TORQUETILT_ON_SPEED 5
#endif

// Max Tiltback Release Speed
#ifndef APPCONF_BALANCE_TORQUETILT_OFF_SPEED
#define APPCONF_BALANCE_TORQUETILT_OFF_SPEED 3
#endif

// Strength
#ifndef APPCONF_BALANCE_TORQUETILT_STRENGTH
#define APPCONF_BALANCE_TORQUETILT_STRENGTH 0
#endif

// Current Filter
#ifndef APPCONF_BALANCE_TORQUETILT_FILTER
#define APPCONF_BALANCE_TORQUETILT_FILTER 2
#endif

// Strength
#ifndef APPCONF_BALANCE_TURNTILT_STRENGTH
#define APPCONF_BALANCE_TURNTILT_STRENGTH 0
#endif

// Tiltback Angle Limit
#ifndef APPCONF_BALANCE_TURNTILT_ANGLE_LIMIT
#define APPCONF_BALANCE_TURNTILT_ANGLE_LIMIT 5
#endif

// Roll Angle Threshold
#ifndef APPCONF_BALANCE_TURNTILT_START_ANGLE
#define APPCONF_BALANCE_TURNTILT_START_ANGLE 1
#endif

// ERPM Threshold
#ifndef APPCONF_BALANCE_TURNTILT_START_ERPM
#define APPCONF_BALANCE_TURNTILT_START_ERPM 100
#endif

// Max Tiltback Speed
#ifndef APPCONF_BALANCE_TURNTILT_SPEED
#define APPCONF_BALANCE_TURNTILT_SPEED 5
#endif

// Speed Boost %
#ifndef APPCONF_BALANCE_TURNTILT_ERPM_BOOST
#define APPCONF_BALANCE_TURNTILT_ERPM_BOOST 20
#endif

// Speed Boost Max ERPM
#ifndef APPCONF_BALANCE_TURNTILT_ERPM_BOOST_END
#define APPCONF_BALANCE_TURNTILT_ERPM_BOOST_END 20000
#endif

// CONF_DEFAULT_H_
#endif

