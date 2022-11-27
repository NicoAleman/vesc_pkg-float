/*
    Copyright 2019 - 2022 Mitch Lustig
	Copyright 2022 Benjamin Vedder	benjamin@vedder.se

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vesc_c_if.h"

#include "conf/datatypes.h"
#include "conf/confparser.h"
#include "conf/confxml.h"
#include "conf/buffer.h"

#include <math.h>
#include <string.h>

// Acceleration average
#define ACCEL_ARRAY_SIZE 40

HEADER

// Return the sign of the argument. -1.0 if negative, 1.0 if zero or positive.
#define SIGN(x)				(((x) < 0.0) ? -1.0 : 1.0)

#define DEG2RAD_f(deg)		((deg) * (float)(M_PI / 180.0))
#define RAD2DEG_f(rad) 		((rad) * (float)(180.0 / M_PI))

// Data type
typedef enum {
	STARTUP = 0,
	RUNNING = 1,
	RUNNING_TILTBACK = 2,
	RUNNING_WHEELSLIP = 3,
	RUNNING_UPSIDEDOWN = 4,
	FAULT_ANGLE_PITCH = 5,
	FAULT_ANGLE_ROLL = 6,
	FAULT_SWITCH_HALF = 7,
	FAULT_SWITCH_FULL = 8,
	FAULT_STARTUP = 9,
	FAULT_REVERSE = 10,
	FAULT_QUICKSTOP = 11
} FloatState;

typedef enum {
	CENTERING = 0,
	REVERSESTOP,
	TILTBACK_DUTY,
	TILTBACK_HV,
	TILTBACK_LV,
	TILTBACK_NONE
} SetpointAdjustmentType;

typedef enum {
	OFF = 0,
	HALF,
	ON
} SwitchState;

typedef struct{
	float a0, a1, a2, b1, b2;
	float z1, z2;
} Biquad;

typedef enum {
	BQ_LOWPASS,
	BQ_HIGHPASS
} BiquadType;

// This is all persistent state of the application, which will be allocated in init. It
// is put here because variables can only be read-only when this program is loaded
// in flash without virtual memory in RAM (as all RAM already is dedicated to the
// main firmware and managed from there). This is probably the main limitation of
// loading applications in runtime, but it is not too bad to work around.
typedef struct {
	lib_thread thread; // Float Thread

	float_config float_conf;

	// Config values
	float loop_time_seconds;
	unsigned int start_counter_clicks, start_counter_clicks_max, start_click_current;
	float startup_step_size;
	float tiltback_duty_step_size, tiltback_hv_step_size, tiltback_lv_step_size, tiltback_return_step_size;
	float torquetilt_on_step_size, torquetilt_off_step_size, turntilt_step_size;
	float tiltback_variable, tiltback_variable_max_erpm, noseangling_step_size;
	float mc_max_temp_fet, mc_max_temp_mot;
	float mc_current_max, mc_current_min, max_continuous_current;
	bool current_beeping;

	// Feature: True Pitch
	ATTITUDE_INFO m_att_ref;

	// Runtime values read from elsewhere
	float pitch_angle, last_pitch_angle, roll_angle, abs_roll_angle, abs_roll_angle_sin, last_gyro_y;
 	float true_pitch_angle;
	float gyro[3];
	float duty_cycle, abs_duty_cycle;
	float erpm, abs_erpm, avg_erpm;
	float motor_current;
	float motor_position;
	float adc1, adc2;
	float max_duty_with_margin;
	SwitchState switch_state;

	// Feature: ATR (Adaptive Torque Response)
	float atr_on_step_size, atr_off_step_size;
	float acceleration, last_erpm;
	float accel_gap;
	float accelhist[ACCEL_ARRAY_SIZE];
	float accelavg;
	float tt_accel_factor;
	int accelidx;
	int direction_counter;
	bool braking;
	bool atr_disable;

	// Feature: Turntilt
	float last_yaw_angle, yaw_angle, abs_yaw_change, last_yaw_change, yaw_change, yaw_aggregate;
	float turntilt_boost_per_erpm, yaw_aggregate_target;
	float turntilt_strength;

	// Rumtime state values
	FloatState state;
	float proportional, integral, proportional2, integral2, pid_rate;
	float last_proportional, abs_proportional;
	float pid_value;
	float setpoint, setpoint_target, setpoint_target_interpolated;
	float applied_booster_current;
	float noseangling_interpolated;
	float torquetilt_filtered_current, torquetilt_target, torquetilt_interpolated;
	Biquad torquetilt_current_biquad;
	Biquad atr_current_biquad;
	float braketilt_factor, braketilt_target, braketilt_interpolated;
	float turntilt_target, turntilt_interpolated;
	SetpointAdjustmentType setpointAdjustmentType;
	float current_time, last_time, diff_time, loop_overshoot; // Seconds
	float disengage_timer; // Seconds
	float filtered_loop_overshoot, loop_overshoot_alpha, filtered_diff_time;
	float fault_angle_pitch_timer, fault_angle_roll_timer, fault_switch_timer, fault_switch_half_timer; // Seconds
	float motor_timeout_seconds;
	float brake_timeout; // Seconds
	float wheelslip_timer, wheelslip_end_timer, overcurrent_timer, tb_highvoltage_timer;
	float switch_warn_buzz_erpm;
	float quickstop_erpm;
	bool traction_control;

	// Darkride aka upside down mode:
	bool is_upside_down;			// the board is upside down
	bool is_upside_down_started;	// dark ride has been engaged
	bool enable_upside_down;		// dark ride mode is enabled (10 seconds after fault)
	bool allow_upside_down;			// dark ride mode feature is ON (roll=180) 
	float delay_upside_down_fault;

	// Feature: Reverse Stop
	float reverse_stop_step_size, reverse_tolerance, reverse_total_erpm;
	float reverse_timer;

	// Brake Amp Rate Limiting:
	float pid_brake_increment;

	// Log values
	float float_setpoint, float_atr, float_braketilt;
	float float_expected_acc, float_measured_acc, float_acc_diff;

	// Debug values
	int debug_render_1, debug_render_2;
	int debug_sample_field, debug_sample_count, debug_sample_index;
	int debug_experiment_1, debug_experiment_2, debug_experiment_3, debug_experiment_4, debug_experiment_5, debug_experiment_6;
} data;

// Function Prototypes
static void set_current(data *d, float current);
static void configure(data *d);

// Utility Functions
static float biquad_process(Biquad *biquad, float in) {
    float out = in * biquad->a0 + biquad->z1;
    biquad->z1 = in * biquad->a1 + biquad->z2 - biquad->b1 * out;
    biquad->z2 = in * biquad->a2 - biquad->b2 * out;
    return out;
}

static void biquad_config(Biquad *biquad, BiquadType type, float Fc) {
	float K = tanf(M_PI * Fc);	// -0.0159;
	float Q = 0.707; // maximum sharpness (0.5 = maximum smoothness)
	float norm = 1 / (1 + K / Q + K * K);
	if (type == BQ_LOWPASS) {
		biquad->a0 = K * K * norm;
		biquad->a1 = 2 * biquad->a0;
		biquad->a2 = biquad->a0;
	}
	else if (type == BQ_HIGHPASS) {
		biquad->a0 = 1 * norm;
		biquad->a1 = -2 * biquad->a0;
		biquad->a2 = biquad->a0;
	}
	biquad->b1 = 2 * (K * K - 1) * norm;
	biquad->b2 = (1 - K / Q + K * K) * norm;
}

static void biquad_reset(Biquad *biquad) {
	biquad->z1 = 0;
	biquad->z2 = 0;
}

static void configure(data *d) {
	d->debug_render_1 = 2;
	d->debug_render_2 = 4;

	// Set calculated values from config
	d->loop_time_seconds = 1.0 / d->float_conf.hertz;

	d->motor_timeout_seconds = d->loop_time_seconds * 20; // Times 20 for a nice long grace period

	d->startup_step_size = d->float_conf.startup_speed / d->float_conf.hertz;
	d->tiltback_duty_step_size = d->float_conf.tiltback_duty_speed / d->float_conf.hertz;
	d->tiltback_hv_step_size = d->float_conf.tiltback_hv_speed / d->float_conf.hertz;
	d->tiltback_lv_step_size = d->float_conf.tiltback_lv_speed / d->float_conf.hertz;
	d->tiltback_return_step_size = d->float_conf.tiltback_return_speed / d->float_conf.hertz;
	d->torquetilt_on_step_size = d->float_conf.torquetilt_on_speed / d->float_conf.hertz;
	d->torquetilt_off_step_size = d->float_conf.torquetilt_off_speed / d->float_conf.hertz;
	d->atr_on_step_size = d->float_conf.atr_on_speed / d->float_conf.hertz;
	d->atr_off_step_size = d->float_conf.atr_off_speed / d->float_conf.hertz;
	d->turntilt_step_size = d->float_conf.turntilt_speed / d->float_conf.hertz;
	d->noseangling_step_size = d->float_conf.noseangling_speed / d->float_conf.hertz;

	// Feature: Stealthy start vs normal start (noticeable click when engaging) - 0-20A
	d->start_click_current = d->float_conf.startup_click_current;
	d->start_counter_clicks_max = 3;

	/* WAITING ON FIRMWARE SUPPORT ///////////////////////////////////////////////
	d->mc_max_temp_fet = VESC_IF->get_cfg_float(CFG_PARAM_l_temp_fet_start) - 3;
	d->mc_max_temp_mot = VESC_IF->get_cfg_float(CFG_PARAM_l_temp_motor_start) - 3;
	/////////////////////////////////////////////// WAITING ON FIRMWARE SUPPORT */

	d->mc_current_max = VESC_IF->get_cfg_float(CFG_PARAM_l_current_max);
	int mcm = d->mc_current_max;
	float mc_max_reduce = d->mc_current_max - mcm;
	if (mc_max_reduce >= 0.5) {
		// reduce the max current by X% to save that for torque tilt situations
		// less than 60 peak amps makes no sense though so I'm not allowing it
		d->mc_current_max = fmaxf(mc_max_reduce * d->mc_current_max, 60);
	}
    
	// min current is a positive value here!
	d->mc_current_min = fabsf(VESC_IF->get_cfg_float(CFG_PARAM_l_current_min));
	mcm = d->mc_current_min;
	float mc_min_reduce = fabsf(d->mc_current_min - mcm);
	if (mc_min_reduce >= 0.5) {
		// reduce the max current by X% to save that for torque tilt situations
		// less than 50 peak breaking amps makes no sense though so I'm not allowing it
		d->mc_current_min = fmaxf(mc_min_reduce * d->mc_current_min, 50);
	}

	// Decimals of abs-max specify max continuous current
	float max_abs = VESC_IF->get_cfg_float(CFG_PARAM_l_abs_current_max);
	int mabs = max_abs;
	d->max_continuous_current = (max_abs - mabs) * 100;
	if (d->max_continuous_current < 25) {
		// anything below 25A is suspicious and will be ignored!
		d->max_continuous_current = d->mc_current_max;
	}
	

	// Maximum amps change when braking
	d->pid_brake_increment = 5;
	if (d->pid_brake_increment < 0.1) {
		d->pid_brake_increment = 5;
	}

	// d->max_duty_with_margin =VESC_IF->get_cfg_float(CFG_PARAM_l_max_duty); - 0.1; /* NEEDS FW SUPPORT */
	d->max_duty_with_margin = 0.9; /* TEMPORARY, NEEDS FW SUPPORT */


	/* WIP /////////////////////////////

	// INSERT ASYMMETRICAL BOOSTER INIT's

	///////////////////////////// WIP */
	
	// Feature: Reverse Stop
	d->reverse_tolerance = 50000;
	d->reverse_stop_step_size = 100.0 / d->float_conf.hertz;

	// Init Filters
	float loop_time_filter = 3.0; // Originally Parameter, now hard-coded to 3Hz
	d->loop_overshoot_alpha = 2.0 * M_PI * ((float)1.0 / (float)d->float_conf.hertz) *
				loop_time_filter / (2.0 * M_PI * (1.0 / (float)d->float_conf.hertz) *
						loop_time_filter + 1.0);

	if (d->float_conf.torquetilt_filter > 0) { // Torquetilt Current Biquad
		float Fc = d->float_conf.torquetilt_filter / d->float_conf.hertz;
		biquad_config(&d->torquetilt_current_biquad, BQ_LOWPASS, Fc);
	}

	if (d->float_conf.atr_filter > 0) { // Torquetilt / ATR Current Biquad
		float Fc = d->float_conf.atr_filter / d->float_conf.hertz;
		biquad_config(&d->atr_current_biquad, BQ_LOWPASS, Fc);
	}

	// Feature: ATR:
	d->tt_accel_factor = d->float_conf.atr_amps_accel_ratio;	// how many amps per acc?
	d->atr_disable = (d->float_conf.atr_strength == 0);
	d->float_acc_diff = 0;

	/* INSERT OG TT LOGIC? */

	// Feature: Braketilt
	d->braketilt_factor = d->float_conf.braketilt_strength;
	d->braketilt_factor = 20 - d->braketilt_factor;
	// incorporate negative sign into braketilt factor instead of adding it each balance loop
	d->braketilt_factor = -(0.5 + d->braketilt_factor / 5.0);

	// Feature: Turntilt
	d->yaw_aggregate_target = fmaxf(50, d->float_conf.turntilt_yaw_aggregate);
	d->turntilt_boost_per_erpm = (float)d->float_conf.turntilt_erpm_boost / 100.0 / (float)d->float_conf.turntilt_erpm_boost_end;
	d->turntilt_strength = d->float_conf.turntilt_strength;

	// Feature: Darkride
	d->allow_upside_down = (d->float_conf.fault_darkride_enabled);
	d->enable_upside_down = false;
	d->is_upside_down = false;

	// Speed at which to warn users about an impending full switch fault
	d->switch_warn_buzz_erpm = 2000;

	// Speed below which we check for quickstop conditions
	d->quickstop_erpm = 200;

	// Variable nose angle adjustment / tiltback (setting is per 1000erpm, convert to per erpm)
	d->tiltback_variable = d->float_conf.tiltback_variable / 1000;
	if (d->tiltback_variable > 0) {
		d->tiltback_variable_max_erpm = fabsf(d->float_conf.tiltback_variable_max / d->tiltback_variable);
	} else {
		d->tiltback_variable_max_erpm = 100000;
	}

	// Reset loop time variables
	d->last_time = 0.0;
	d->filtered_loop_overshoot = 0.0;
}

