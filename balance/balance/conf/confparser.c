// This file is autogenerated by VESC Tool

#include <string.h>
#include "buffer.h"
#include "conf_general.h"
#include "confparser.h"

int32_t confparser_serialize_balance_config(uint8_t *buffer, const balance_config *conf) {
	int32_t ind = 0;

	buffer_append_uint32(buffer, BALANCE_CONFIG_SIGNATURE, &ind);

	buffer[ind++] = conf->pid_mode;
	buffer_append_float32_auto(buffer, conf->kp, &ind);
	buffer_append_float32_auto(buffer, conf->ki, &ind);
	buffer_append_float32_auto(buffer, conf->kp2, &ind);
	buffer_append_float32_auto(buffer, conf->ki2, &ind);
	buffer_append_uint16(buffer, conf->hertz, &ind);
	buffer_append_float32_auto(buffer, conf->fault_pitch, &ind);
	buffer_append_float32_auto(buffer, conf->fault_roll, &ind);
	buffer_append_float32_auto(buffer, conf->fault_adc1, &ind);
	buffer_append_float32_auto(buffer, conf->fault_adc2, &ind);
	buffer_append_uint16(buffer, conf->fault_delay_pitch, &ind);
	buffer_append_uint16(buffer, conf->fault_delay_roll, &ind);
	buffer_append_uint16(buffer, conf->fault_delay_switch_half, &ind);
	buffer_append_uint16(buffer, conf->fault_delay_switch_full, &ind);
	buffer_append_uint16(buffer, conf->fault_adc_half_erpm, &ind);
	buffer[ind++] = conf->fault_is_dual_switch;
	buffer_append_float16(buffer, conf->tiltback_duty_angle, 100, &ind);
	buffer_append_float16(buffer, conf->tiltback_duty_speed, 100, &ind);
	buffer_append_float16(buffer, conf->tiltback_duty, 1000, &ind);
	buffer_append_float16(buffer, conf->tiltback_hv_angle, 100, &ind);
	buffer_append_float16(buffer, conf->tiltback_hv_speed, 100, &ind);
	buffer_append_float32_auto(buffer, conf->tiltback_hv, &ind);
	buffer_append_float16(buffer, conf->tiltback_lv_angle, 100, &ind);
	buffer_append_float16(buffer, conf->tiltback_lv_speed, 100, &ind);
	buffer_append_float32_auto(buffer, conf->tiltback_lv, &ind);
	buffer_append_float16(buffer, conf->tiltback_return_speed, 100, &ind);
	buffer_append_float32_auto(buffer, conf->tiltback_constant, &ind);
	buffer_append_uint16(buffer, conf->tiltback_constant_erpm, &ind);
	buffer_append_float32_auto(buffer, conf->tiltback_variable, &ind);
	buffer_append_float32_auto(buffer, conf->tiltback_variable_max, &ind);
	buffer_append_float16(buffer, conf->noseangling_speed, 100, &ind);
	buffer_append_float32_auto(buffer, conf->startup_pitch_tolerance, &ind);
	buffer_append_float32_auto(buffer, conf->startup_roll_tolerance, &ind);
	buffer_append_float32_auto(buffer, conf->startup_speed, &ind);
	buffer[ind++] = (uint8_t)conf->startup_click_current;
	buffer_append_float32_auto(buffer, conf->brake_current, &ind);
	buffer_append_float32_auto(buffer, conf->ki_limit, &ind);
	buffer_append_float32_auto(buffer, conf->booster_angle, &ind);
	buffer_append_float32_auto(buffer, conf->booster_ramp, &ind);
	buffer_append_float32_auto(buffer, conf->booster_current, &ind);
	buffer_append_float32_auto(buffer, conf->torquetilt_start_current, &ind);
	buffer_append_float32_auto(buffer, conf->torquetilt_angle_limit, &ind);
	buffer_append_float32_auto(buffer, conf->torquetilt_on_speed, &ind);
	buffer_append_float32_auto(buffer, conf->torquetilt_off_speed, &ind);
	buffer_append_float32_auto(buffer, conf->torquetilt_strength, &ind);
	buffer_append_float32_auto(buffer, conf->torquetilt_filter, &ind);
	buffer[ind++] = conf->turntilt_mode;
	buffer_append_float32_auto(buffer, conf->turntilt_strength, &ind);
	buffer_append_float32_auto(buffer, conf->turntilt_angle_limit, &ind);
	buffer_append_float32_auto(buffer, conf->turntilt_start_angle, &ind);
	buffer_append_uint16(buffer, conf->turntilt_start_erpm, &ind);
	buffer_append_float32_auto(buffer, conf->turntilt_speed, &ind);
	buffer_append_uint16(buffer, conf->turntilt_erpm_boost, &ind);
	buffer_append_uint16(buffer, conf->turntilt_erpm_boost_end, &ind);
	buffer[ind++] = (uint8_t)conf->turntilt_yaw_aggregate;

	return ind;
}