static void reset_vars(data *d) {
	// Clear accumulated values.
	d->integral = 0;
	d->last_proportional = 0;
	d->integral2 = 0;
	// Set values for startup
	d->setpoint = d->pitch_angle;
	d->setpoint_target_interpolated = d->pitch_angle;
	d->setpoint_target = 0;
	d->noseangling_interpolated = 0;
	d->torquetilt_target = 0;
	d->torquetilt_interpolated = 0;
	d->torquetilt_filtered_current = 0;
	biquad_reset(&d->torquetilt_current_biquad);
	biquad_reset(&d->atr_current_biquad);
	d->braketilt_target = 0;
	d->braketilt_interpolated = 0;
	d->turntilt_target = 0;
	d->turntilt_interpolated = 0;
	d->setpointAdjustmentType = CENTERING;
	d->state = RUNNING;
	d->current_time = 0;
	d->last_time = 0;
	d->diff_time = 0;
	d->brake_timeout = 0;
	d->traction_control = false;
	d->pid_value = 0;

	// ATR:
	d->accel_gap = 0;
	d->direction_counter = 0;

	for (int i = 0; i < 40; i++)
		d->accelhist[i] = 0;
	d->accelidx = 0;
	d->accelavg = 0;

	// Turntilt:
	d->last_yaw_angle = 0;
	d->yaw_aggregate = 0;

	// Feature: click on start
	d->start_counter_clicks = d->start_counter_clicks_max;
}

static float get_setpoint_adjustment_step_size(data *d) {
	switch(d->setpointAdjustmentType){
		case (CENTERING):
			return d->startup_step_size;
		case (TILTBACK_DUTY):
			return d->tiltback_duty_step_size;
		case (TILTBACK_HV):
			return d->tiltback_hv_step_size;
		case (TILTBACK_LV):
			return d->tiltback_lv_step_size;
		case (TILTBACK_NONE):
			return d->tiltback_return_step_size;
		case (REVERSESTOP):
			return d->reverse_stop_step_size;
		default:
			;
	}
	return 0;
}

// Read ADCs and determine switch state
static SwitchState check_adcs(data *d) {
	SwitchState sw_state;

	// Calculate switch state from ADC values
	if(d->float_conf.fault_adc1 == 0 && d->float_conf.fault_adc2 == 0){ // No Switch
		sw_state = ON;
	}else if(d->float_conf.fault_adc2 == 0){ // Single switch on ADC1
		if(d->adc1 > d->float_conf.fault_adc1){
			sw_state = ON;
		} else {
			sw_state = OFF;
		}
	}else if(d->float_conf.fault_adc1 == 0){ // Single switch on ADC2
		if(d->adc2 > d->float_conf.fault_adc2){
			sw_state = ON;
		} else {
			sw_state = OFF;
		}
	}else{ // Double switch
		if(d->adc1 > d->float_conf.fault_adc1 && d->adc2 > d->float_conf.fault_adc2){
			sw_state = ON;
		}else if(d->adc1 > d->float_conf.fault_adc1 || d->adc2 > d->float_conf.fault_adc2){
			// 5 seconds after stopping we allow starting with a single sensor (e.g. for jump starts)
			bool is_simple_start = d->current_time - d->disengage_timer > 5;
			if (d->float_conf.fault_is_dual_switch || is_simple_start)
				sw_state = ON;
			else
				sw_state = HALF;
		}else{
			sw_state = OFF;
		}
	}

	/* INSERT DADO's BUZZER LOGIC *//////////////////////////////////////////////////
	// /*
	// * Use external buzzer to notify rider of foot switch faults.
	// */
	// #ifdef HAS_EXT_BUZZER
	//		if (switch_state == OFF) {
	//			if (abs_erpm > switch_warn_buzz_erpm) {
	//				// If we're at riding speed and the switch is off => ALERT the user
	//				// set force=true since this could indicate an imminent shutdown/nosedive
	//				beep_on(true);
	//			}
	//			else {
	//				// if we drop below riding speed stop buzzing
	//				beep_off(false);
	//			}
	//		}
	//		else {
	//			// if the switch comes back on we stop buzzing
	//			beep_off(false);
	//		}
	// #endif
	/////////////////////////////////////////////////////////////////////////////////

	return sw_state;
}

// Fault checking order does not really matter. From a UX perspective, switch should be before angle.
static bool check_faults(data *d, bool ignoreTimers){
	// Aggressive reverse stop in case the board runs off when upside down
	if (d->is_upside_down) {
		if (d->erpm > 1000) {
			// erpms are also reversed when upside down!
			if (((d->current_time - d->fault_switch_timer) * 1000 > 100) ||
				(d->erpm > 2000) ||
				((d->state == RUNNING_WHEELSLIP) && (d->current_time - d->delay_upside_down_fault > 1) &&
				 ((d->current_time - d->fault_switch_timer) * 1000 > 30)) ) {
				
				// Trigger FAULT_REVERSE when board is going reverse AND
				// going > 2mph for more than 100ms
				// going > 4mph
				// detecting wheelslip (aka excorcist wiggle) after the first second
				d->state = FAULT_REVERSE;
				return true;
			}
		}
		else {
			d->fault_switch_timer = d->current_time;
			if (d->erpm > 300) {
				// erpms are also reversed when upside down!
				if ((d->current_time - d->fault_angle_roll_timer) * 1000 > 500){
					d->state = FAULT_REVERSE;
					return true;
				}
			}
			else {
				d->fault_angle_roll_timer = d->current_time;
			}
		}
		if (d->switch_state == ON) {
			// allow turning it off by engaging foot sensors
			d->state = FAULT_SWITCH_HALF;
			return true;
		}
	}
	else {
		// Check switch
		// Switch fully open
		if (d->switch_state == OFF) {
			if((1000.0 * (d->current_time - d->fault_switch_timer)) > d->float_conf.fault_delay_switch_full || ignoreTimers){
				d->state = FAULT_SWITCH_FULL;
				return true;
			}
			// low speed (below 6 x half-fault threshold speed):
			else if ((d->abs_erpm < d->float_conf.fault_adc_half_erpm * 6)
				&& (1000.0 * (d->current_time - d->fault_switch_timer) > d->float_conf.fault_delay_switch_half)){
				d->state = FAULT_SWITCH_FULL;
				return true;
			}
			else if ((d->abs_erpm < d->quickstop_erpm) && (fabsf(d->true_pitch_angle) > 14) && (SIGN(d->true_pitch_angle) == SIGN(d->erpm))) {
				// QUICK STOP
				d->state = FAULT_QUICKSTOP;
				return true;
			}
		} else {
			d->fault_switch_timer = d->current_time;
		}

		// Feature: Reverse-Stop
		if(d->setpointAdjustmentType == REVERSESTOP){
			//  Taking your foot off entirely while reversing? Ignore delays
			if (d->switch_state == OFF) {
				d->state = FAULT_SWITCH_FULL;
				return true;
			}
			if (fabsf(d->true_pitch_angle) > 15) {
				d->state = FAULT_REVERSE;
				return true;
			}
			// Above 10 degrees for a half a second? Switch it off
			if ((fabsf(d->true_pitch_angle) > 10) && (d->current_time - d->reverse_timer > .5)) {
				d->state = FAULT_REVERSE;
				return true;
			}
			// Above 5 degrees for a full second? Switch it off
			if ((fabsf(d->true_pitch_angle) > 5) && (d->current_time - d->reverse_timer > 1)) {
				d->state = FAULT_REVERSE;
				return true;
			}
			if (d->reverse_total_erpm > d->reverse_tolerance * 3) {
				d->state = FAULT_REVERSE;
				return true;
			}
			if (fabsf(d->true_pitch_angle) < 5) {
				d->reverse_timer = d->current_time;
			}
		}

		// Switch partially open and stopped
		if(!d->float_conf.fault_is_dual_switch) {
			if((d->switch_state == HALF || d->switch_state == OFF) && d->abs_erpm < d->float_conf.fault_adc_half_erpm){
				if ((1000.0 * (d->current_time - d->fault_switch_half_timer)) > d->float_conf.fault_delay_switch_half || ignoreTimers){
					d->state = FAULT_SWITCH_HALF;
					return true;
				}
			} else {
				d->fault_switch_half_timer = d->current_time;
			}
		}

		// Check roll angle
		if (fabsf(d->roll_angle) > d->float_conf.fault_roll) {
			if ((1000.0 * (d->current_time - d->fault_angle_roll_timer)) > d->float_conf.fault_delay_roll || ignoreTimers) {
				d->state = FAULT_ANGLE_ROLL;
				return true;
			}
		} else {
			d->fault_angle_roll_timer = d->current_time;

			if (d->allow_upside_down) {
				if((fabsf(d->roll_angle) > 100) && (fabsf(d->roll_angle) < 135)) {
					d->state = FAULT_ANGLE_ROLL;
					return true;
				}
			}
		}
	}

	// Check pitch angle
	if (fabsf(d->true_pitch_angle) > d->float_conf.fault_pitch) {
		if ((1000.0 * (d->current_time - d->fault_angle_pitch_timer)) > d->float_conf.fault_delay_pitch || ignoreTimers) {
			d->state = FAULT_ANGLE_PITCH;
			return true;
		}
	} else {
		d->fault_angle_pitch_timer = d->current_time;
	}

	// *Removed Duty Cycle Fault*

	return false;
}