bool confparser_deserialize_balance_config(const uint8_t *buffer, balance_config *conf) {
	int32_t ind = 0;

	uint32_t signature = buffer_get_uint32(buffer, &ind);
	if (signature != BALANCE_CONFIG_SIGNATURE) {
		return false;
	}

	conf->pid_mode = buffer[ind++];
	conf->kp = buffer_get_float32_auto(buffer, &ind);
	conf->ki = buffer_get_float32_auto(buffer, &ind);
	conf->kp2 = buffer_get_float32_auto(buffer, &ind);
	conf->ki2 = buffer_get_float32_auto(buffer, &ind);
	conf->hertz = buffer_get_uint16(buffer, &ind);
	conf->fault_pitch = buffer_get_float32_auto(buffer, &ind);
	conf->fault_roll = buffer_get_float32_auto(buffer, &ind);
	conf->fault_adc1 = buffer_get_float32_auto(buffer, &ind);
	conf->fault_adc2 = buffer_get_float32_auto(buffer, &ind);
	conf->fault_delay_pitch = buffer_get_uint16(buffer, &ind);
	conf->fault_delay_roll = buffer_get_uint16(buffer, &ind);
	conf->fault_delay_switch_half = buffer_get_uint16(buffer, &ind);
	conf->fault_delay_switch_full = buffer_get_uint16(buffer, &ind);
	conf->fault_adc_half_erpm = buffer_get_uint16(buffer, &ind);
	conf->fault_is_dual_switch = buffer[ind++];
	conf->tiltback_duty_angle = buffer_get_float16(buffer, 100, &ind);
	conf->tiltback_duty_speed = buffer_get_float16(buffer, 100, &ind);
	conf->tiltback_duty = buffer_get_float16(buffer, 1000, &ind);
	conf->tiltback_hv_angle = buffer_get_float16(buffer, 100, &ind);
	conf->tiltback_hv_speed = buffer_get_float16(buffer, 100, &ind);
	conf->tiltback_hv = buffer_get_float32_auto(buffer, &ind);
	conf->tiltback_lv_angle = buffer_get_float16(buffer, 100, &ind);
	conf->tiltback_lv_speed = buffer_get_float16(buffer, 100, &ind);
	conf->tiltback_lv = buffer_get_float32_auto(buffer, &ind);
	conf->tiltback_return_speed = buffer_get_float16(buffer, 100, &ind);
	conf->tiltback_constant = buffer_get_float32_auto(buffer, &ind);
	conf->tiltback_constant_erpm = buffer_get_uint16(buffer, &ind);
	conf->tiltback_variable = buffer_get_float32_auto(buffer, &ind);
	conf->tiltback_variable_max = buffer_get_float32_auto(buffer, &ind);
	conf->noseangling_speed = buffer_get_float16(buffer, 100, &ind);
	conf->startup_pitch_tolerance = buffer_get_float32_auto(buffer, &ind);
	conf->startup_roll_tolerance = buffer_get_float32_auto(buffer, &ind);
	conf->startup_speed = buffer_get_float32_auto(buffer, &ind);
	conf->startup_click_current = buffer[ind++];
	conf->brake_current = buffer_get_float32_auto(buffer, &ind);
	conf->ki_limit = buffer_get_float32_auto(buffer, &ind);
	conf->booster_angle = buffer_get_float32_auto(buffer, &ind);
	conf->booster_ramp = buffer_get_float32_auto(buffer, &ind);
	conf->booster_current = buffer_get_float32_auto(buffer, &ind);
	conf->torquetilt_start_current = buffer_get_float32_auto(buffer, &ind);
	conf->torquetilt_angle_limit = buffer_get_float32_auto(buffer, &ind);
	conf->torquetilt_on_speed = buffer_get_float32_auto(buffer, &ind);
	conf->torquetilt_off_speed = buffer_get_float32_auto(buffer, &ind);
	conf->torquetilt_strength = buffer_get_float32_auto(buffer, &ind);
	conf->torquetilt_filter = buffer_get_float32_auto(buffer, &ind);
	conf->turntilt_mode = buffer[ind++];
	conf->turntilt_strength = buffer_get_float32_auto(buffer, &ind);
	conf->turntilt_angle_limit = buffer_get_float32_auto(buffer, &ind);
	conf->turntilt_start_angle = buffer_get_float32_auto(buffer, &ind);
	conf->turntilt_start_erpm = buffer_get_uint16(buffer, &ind);
	conf->turntilt_speed = buffer_get_float32_auto(buffer, &ind);
	conf->turntilt_erpm_boost = buffer_get_uint16(buffer, &ind);
	conf->turntilt_erpm_boost_end = buffer_get_uint16(buffer, &ind);
	conf->turntilt_yaw_aggregate = buffer[ind++];

	return true;
}