static void calculate_setpoint_target(data *d) {
	float input_voltage = VESC_IF->mc_get_input_voltage_filtered();

	if (input_voltage < d->float_conf.tiltback_hv) {
		d->tb_highvoltage_timer = d->current_time;
	}

	if (d->setpointAdjustmentType == CENTERING && d->setpoint_target_interpolated != d->setpoint_target) {
		// Ignore tiltback during centering sequence
		d->state = RUNNING;
	} else if (d->setpointAdjustmentType == REVERSESTOP) {
		// accumalete erpms:
		d->reverse_total_erpm += d->erpm;
		if (fabsf(d->reverse_total_erpm) > d->reverse_tolerance) {
			// tilt down by 10 degrees after 50k aggregate erpm
			d->setpoint_target = 10 * (fabsf(d->reverse_total_erpm) - d->reverse_tolerance) / 50000;
		}
		else {
			if (fabsf(d->reverse_total_erpm) <= d->reverse_tolerance / 2) {
				if (d->erpm >= 0){
					d->setpointAdjustmentType = TILTBACK_NONE;
					d->reverse_total_erpm = 0;
					d->setpoint_target = 0;
					d->integral = 0;
				}
			}
		}
	} else if ((fabs(d->acceleration) > 10) &&					// this isn't normal, either wheelslip or wheel getting stuck
			  (SIGN(d->acceleration) == SIGN(d->erpm)) &&		// we only act on wheelslip, not when the wheel gets stuck
			  (d->abs_duty_cycle > 0.3) &&
			  (d->abs_erpm > 1500))								// acceleration can jump a lot at very low speeds
	{
		d->state = RUNNING_WHEELSLIP;
		d->setpointAdjustmentType = TILTBACK_NONE;
		d->wheelslip_timer = d->current_time;
		if (d->is_upside_down) {
			d->traction_control = true;
		}
	} else if (d->state == RUNNING_WHEELSLIP) {
		if (fabsf(d->acceleration) < 10) {
			// acceleration is slowing down, traction control seems to have worked
			d->traction_control = false;
		}
		// Remain in wheelslip state for at least 500ms to avoid any overreactions
		if (d->abs_duty_cycle > d->max_duty_with_margin) {
			d->wheelslip_timer = d->current_time;
		}
		else if (d->current_time - d->wheelslip_timer > 0.5) {
			if (d->abs_duty_cycle < 0.7) {
				// Leave wheelslip state only if duty < 70%
				d->traction_control = false;
				d->state = RUNNING;
			}
		}
	} else if (d->abs_duty_cycle > d->float_conf.tiltback_duty) {
		if (d->erpm > 0) {
			d->setpoint_target = d->float_conf.tiltback_duty_angle;
		} else {
			d->setpoint_target = -d->float_conf.tiltback_duty_angle;
		}
		d->setpointAdjustmentType = TILTBACK_DUTY;
		d->state = RUNNING_TILTBACK;
	} else if (d->abs_duty_cycle > 0.05 && input_voltage > d->float_conf.tiltback_hv) {
		if (((d->current_time - d->tb_highvoltage_timer) > .5) ||
		   (input_voltage > d->float_conf.tiltback_hv + 1)) {
			// 500ms have passed or voltage is another volt higher, time for some tiltback
			if (d->erpm > 0){
				d->setpoint_target = d->float_conf.tiltback_hv_angle;
			} else {
				d->setpoint_target = -d->float_conf.tiltback_hv_angle;
			}

			d->setpointAdjustmentType = TILTBACK_HV;
			d->state = RUNNING_TILTBACK;
		}
		else {
			// Was possibly just a short spike
			d->setpointAdjustmentType = TILTBACK_NONE;
			d->state = RUNNING;
		}
	} else if (d->abs_duty_cycle > 0.05 && input_voltage < d->float_conf.tiltback_lv) {
		
		float abs_motor_current = fabsf(d->motor_current);
		float vdelta = d->float_conf.tiltback_lv - input_voltage;
		float ratio = vdelta * 20 / abs_motor_current;
		// When to do LV tiltback:
		// a) we're 2V below lv threshold
		// b) motor current is small (we cannot assume vsag)
		// c) we have more than 20A per Volt of difference (we tolerate some amount of vsag)
		if ((vdelta > 2) || (abs_motor_current < 5) || (ratio > 1)) {
			if (d->erpm > 0) {
				d->setpoint_target = d->float_conf.tiltback_lv_angle;
			} else {
				d->setpoint_target = -d->float_conf.tiltback_lv_angle;
			}

			d->setpointAdjustmentType = TILTBACK_LV;
			d->state = RUNNING_TILTBACK;
		}
		else {
			d->setpointAdjustmentType = TILTBACK_NONE;
			d->setpoint_target = 0;
			d->state = RUNNING;
		}
	} else {
		// Normal running
		if (d->float_conf.fault_reversestop_enabled && (d->erpm < -200) && !d->is_upside_down) {
			d->setpointAdjustmentType = REVERSESTOP;
			d->reverse_timer = d->current_time;
			d->reverse_total_erpm = 0;
		}
		else {
			d->setpointAdjustmentType = TILTBACK_NONE;
		}
		d->setpoint_target = 0;
		d->state = RUNNING;
	}

	if ((d->state == RUNNING_WHEELSLIP) && (d->abs_duty_cycle > d->max_duty_with_margin)) {
		d->setpoint_target = 0;
	}
	if (d->is_upside_down && (d->state == RUNNING)) {
		d->state = RUNNING_UPSIDEDOWN;
		if (!d->is_upside_down_started) {
			// right after flipping when first engaging dark ride we add a 1 second grace period
			// before aggressively checking for board wiggle (based on acceleration)
			d->is_upside_down_started = true;
			d->delay_upside_down_fault = d->current_time;
		}
	}
}

static void calculate_setpoint_interpolated(data *d) {
	if (d->setpoint_target_interpolated != d->setpoint_target) {
		// If we are less than one step size away, go all the way
		if (fabsf(d->setpoint_target - d->setpoint_target_interpolated) < get_setpoint_adjustment_step_size(d)) {
			d->setpoint_target_interpolated = d->setpoint_target;
		} else if (d->setpoint_target - d->setpoint_target_interpolated > 0) {
			d->setpoint_target_interpolated += get_setpoint_adjustment_step_size(d);
		} else {
			d->setpoint_target_interpolated -= get_setpoint_adjustment_step_size(d);
		}
	}
}

static void apply_noseangling(data *d){
	if (d->state != RUNNING_WHEELSLIP) {
		// Nose angle adjustment, add variable then constant tiltback
		float noseangling_target = 0;
		if (fabsf(d->erpm) > d->tiltback_variable_max_erpm) {
			noseangling_target = fabsf(d->float_conf.tiltback_variable_max) * SIGN(d->erpm);
		} else {
			noseangling_target = d->tiltback_variable * d->erpm;
		}

		if (d->erpm > d->float_conf.tiltback_constant_erpm) {
			noseangling_target += d->float_conf.tiltback_constant;
		} else if (d->erpm < -d->float_conf.tiltback_constant_erpm){
			noseangling_target += -d->float_conf.tiltback_constant;
		}

		if (fabsf(noseangling_target - d->noseangling_interpolated) < d->noseangling_step_size) {
			d->noseangling_interpolated = noseangling_target;
		} else if (noseangling_target - d->noseangling_interpolated > 0) {
			d->noseangling_interpolated += d->noseangling_step_size;
		} else {
			d->noseangling_interpolated -= d->noseangling_step_size;
		}
	}

	d->setpoint += d->noseangling_interpolated;
}

static void apply_torquetilt(data *d) {
	float step_size;
	int torque_sign;
	float abs_torque;
	float torque_offset;
	float accel_factor2;
	float expected_acc;

	if (d->atr_disable) { // ATR Disabled; Revert to Stock Torque Tilt /* NOT PERMANENT BEHAVIOR */
		// Filter current (Biquad)
		if (d->float_conf.torquetilt_filter > 0) {
			d->torquetilt_filtered_current = biquad_process(&d->torquetilt_current_biquad, d->motor_current);
		} else {
			d->torquetilt_filtered_current = d->motor_current;
		}

		torque_sign = SIGN(d->torquetilt_filtered_current);

		if ((d->abs_erpm > 250) && (torque_sign != SIGN(d->erpm))) {
			// current is negative, so we are braking or going downhill
			// high currents downhill are less likely
			d->braking = true;
		}
		else {
			d->braking = false;
		}

		// Do stock FW torque tilt: (comment from Mitch Lustig)
		// Take abs motor current, subtract start offset, and take the max of that with 0 to get the current above our start threshold (absolute).
		// Then multiply it by "power" to get our desired angle, and min with the limit to respect boundaries.
		// Finally multiply it by sign motor current to get directionality back
		d->torquetilt_target = fminf(fmaxf((fabsf(d->torquetilt_filtered_current) - d->float_conf.torquetilt_start_current), 0) *
				d->float_conf.torquetilt_strength, d->float_conf.torquetilt_angle_limit) * SIGN(d->torquetilt_filtered_current);

		if ((d->torquetilt_interpolated - d->torquetilt_target > 0 && d->torquetilt_target > 0) ||
				(d->torquetilt_interpolated - d->torquetilt_target < 0 && d->torquetilt_target < 0)) {
			step_size = d->torquetilt_off_step_size;
		} else {
			step_size = d->torquetilt_on_step_size;
		}
	}
	else { // ATR Enabled
		// Filter current (Biquad)
		if (d->float_conf.atr_filter > 0) {
			d->torquetilt_filtered_current = biquad_process(&d->atr_current_biquad, d->motor_current);
		} else {
			d->torquetilt_filtered_current = d->motor_current;
		}

		torque_sign = SIGN(d->torquetilt_filtered_current);
		abs_torque = fabsf(d->torquetilt_filtered_current);
		torque_offset = d->float_conf.atr_torque_offset;
		accel_factor2 = d->tt_accel_factor * 1.3;

		if ((d->abs_erpm > 250) && (torque_sign != SIGN(d->erpm))) {
			// current is negative, so we are braking or going downhill
			// high currents downhill are less likely
			d->braking = true;
		}
		else {
			d->braking = false;
		}

		// Are we dealing with a free-spinning wheel?
		// If yes, don't change the torquetilt till we got traction again
		// instead slightly decrease it each cycle
		if (d->state == RUNNING_WHEELSLIP) {
			d->torquetilt_interpolated *= 0.995;
			d->torquetilt_target *= 0.99;
			d->braketilt_interpolated *= 0.995;
			d->braketilt_target *= 0.99;
			d->setpoint += d->torquetilt_interpolated + d->braketilt_interpolated;
			d->wheelslip_end_timer = d->current_time;
			return;
		}
		else {
			if ((d->current_time - d->wheelslip_end_timer) * 1000 < 100) {
				// for 100ms after wheelslip we still don't do ATR to allow the wheel to decelerate
				/*if (d->float_conf.yaw_current_clamp > 1) beep_alert(1, 0);*/ // TO BE RE-IMPLEMENTED
				d->torquetilt_interpolated *= 0.998;
				d->torquetilt_target *= 0.999;
				d->braketilt_interpolated *= 0.998;
				d->braketilt_target *= 0.999;
				d->setpoint += d->torquetilt_interpolated + d->braketilt_interpolated;
				return;
			}
			else if ((fabsf(d->acceleration) > 10) && (d->abs_erpm > 1000)) {
				/*if (float_conf.yaw_current_clamp > 1) beep_alert(1, 0);*/
				d->torquetilt_interpolated *= 0.998;
				d->torquetilt_target *= 0.999;
				d->braketilt_interpolated *= 0.998;
				d->braketilt_target *= 0.999;
				d->setpoint += d->torquetilt_interpolated + d->braketilt_interpolated;
				return;
			}
		}

		float atr_strength = d->float_conf.atr_strength;
		// from 3000 to 6000 erpm gradually crank up the torque response
		if ((d->abs_erpm > 3000) && (!d->braking)) {
			float speedboost = (d->abs_erpm - 3000) / 3000;
			speedboost = fminf(1, speedboost) * d->float_conf.atr_speed_boost;
			atr_strength += d->float_conf.atr_strength * speedboost;
		}

		// compare measured acceleration to expected acceleration
		float measured_acc = fmaxf(d->acceleration, -5);
		measured_acc = fminf(d->acceleration, 5);

		// expected acceleration is proportional to current (minus an offset, required to balance/maintain speed)
		//XXXXXfloat expected_acc;
		if (abs_torque < 25) {
			expected_acc = (d->torquetilt_filtered_current - SIGN(d->erpm) * torque_offset) / d->tt_accel_factor;
		}
		else {
			// primitive linear approximation of non-linear torque-accel relationship
			expected_acc = (torque_sign * 25 - SIGN(d->erpm) * torque_offset) / d->tt_accel_factor;
			expected_acc += torque_sign * (abs_torque - 25) / accel_factor2;
		}

		bool forward = (d->erpm > 0);
		if ((d->abs_erpm < 250) && (abs_torque > 30)) {
			forward = (expected_acc > 0);
		}

		float acc_diff = expected_acc - measured_acc;
		d->float_expected_acc = expected_acc;
		d->float_measured_acc = measured_acc;
		d->float_acc_diff = acc_diff;

		if (d->abs_erpm > 2000)
			d->accel_gap = 0.9 * d->accel_gap + 0.1 * acc_diff;
		else if (d->abs_erpm > 1000)
			d->accel_gap = 0.95 * d->accel_gap + 0.05 * acc_diff;
		else if (d->abs_erpm > 250)
			d->accel_gap = 0.98 * d->accel_gap + 0.02 * acc_diff;
		else {
			d->accel_gap = 0;
		}

		// now torquetilt target is purely based on gap between expected and actual acceleration
		float new_ttt = atr_strength * d->accel_gap;

		/* OG TT LOGIC
		if (!braking && (abs_erpm > 250))  {
			float og_tt_angle_limit = 3;
			float og_tt_start_current = 15;
			og_tt_target = (abs_torque - og_tt_start_current) * og_tt_strength;
			og_tt_target = fminf(fmaxf(og_tt_target, 0), og_tt_angle_limit);
			if (og_tt_target > fabsf(new_ttt)) {
				new_ttt = og_tt_target * torque_sign;
			}
		} else if (abs_erpm <= 250) {
			// TODO: fall back to og_tt!
		}
		*/

		// braking also should cause setpoint change lift, causing a delayed lingering nose lift
		if ((d->braketilt_factor < 0) && d->braking && (d->abs_erpm > 2000)) {
			// negative currents alone don't necessarily consitute active braking, look at proportional:
			if (SIGN(d->proportional) != SIGN(d->erpm)) {
				float downhill_damper = 1;
				// if we're braking on a downhill we don't want braking to lift the setpoint quite as much
				if (((d->erpm > 1000) && (d->accel_gap < -1)) ||
					((d->erpm < -1000) && (d->accel_gap > 1))) {
					downhill_damper += fabsf(d->accel_gap) / 2;
				}
				d->braketilt_target = d->proportional / d->braketilt_factor / downhill_damper;
				if (downhill_damper > 2) {
					// steep downhills, we don't enable this feature at all!
					d->braketilt_target = 0;
				}
			}
		}
		else {
			d->braketilt_target = 0;
		}

		d->torquetilt_target = d->torquetilt_target * 0.95 + 0.05 * new_ttt;
		d->torquetilt_target = fminf(d->torquetilt_target, d->float_conf.atr_angle_limit);
		d->torquetilt_target = fmaxf(d->torquetilt_target, -d->float_conf.atr_angle_limit);

		float response_boost = 1;
		if (d->abs_erpm > 2500) {
			response_boost = d->float_conf.atr_response_boost;
		}
		if (d->abs_erpm > 6000) {
			response_boost *= d->float_conf.atr_response_boost;
		}

		// Key to keeping the board level and consistent is to determine the appropriate step size!
		// We want to react quickly to changes, but we don't want to overreact to glitches in acceleration data
		// or trigger oscillations...
		const float TT_BOOST_MARGIN = 2;
		if (forward) {
			if (d->torquetilt_interpolated < 0) {
				// downhill
				if (d->torquetilt_interpolated < d->torquetilt_target) {
					// to avoid oscillations we go down slower than we go up
					step_size = d->atr_off_step_size;
					if ((d->torquetilt_target > 0)
						&& ((d->torquetilt_target - d->torquetilt_interpolated) > TT_BOOST_MARGIN)
						&& (d->abs_erpm > 2000))
					{
						// boost the speed if tilt target has reversed (and if there's a significant margin)
						step_size = d->atr_off_step_size * d->float_conf.atr_transition_boost;
					}
				}
				else {
					// torquetilt is increasing
					step_size = d->atr_on_step_size * response_boost;
				}
			}
			else {
				// uphill or other heavy resistance (grass, mud, etc)
				if ((d->torquetilt_target > -3) && (d->torquetilt_interpolated > d->torquetilt_target)) {
					// torquetilt winding down (current torquetilt is bigger than the target)
					// normal wind down case: to avoid oscillations we go down slower than we go up
					step_size = d->atr_off_step_size;
				}else{
					// standard case of increasing torquetilt
					step_size = d->atr_on_step_size * response_boost;
				}
			}
		}
		else {
			if (d->torquetilt_interpolated > 0) {
				// downhill
				if (d->torquetilt_interpolated > d->torquetilt_target) {
					// to avoid oscillations we go down slower than we go up
					step_size = d->atr_off_step_size;
					if ((d->torquetilt_target < 0)
						&& ((d->torquetilt_interpolated - d->torquetilt_target) > TT_BOOST_MARGIN)
						&& (d->abs_erpm > 2000)) {
						// boost the speed if tilt target has reversed (and if there's a significant margin)
						step_size = d->atr_off_step_size * d->float_conf.atr_transition_boost;
					}
				}
				else {
					// torquetilt is increasing
					step_size = d->atr_on_step_size * response_boost;
				}
			}
			else {
				// uphill or other heavy resistance (grass, mud, etc)
				if ((d->torquetilt_target < 3) && (d->torquetilt_interpolated < d->torquetilt_target)) {
					// normal wind down case: to avoid oscillations we go down slower than we go up
					step_size = d->atr_off_step_size;
				}else{
					// standard case of increasing torquetilt
					step_size = d->atr_on_step_size * response_boost;
				}
			}
		}
	}

	// when slow then erpm data is especially choppy, causing fake spikes in acceleration
	// mellow down the reaction to reduce noticeable oscillations
	if (d->abs_erpm < 500) {
		step_size /= 2;
	}

	if (fabsf(d->torquetilt_target - d->torquetilt_interpolated) < step_size) {
		d->torquetilt_interpolated = d->torquetilt_target;
	} else if (d->torquetilt_target - d->torquetilt_interpolated > 0) {
		d->torquetilt_interpolated += step_size;
	} else {
		d->torquetilt_interpolated -= step_size;
	}

	
	// Brake Tilt
	step_size = d->torquetilt_off_step_size / d->float_conf.braketilt_lingering;
	if(fabsf(d->braketilt_target) > fabsf(d->braketilt_interpolated)) {
		step_size = d->atr_on_step_size * 1.5;
	}
	else if (d->abs_erpm < 800) {
		step_size = d->atr_on_step_size;
	}

	if(fabsf(d->braketilt_target - d->braketilt_interpolated) < step_size){
		d->braketilt_interpolated = d->braketilt_target;
	}else if (d->braketilt_target - d->braketilt_interpolated > 0){
		d->braketilt_interpolated += step_size;
	}else{
		d->braketilt_interpolated -= step_size;
	}
	
	d->setpoint += d->torquetilt_interpolated + d->braketilt_interpolated;
}