void confparser_set_defaults_balance_config(balance_config *conf) {
	conf->pid_mode = APPCONF_BALANCE_PID_MODE;
	conf->kp = APPCONF_BALANCE_KP;
	conf->ki = APPCONF_BALANCE_KI;
	conf->kp2 = APPCONF_BALANCE_KP2;
	conf->ki2 = APPCONF_BALANCE_KI2;
	conf->hertz = APPCONF_BALANCE_HERTZ;
	conf->fault_pitch = APPCONF_BALANCE_FAULT_PITCH;
	conf->fault_roll = APPCONF_BALANCE_FAULT_ROLL;
	conf->fault_adc1 = APPCONF_BALANCE_FAULT_ADC1;
	conf->fault_adc2 = APPCONF_BALANCE_FAULT_ADC2;
	conf->fault_delay_pitch = APPCONF_BALANCE_FAULT_DELAY_PITCH;
	conf->fault_delay_roll = APPCONF_BALANCE_FAULT_DELAY_ROLL;
	conf->fault_delay_switch_half = APPCONF_BALANCE_FAULT_DELAY_SWITCH_HALF;
	conf->fault_delay_switch_full = APPCONF_BALANCE_FAULT_DELAY_SWITCH_FULL;
	conf->fault_adc_half_erpm = APPCONF_BALANCE_FAULT_ADC_HALF_ERPM;
	conf->fault_is_dual_switch = APPCONF_BALANCE_FAULT_IS_DUAL_SWITCH;
	conf->tiltback_duty_angle = APPCONF_BALANCE_TILTBACK_DUTY_ANGLE;
	conf->tiltback_duty_speed = APPCONF_BALANCE_TILTBACK_DUTY_SPEED;
	conf->tiltback_duty = APPCONF_BALANCE_TILTBACK_DUTY;
	conf->tiltback_hv_angle = APPCONF_BALANCE_TILTBACK_HV_ANGLE;
	conf->tiltback_hv_speed = APPCONF_BALANCE_TILTBACK_HV_SPEED;
	conf->tiltback_hv = APPCONF_BALANCE_TILTBACK_HV;
	conf->tiltback_lv_angle = APPCONF_BALANCE_TILTBACK_LV_ANGLE;
	conf->tiltback_lv_speed = APPCONF_BALANCE_TILTBACK_LV_SPEED;
	conf->tiltback_lv = APPCONF_BALANCE_TILTBACK_LV;
	conf->tiltback_return_speed = APPCONF_BALANCE_TILTBACK_RETURN_SPEED;
	conf->tiltback_constant = APPCONF_BALANCE_TILTBACK_CONSTANT;
	conf->tiltback_constant_erpm = APPCONF_BALANCE_TILTBACK_CONSTANT_ERPM;
	conf->tiltback_variable = APPCONF_BALANCE_TILTBACK_VARIABLE;
	conf->tiltback_variable_max = APPCONF_BALANCE_TILTBACK_VARIABLE_MAX;
	conf->noseangling_speed = APPCONF_BALANCE_NOSEANGLING_SPEED;
	conf->startup_pitch_tolerance = APPCONF_BALANCE_STARTUP_PITCH_TOLERANCE;
	conf->startup_roll_tolerance = APPCONF_BALANCE_STARTUP_ROLL_TOLERANCE;
	conf->startup_speed = APPCONF_BALANCE_STARTUP_SPEED;
	conf->startup_click_current = APPCONF_BALANCE_STARTUP_CLICK_CURRENT;
	conf->brake_current = APPCONF_BALANCE_BRAKE_CURRENT;
	conf->ki_limit = APPCONF_BALANCE_KI_LIMIT;
	conf->booster_angle = APPCONF_BALANCE_BOOSTER_ANGLE;
	conf->booster_ramp = APPCONF_BALANCE_BOOSTER_RAMP;
	conf->booster_current = APPCONF_BALANCE_BOOSTER_CURRENT;
	conf->torquetilt_start_current = APPCONF_BALANCE_TORQUETILT_START_CURRENT;
	conf->torquetilt_angle_limit = APPCONF_BALANCE_TORQUETILT_ANGLE_LIMIT;
	conf->torquetilt_on_speed = APPCONF_BALANCE_TORQUETILT_ON_SPEED;
	conf->torquetilt_off_speed = APPCONF_BALANCE_TORQUETILT_OFF_SPEED;
	conf->torquetilt_strength = APPCONF_BALANCE_TORQUETILT_STRENGTH;
	conf->torquetilt_filter = APPCONF_BALANCE_TORQUETILT_FILTER;
	conf->turntilt_mode = APPCONF_BALANCE_TURNTILT_MODE;
	conf->turntilt_strength = APPCONF_BALANCE_TURNTILT_STRENGTH;
	conf->turntilt_angle_limit = APPCONF_BALANCE_TURNTILT_ANGLE_LIMIT;
	conf->turntilt_start_angle = APPCONF_BALANCE_TURNTILT_START_ANGLE;
	conf->turntilt_start_erpm = APPCONF_BALANCE_TURNTILT_START_ERPM;
	conf->turntilt_speed = APPCONF_BALANCE_TURNTILT_SPEED;
	conf->turntilt_erpm_boost = APPCONF_BALANCE_TURNTILT_ERPM_BOOST;
	conf->turntilt_erpm_boost_end = APPCONF_BALANCE_TURNTILT_ERPM_BOOST_END;
	conf->turntilt_yaw_aggregate = APPCONF_BALANCE_TURNTILT_YAW_AGGREGATE;
}