static void apply_turntilt(data *d) {
	if (d->turntilt_strength == 0) {
		return;
	}

	float is_yaw_based = d->float_conf.turntilt_mode == YAW_BASED_TURNTILT;

	float abs_yaw_scaled = d->abs_yaw_change * 100;
	float turn_angle = is_yaw_based ? abs_yaw_scaled : d->abs_roll_angle; 

	// Apply cutzone
	if ((turn_angle < d->float_conf.turntilt_start_angle) || (d->state != RUNNING)) {
		d->turntilt_target = 0;
	}
	else {
		// Calculate desired angle
		float turn_change = is_yaw_based ? d->abs_yaw_change : d->abs_roll_angle_sin;
		d->turntilt_target = turn_change * d->turntilt_strength;

		// Apply speed scaling
		float boost;
		if (d->abs_erpm < d->float_conf.turntilt_erpm_boost_end) {
			boost = 1.0 + d->abs_erpm * d->turntilt_boost_per_erpm;
		} else {
			boost = 1.0 + (float)d->float_conf.turntilt_erpm_boost / 100.0;
		}
		d->turntilt_target *= boost;

		if (is_yaw_based) {
			// Increase turntilt based on aggregate yaw change (at most: double it)
			float aggregate_damper = 1.0;
			if (d->abs_erpm < 2000) {
				aggregate_damper = 0.5;
			}
			boost = 1 + aggregate_damper * fabsf(d->yaw_aggregate) / d->yaw_aggregate_target;
			boost = fminf(boost, 2);
			d->turntilt_target *= boost;
		}

		// Limit angle to max angle
		if (d->turntilt_target > 0) {
			d->turntilt_target = fminf(d->turntilt_target, d->float_conf.turntilt_angle_limit);
		} else {
			d->turntilt_target = fmaxf(d->turntilt_target, -d->float_conf.turntilt_angle_limit);
		}

		// Disable below erpm threshold otherwise add directionality
		if (d->abs_erpm < d->float_conf.turntilt_start_erpm) {
			d->turntilt_target = 0;
		} else {
			d->turntilt_target *= SIGN(d->erpm);
		}

		// ATR interference: Reduce turntilt_target during moments of high torque response
		float atr_min = 2;
		float atr_max = 5;
		if (SIGN(d->torquetilt_target) != SIGN(d->turntilt_target)) {
			// further reduced turntilt during moderate to steep downhills
			atr_min = 1;
			atr_max = 4;
		}
		if (fabsf(d->torquetilt_target) > atr_min) {
			// Start scaling turntilt when ATR>2, down to 0 turntilt for ATR > 5 degrees
			float atr_scaling = (atr_max - fabsf(d->torquetilt_target)) / (atr_max-atr_min);
			if (atr_scaling < 0) {
				atr_scaling = 0;
				// during heavy torque response clear the yaw aggregate too
				d->yaw_aggregate = 0;
			}
			d->turntilt_target *= atr_scaling;
		}
		if (fabsf(d->pitch_angle - d->noseangling_interpolated) > 4) {
			// no setpoint changes during heavy acceleration or braking
			d->turntilt_target = 0;
			d->yaw_aggregate = 0;
		}
	}

	// Move towards target limited by max speed
	if (fabsf(d->turntilt_target - d->turntilt_interpolated) < d->turntilt_step_size) {
		d->turntilt_interpolated = d->turntilt_target;
	} else if (d->turntilt_target - d->turntilt_interpolated > 0) {
		d->turntilt_interpolated += d->turntilt_step_size;
	} else {
		d->turntilt_interpolated -= d->turntilt_step_size;
	}

	d->setpoint += d->turntilt_interpolated;
}

static void brake(data *d) {
	// Brake timeout logic
	float brake_timeout_length = 1; // Brake Timeout hard-coded to 1s
	if ((d->abs_erpm > 1 || d->brake_timeout == 0)) {
		d->brake_timeout = d->current_time + brake_timeout_length;
	}

	if (d->brake_timeout != 0 && d->current_time > d->brake_timeout) {
		return;
	}

	// Reset the timeout
	VESC_IF->timeout_reset();

	// Set current
	VESC_IF->mc_set_brake_current(d->float_conf.brake_current);
}

static void set_current(data *d, float current){
	// Limit current output to configured max output
	if (current > 0 && current > VESC_IF->get_cfg_float(CFG_PARAM_l_current_max)) {
		current = VESC_IF->get_cfg_float(CFG_PARAM_l_current_max);
	} else if(current < 0 && current < VESC_IF->get_cfg_float(CFG_PARAM_l_current_min)) {
		current = VESC_IF->get_cfg_float(CFG_PARAM_l_current_min);
	}

	// Reset the timeout
	VESC_IF->timeout_reset();
	// Set the current delay
	VESC_IF->mc_set_current_off_delay(d->motor_timeout_seconds);
	// Set Current
	VESC_IF->mc_set_current(current);
}

static void imu_ref_callback(float *acc, float *gyro, float *mag, float dt) {
	data *d = (data*)ARG;
	VESC_IF->ahrs_update_mahony_imu(gyro, acc, dt, &d->m_att_ref);
}

static void float_thd(void *arg) {
	data *d = (data*)arg;

	while (!VESC_IF->should_terminate()) {
		// Update times
		d->current_time = VESC_IF->system_time();
		if (d->last_time == 0) {
			d->last_time = d->current_time;
		}

		d->diff_time = d->current_time - d->last_time;
		d->filtered_diff_time = 0.03 * d->diff_time + 0.97 * d->filtered_diff_time; // Purely a metric
		d->last_time = d->current_time;

		if (d->float_conf.loop_time_filter > 0) {
			d->loop_overshoot = d->diff_time - (d->loop_time_seconds - roundf(d->filtered_loop_overshoot));
			d->filtered_loop_overshoot = d->loop_overshoot_alpha * d->loop_overshoot + (1.0 - d->loop_overshoot_alpha) * d->filtered_loop_overshoot;
		}

		// Read values for GUI
		d->motor_current = VESC_IF->mc_get_tot_current_directional_filtered();
		d->motor_position = VESC_IF->mc_get_pid_pos_now();

		// Get the IMU Values
		d->roll_angle = RAD2DEG_f(VESC_IF->ahrs_get_roll(&d->m_att_ref));
		d->abs_roll_angle = fabsf(d->roll_angle);
		d->abs_roll_angle_sin = sinf(DEG2RAD_f(d->abs_roll_angle));

		// Darkride:
		if (d->allow_upside_down) {
			if (d->is_upside_down) {
				if (d->abs_roll_angle < 120) {
					d->is_upside_down = false;
				}
			} else if (d->enable_upside_down) {
				if (d->abs_roll_angle > 150) {
					d->is_upside_down = true;
					d->is_upside_down_started = false;
					d->pitch_angle = -d->pitch_angle;
				}
			}
		}

		d->last_pitch_angle = d->pitch_angle;

		// True pitch is derived from the secondary IMU filter running with kp=0.2
		d->true_pitch_angle = RAD2DEG_f(VESC_IF->ahrs_get_pitch(&d->m_att_ref));
		d->pitch_angle = RAD2DEG_f(VESC_IF->imu_get_pitch());
		if (d->is_upside_down) {
			d->pitch_angle = -d->pitch_angle;
			d->true_pitch_angle = -d->true_pitch_angle;
		}
		
		d->last_gyro_y = d->gyro[1];
		VESC_IF->imu_get_gyro(d->gyro);

		// Get the values we want
		d->duty_cycle = VESC_IF->mc_get_duty_cycle_now();
		d->abs_duty_cycle = fabsf(d->duty_cycle);
		d->erpm = VESC_IF->mc_get_rpm();
		d->abs_erpm = fabsf(d->erpm);
		d->adc1 = VESC_IF->io_read_analog(VESC_PIN_ADC1);
		d->adc2 = VESC_IF->io_read_analog(VESC_PIN_ADC2); // Returns -1.0 if the pin is missing on the hardware
		if (d->adc2 < 0.0) {
			d->adc2 = 0.0;
		}

		// Torque Tilt:

		/* FW SUPPORT NEEDED FOR SMOOTH ERPM //////////////////////////
		float smooth_erpm = erpm_sign * mcpwm_foc_get_smooth_erpm();
		float acceleration_raw = smooth_erpm - last_erpm;
		d->last_erpm = smooth_erpm;
		////////////////////////// FW SUPPORT NEEDED FOR SMOOTH ERPM */

		float acceleration_raw = d->erpm - d->last_erpm;
		d->last_erpm = d->erpm;

		d->accelavg += (acceleration_raw - d->accelhist[d->accelidx]) / ACCEL_ARRAY_SIZE;
		d->accelhist[d->accelidx] = acceleration_raw;
		d->accelidx++;
		if (d->accelidx == ACCEL_ARRAY_SIZE)
			d->accelidx = 0;

		d->acceleration = d->accelavg;
		
		// Turn Tilt:
		d->yaw_angle = VESC_IF->ahrs_get_yaw(&d->m_att_ref) * 180.0f / M_PI;
		float new_change = d->yaw_angle - d->last_yaw_angle;
		bool unchanged = false;
		if ((new_change == 0) // Exact 0's only happen when the IMU is not updating between loops
			|| (fabsf(new_change) > 100)) // yaw flips signs at 180, ignore those changes
		{
			new_change = d->last_yaw_change;
			unchanged = true;
		}
		d->last_yaw_change = new_change;
		d->last_yaw_angle = d->yaw_angle;

		// To avoid overreactions at low speed, limit change here:
		new_change = fminf(new_change, 0.10);
		new_change = fmaxf(new_change, -0.10);
		d->yaw_change = d->yaw_change * 0.8 + 0.2 * (new_change);
		// Clear the aggregate yaw whenever we change direction
		if (SIGN(d->yaw_change) != SIGN(d->yaw_aggregate))
			d->yaw_aggregate = 0;
		d->abs_yaw_change = fabsf(d->yaw_change);
		if ((d->abs_yaw_change > 0.04) && !unchanged)	// don't count tiny yaw changes towards aggregate
			d->yaw_aggregate += d->yaw_change;

		d->switch_state = check_adcs(d);

		// Log Values
		d->float_setpoint = d->setpoint;
		d->float_atr = d->torquetilt_interpolated;
		d->float_braketilt = d->braketilt_interpolated;

		float new_pid_value = 0;

		// Control Loop State Logic
		switch(d->state) {
		case (STARTUP):
				// Disable output
				brake(d);
				if (VESC_IF->imu_startup_done()) {
					reset_vars(d);
					d->state = FAULT_STARTUP; // Trigger a fault so we need to meet start conditions to start

					/*MORE DADO BUZZER LOGIC*/////////////////////////////////////////////////////////////////
					// // Let the rider know that the board is ready (one short beep)
					// beep_alert(1, false);
					// // Are we within 5V of the LV tiltback threshold? Issue 1 beep for each volt below that
					// float bat_volts = GET_INPUT_VOLTAGE();
					// float threshold = float_conf.tiltback_lv + 5;
					// if (bat_volts < threshold) {
					// 	int beeps = (int)fminf(6, threshold - bat_volts);
					// 	beep_alert(beeps, true);
					// }
					//////////////////////////////////////////////////////////////////////////////////////////

				}
				break;

		case (RUNNING):
		case (RUNNING_TILTBACK):
		case (RUNNING_WHEELSLIP):
		case (RUNNING_UPSIDEDOWN):
			// Check for faults
			if (check_faults(d, false)) {
				break;
			}

			d->enable_upside_down = true;
			d->disengage_timer = d->current_time;

			// Calculate setpoint and interpolation
			calculate_setpoint_target(d);
			calculate_setpoint_interpolated(d);
			d->setpoint = d->setpoint_target_interpolated;
			if (!d->is_upside_down) {
				apply_noseangling(d);
				apply_torquetilt(d);
				apply_turntilt(d);
			}

			// Do PID maths
			d->proportional = d->setpoint - d->pitch_angle;

			// Resume real PID maths
			d->integral = d->integral + d->proportional;

			if (!d->atr_disable) {
				// Produce controlled nose/tail lift with increased torque
				float tt_impact;
				if (d->torquetilt_interpolated < 0)
					// Downhill tail lift doesn't need to be as high as uphill nose lift
					tt_impact = d->float_conf.atr_downhill_tilt;
				else {
					tt_impact = d->float_conf.atr_uphill_tilt;

					const float max_impact_erpm = 2500;
					const float starting_impact = 0.5;
					if (d->abs_erpm < max_impact_erpm) {
						// Reduced nose lift at lower speeds
						// Creates a value between 0.5 and 1.0
						float erpm_scaling = fmaxf(starting_impact, d->abs_erpm / max_impact_erpm);
						tt_impact = (1.0 - (tt_impact * erpm_scaling));
					}
				}
				d->integral -= d->torquetilt_interpolated * tt_impact;
			}

			if (d->setpointAdjustmentType == REVERSESTOP) {
				d->integral = d->integral * 0.9;
			}

			// Apply I term Filter
			if (d->float_conf.ki_limit > 0 && fabsf(d->integral * d->float_conf.ki) > d->float_conf.ki_limit) {
				d->integral = d->float_conf.ki_limit / d->float_conf.ki * SIGN(d->integral);
			}

			new_pid_value = (d->float_conf.kp * d->proportional) + (d->float_conf.ki * d->integral);

			d->last_proportional = d->proportional;

			// Start Rate PID and Booster portion a few cycles later, after the start clicks have been emitted
			// this keeps the start smooth and predictable
			if (d->start_counter_clicks == 0) {
				// Rate PID (Angle + Rate, rather than Angle-Rate Cascading)
				d->proportional2 = -d->gyro[1];
				d->integral2 = d->integral2 + d->proportional2;

				// Apply I term Filter
				if (d->float_conf.ki_limit > 0 && fabsf(d->integral2 * d->float_conf.ki2) > d->float_conf.ki_limit) {
					d->integral2 = d->float_conf.ki_limit / d->float_conf.ki2 * SIGN(d->integral2);
				}

				new_pid_value += (d->float_conf.kp2 * d->proportional2) +
						(d->float_conf.ki2 * d->integral2);


				// Apply Booster
				d->abs_proportional = fabsf(d->proportional);
				float booster_current = d->float_conf.booster_current;

				// Make booster a bit stronger at higher speed (up to 2x stronger when braking)
				const float boost_min_erpm = 3000;
				if (d->abs_erpm > boost_min_erpm) {
					float speedstiffness = fminf(1, (d->abs_erpm - boost_min_erpm) / 10000);
					if (d->braking)
						booster_current += booster_current * speedstiffness;
					else
						booster_current += booster_current * speedstiffness / 2;
				}


				if (d->abs_proportional > d->float_conf.booster_angle) {
					if (d->abs_proportional - d->float_conf.booster_angle < d->float_conf.booster_ramp) {
						d->applied_booster_current = (d->float_conf.booster_current * SIGN(d->proportional)) *
								((d->abs_proportional - d->float_conf.booster_angle) / d->float_conf.booster_ramp);
					} else {
						d->applied_booster_current = d->float_conf.booster_current * SIGN(d->proportional);
					}
				}

				new_pid_value += d->applied_booster_current;
			}
			
			// Current Limiting!
			float current_limit;
			if (d->braking) {
				current_limit = d->mc_current_min * (1 + 0.6 * fabsf(d->torquetilt_interpolated / 10));
			}
			else {
				current_limit = d->mc_current_max * (1 + 0.6 * fabsf(d->torquetilt_interpolated / 10));
			}
			if (fabsf(new_pid_value) > current_limit) {
				new_pid_value = SIGN(new_pid_value) * current_limit;
			}
			else {
				// Over continuous current for more than 3 seconds? Just beep, don't actually limit currents
				if (fabsf(d->torquetilt_filtered_current) < d->max_continuous_current) {
					d->overcurrent_timer = d->current_time;
					if (d->current_beeping) {
						d->current_beeping = false;
						// beep_off(false);  /* REQUIRES BUZZER SUPPORT */
					}
				} else {
					if (d->current_time - d->overcurrent_timer > 3) {
						// beep_on(true);  /* REQUIRES BUZZER SUPPORT */
						d->current_beeping = true;
					}
				}
			}
			
			if (d->traction_control) {
				// freewheel while traction loss is detected
				d->pid_value = 0;
			}
			else {
				// Brake Amp Rate Limiting
				if (d->braking && (fabsf(d->pid_value - new_pid_value) > d->pid_brake_increment)) {
					if (new_pid_value > d->pid_value) {
						d->pid_value += d->pid_brake_increment;
					}
					else {
						d->pid_value -= d->pid_brake_increment;
					}
				}
				else {
					d->pid_value = d->pid_value * 0.8 + new_pid_value * 0.2;
				}
			}

			// Output to motor
			if (d->start_counter_clicks) {
				// Generate alternate pulses to produce distinct "click"
				d->start_counter_clicks--;
				if ((d->start_counter_clicks & 0x1) == 0)
					set_current(d, d->pid_value - d->start_click_current);
				else
					set_current(d, d->pid_value + d->start_click_current);
			}
			else {
				set_current(d, d->pid_value);
			}

			break;

		case (FAULT_ANGLE_PITCH):
		case (FAULT_ANGLE_ROLL):
		case (FAULT_REVERSE):
		case (FAULT_QUICKSTOP):
		case (FAULT_SWITCH_HALF):
		case (FAULT_SWITCH_FULL):
		case (FAULT_STARTUP):
			if (d->current_time - d->disengage_timer > 10) {
				// 10 seconds of grace period between flipping the board over and allowing darkride mode...
				if (d->is_upside_down) {
					// beep_alert(1, 1); /* REQUIRES BUZZER SUPPORT */
				}
				d->enable_upside_down = false;
				d->is_upside_down = false;
			}
			
			// Check for valid startup position and switch state
			if (fabsf(d->pitch_angle) < d->float_conf.startup_pitch_tolerance &&
				fabsf(d->roll_angle) < d->float_conf.startup_roll_tolerance && 
				d->switch_state == ON) {
				reset_vars(d);
				break;
			}
			// Ignore roll while it's upside down
			if(d->is_upside_down && (fabsf(d->pitch_angle) < d->float_conf.startup_pitch_tolerance)) {
				if ((d->state != FAULT_REVERSE) ||
					// after a reverse fault, wait at least 1 second before allowing to re-engage
					(d->current_time - d->disengage_timer) > 1) {
					reset_vars(d);
				}
				break;
			}
			// Disable output
			brake(d);
			break;
		}

	  /*update_beep_alert();*/

		// Debug outputs
//		app_balance_sample_debug();
//		app_balance_experiment();

		// Delay between loops
		VESC_IF->sleep_us((uint32_t)((d->loop_time_seconds - roundf(d->filtered_loop_overshoot)) * 1000000.0));
	}
}

static float app_float_get_debug(int index) {
	data *d = (data*)ARG;

	switch(index){
		case(1):
			return d->setpoint;
		case(2):
			return d->float_setpoint;
		case(3):
			return d->torquetilt_filtered_current;
		case(4):
			return d->float_atr;
		case(5):
			return d->last_pitch_angle - d->pitch_angle;
		case(6):
			return d->motor_current;
		case(7):
			return d->erpm;
		case(8):
			return d->abs_erpm;
		case(9):
			return d->loop_time_seconds;
		case(10):
			return d->diff_time;
		case(11):
			return d->loop_overshoot;
		case(12):
			return d->filtered_loop_overshoot;
		case(13):
			return d->filtered_diff_time;
		case(14):
			return d->integral;
		case(15):
			return d->integral * d->float_conf.ki;
		case(16):
			return d->integral2;
		case(17):
			return d->integral2 * d->float_conf.ki2;
		default:
			return 0;
	}
}

static void send_realtime_data(data *d){
	int32_t ind = 0;
	uint8_t send_buffer[50];
//	send_buffer[ind++] = COMM_GET_DECODED_BALANCE;
	buffer_append_float32_auto(send_buffer, d->pid_value, &ind);
	buffer_append_float32_auto(send_buffer, d->pitch_angle, &ind);
	buffer_append_float32_auto(send_buffer, d->roll_angle, &ind);
	buffer_append_float32_auto(send_buffer, d->diff_time, &ind);
	buffer_append_float32_auto(send_buffer, d->motor_current, &ind);
	// buffer_append_float32_auto(send_buffer, app_float_get_debug(d->debug_render_1), &ind);
	buffer_append_uint16(send_buffer, d->state, &ind);
	buffer_append_uint16(send_buffer, d->switch_state, &ind);
	buffer_append_float32_auto(send_buffer, d->adc1, &ind);
	buffer_append_float32_auto(send_buffer, d->adc2, &ind);
	// buffer_append_float32_auto(send_buffer, app_float_get_debug(d->debug_render_2), &ind);
	
	// -DEBUG-
	buffer_append_float32_auto(send_buffer, d->true_pitch_angle, &ind);
	buffer_append_float32_auto(send_buffer, d->float_setpoint, &ind);
	buffer_append_float32_auto(send_buffer, d->float_atr, &ind);
	buffer_append_float32_auto(send_buffer, d->float_braketilt, &ind);
	buffer_append_float32_auto(send_buffer, d->erpm, &ind);
	buffer_append_float32_auto(send_buffer, d->torquetilt_filtered_current, &ind);
	buffer_append_float32_auto(send_buffer, d->braking, &ind);
	buffer_append_float32_auto(send_buffer, d->float_expected_acc, &ind);
	buffer_append_float32_auto(send_buffer, d->float_measured_acc, &ind);
	buffer_append_float32_auto(send_buffer, d->float_acc_diff, &ind);

	VESC_IF->send_app_data(send_buffer, ind);
}

// Handler for incoming app commands
static void on_command_recieved(unsigned char *buffer, unsigned int len) {
	data *d = (data*)ARG;

	if(len > 0){
		uint8_t command = buffer[0];
		if(command == 0x01){
			send_realtime_data(d);
		}else{
			VESC_IF->printf("Unknown command received %d", command);
		}
	}
}

// Register get_debug as a lisp extension
static lbm_value ext_bal_dbg(lbm_value *args, lbm_uint argn) {
	if (argn != 1 || !VESC_IF->lbm_is_number(args[0])) {
		return VESC_IF->lbm_enc_sym_eerror;
	}

	return VESC_IF->lbm_enc_float(app_float_get_debug(VESC_IF->lbm_dec_as_i32(args[0])));
}

// These functions are used to send the config page to VESC Tool
// and to make persistent read and write work
static int get_cfg(uint8_t *buffer, bool is_default) {
	data *d = (data*)ARG;
	float_config *cfg = VESC_IF->malloc(sizeof(float_config));

	*cfg = d->float_conf;

	if (is_default) {
		confparser_set_defaults_float_config(cfg);
	}

	int res = confparser_serialize_float_config(buffer, cfg);
	VESC_IF->free(cfg);

	return res;
}

static bool set_cfg(uint8_t *buffer) {
	data *d = (data*)ARG;
	bool res = confparser_deserialize_float_config(buffer, &(d->float_conf));

	// Store to EEPROM
	if (res) {
		uint32_t ints = sizeof(float_config) / 4 + 1;
		uint32_t *buffer = VESC_IF->malloc(ints * sizeof(uint32_t));
		bool write_ok = true;
		memcpy(buffer, &(d->float_conf), sizeof(float_config));
		for (uint32_t i = 0;i < ints;i++) {
			eeprom_var v;
			v.as_u32 = buffer[i];
			if (!VESC_IF->store_eeprom_var(&v, i + 1)) {
				write_ok = false;
				break;
			}
		}

		VESC_IF->free(buffer);

		if (write_ok) {
			eeprom_var v;
			v.as_u32 = FLOAT_CONFIG_SIGNATURE;
			VESC_IF->store_eeprom_var(&v, 0);
		}

		configure(d);
	}

	return res;
}

static int get_cfg_xml(uint8_t **buffer) {
	// Note: As the address of data_float_config_ is not known
	// at compile time it will be relative to where it is in the
	// linked binary. Therefore we add PROG_ADDR to it so that it
	// points to where it ends up on the STM32.
	*buffer = data_float_config_ + PROG_ADDR;
	return DATA_FLOAT_CONFIG__SIZE;
}

// Called when code is stopped
static void stop(void *arg) {
	data *d = (data*)arg;
	VESC_IF->set_app_data_handler(NULL);
	VESC_IF->conf_custom_clear_configs();
	VESC_IF->request_terminate(d->thread);
	VESC_IF->printf("Float App Terminated");
	VESC_IF->free(d);
}

INIT_FUN(lib_info *info) {
	INIT_START

	data *d = VESC_IF->malloc(sizeof(data));
	memset(d, 0, sizeof(data));
	
	if (!d) {
		VESC_IF->printf("Out of memory!");
		return false;
	}

	// Read config from EEPROM if signature is correct
	eeprom_var v;
	uint32_t ints = sizeof(float_config) / 4 + 1;
	uint32_t *buffer = VESC_IF->malloc(ints * sizeof(uint32_t));
	bool read_ok = VESC_IF->read_eeprom_var(&v, 0);
	if (read_ok && v.as_u32 == FLOAT_CONFIG_SIGNATURE) {
		for (uint32_t i = 0;i < ints;i++) {
			if (!VESC_IF->read_eeprom_var(&v, i + 1)) {
				read_ok = false;
				break;
			}
			buffer[i] = v.as_u32;
		}
	} else {
		read_ok = false;
	}
	
	if (read_ok) {
		memcpy(&(d->float_conf), buffer, sizeof(float_config));
	} else {
		confparser_set_defaults_float_config(&(d->float_conf));
	}
	
	VESC_IF->free(buffer);

	info->stop_fun = stop;	
	info->arg = d;
	
	VESC_IF->conf_custom_add_config(get_cfg, set_cfg, get_cfg_xml);

	configure(d);

	VESC_IF->ahrs_init_attitude_info(&d->m_att_ref);
	d->m_att_ref.acc_confidence_decay = 0.1;
	d->m_att_ref.kp = 0.2;

	VESC_IF->imu_set_read_callback(imu_ref_callback);

	d->thread = VESC_IF->spawn(float_thd, 2048, "Float Main", d);

	VESC_IF->set_app_data_handler(on_command_recieved);
	VESC_IF->lbm_add_extension("ext-float-dbg", ext_bal_dbg);

	return true;
}

