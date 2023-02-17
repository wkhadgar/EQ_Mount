//
// Created by paulo on 22/09/2022
//

#include "astro_conv.h"
#include "tim.h"

#define STEPPER_PERIOD_FINE_ADJUST_SCALER 10

#define UTC_OFFSET (-3)

/**
 * @brief Módulo de abstração do motor de passo.
 */
typedef struct {
    stepper_t stp;
    const motor_axis_t axis;
} stepper_axis_t;

/**
 * @brief Estrutura da montagem equatorial.
 */
struct mount_data {

    int8_t raw_fine_adjusts;
    GNSS_data_t GNSS_data;

    struct {
        int16_t RA_fine_period_adjust;
        time__t LST_time;
        time__t civilian_time;
        astro_pos_t orientation;
        bool is_initialized;
    } time_reference;

    struct {
        astro_target_t current_target;
        reachability_t reachability;
        movement_t current_movement;
    } target_info;

    struct {
        stepper_axis_t RA;
        stepper_axis_t DEC;
    } axises;

} self = {
        .raw_fine_adjusts = 0,
        .GNSS_data = {0},
        .time_reference = {
                .is_initialized = false,
        },
        .target_info = {
                .reachability = UNKNOWN_REACH,
                .current_movement = STOPPED,
        },
        .axises = {
                .RA = {
                        .axis = Right_Ascension,
                        .stp = {
                                .info = {
                                        .position = STEPPER_MAX_STEPS * (WORM_GEAR_RATIO /
                                                                         2), /**< setting 0 as the unreachable lowest orientation */
                                        .target_position = STEPPER_MAX_STEPS * (WORM_GEAR_RATIO /
                                                                                2), /**< setting 0 as the unreachable lowest orientation */
                                        .on_status = false,
                                        .is_configured = false,
                                },
                                .timer_config = {
                                        .TIM = TIM3,
                                        .htim = &htim3,
                                        .TIM_CHANNEL = TIM_CHANNEL_1
                                },
                                .step_pin = {
                                        .GPIO = M1_STEP_GPIO_Port,
                                        .port = M1_STEP_Pin,
                                },
                                .dir_pin = {
                                        .GPIO = M1_DIR_GPIO_Port,
                                        .port = M1_DIR_Pin,
                                },
                                .enable_pin = {
                                        .GPIO = M1_ENABLE_GPIO_Port,
                                        .port = M1_ENABLE_Pin,
                                },
                        },
                },
                .DEC = {
                        .axis = Declination,
                        .stp = {
                                .info = {
                                        .position = 0,
                                        .target_position = 0,
                                        .on_status = false,
                                        .is_configured = false,
                                },
                                .timer_config = {
                                        .TIM = TIM2,
                                        .htim = &htim2,
                                        .TIM_CHANNEL = TIM_CHANNEL_4,
                                },
                                .step_pin = {
                                        .GPIO = M2_STEP_GPIO_Port,
                                        .port = M2_STEP_Pin,
                                },
                                .dir_pin = {
                                        .GPIO = M2_DIR_GPIO_Port,
                                        .port = M2_DIR_Pin,
                                },
                                .enable_pin = {
                                        .GPIO = M2_ENABLE_GPIO_Port,
                                        .port = M2_ENABLE_Pin,
                                },
                        },
                },
        },
};

static inline double signed_cyclic_distance(double origin, double target, uint32_t cycle_size) {
    double cw_d = origin > target ? (origin - target) : (target - origin);
    double ccw_d = cycle_size - cw_d;

    return cw_d < ccw_d ? cw_d : -ccw_d;
}

static inline uint32_t
cyclic_modulus(int32_t
               value,
               uint32_t modulus
) {

    value = value % modulus;

    if (value < 0) {
        value +=
                modulus;
    }

    return
            value;
}

static inline double cyclic_float_modulus(double num, uint32_t mod) {

    while (num > mod) {
        num -= mod;
    }

    if (num < 0) {
        num += mod; /**< Ensure circular modulus */
    }

    return num;
}

static void get_as_angle_type(angle_t* angle_var, double decimal_degrees) {

    decimal_degrees = cyclic_float_modulus(decimal_degrees, 360);

    angle_var->decimal_degrees = decimal_degrees;
    angle_var->degrees = (uint16_t) decimal_degrees;
    angle_var->arc_minutes = (uint8_t)((decimal_degrees - angle_var->degrees) * 60);
    angle_var->arc_seconds = (uint8_t)(((double) decimal_degrees - angle_var->degrees) -
                                       ((double) angle_var->arc_minutes / MINUTES_PER_HOUR)) *
                             SECONDS_PER_HOUR;
}

static void get_as_ptime_type(time__t* time_var, double decimal_hours) {

    decimal_hours = cyclic_float_modulus(decimal_hours, 24);

    time_var->decimal_hours = decimal_hours;
    time_var->hours = (uint8_t) decimal_hours;
    time_var->minutes = (uint8_t)((decimal_hours - time_var->hours) * 60);
    time_var->seconds =
            (uint8_t)(((double) decimal_hours - time_var->hours) - ((double) time_var->minutes / MINUTES_PER_HOUR)) *
            SECONDS_PER_HOUR;
}

void astro_init(void) {

    stepper_init(&self.axises.RA.stp);
    stepper_init(&self.axises.DEC.stp);

    self.axises.RA.stp.timer_config.pwm_period = TRACKING_SPEED_PULSE_PERIOD_duS;
}

void astro_update_raw_fine_adjusts(void) {

    uint8_t raw_reading;

    raw_reading = fine_adjusts_prescaler_value();

    self.raw_fine_adjusts = (int8_t)(raw_reading - 50); //creates scale from -50 to 50
}

void astro_update_LMST(void) {

/** LST calculation variables */
    uint16_t Year;
    uint8_t Month;
    uint8_t Seconds;
    uint8_t Minutes;
    double Day;
    double Hours;
    double current_longitude;
    double A;
    double B;
    double C;
    double E;
    double F;
    double current_JDN;
    double GMST_raw;
    double T;
    double J0;
    double D0;
    double H;

    if (self.GNSS_data.is_valid == VALID_DATA) {

        Seconds = self.GNSS_data.nmea_utc % 100;

        Hours = (uint8_t)(self.GNSS_data.nmea_utc / 10000);
        Minutes = (uint8_t)((self.GNSS_data.nmea_utc / 100) - (Hours * 100));
        Day = (uint8_t)(self.GNSS_data.nmea_date / 10000);
        Month = (uint8_t)((self.GNSS_data.nmea_date / 100) - (Day * 100));
        Year = (self.GNSS_data.nmea_date % 100) + 2000;
        if (Month < 3) { /** julian calendar correction */
            Month += 12;
            Year -= 1;
        }

        A = (int16_t)(Year / 100);
        B = (int16_t)(A / 4);
        C = (int16_t)(2 - A + B);
        E = (int) (365.25 * (Year + 4716));
        F = (int16_t)(30.6001 * (Month + 1));

        get_as_ptime_type(&self.time_reference.civilian_time,
                          (Hours + ((double) Minutes / 60) + ((double) Seconds / 3600)) + UTC_OFFSET);

        /** longitude filtering in degrees */
        current_longitude = (uint8_t)(self.GNSS_data.nmea_longitude / 100);
        current_longitude += (self.GNSS_data.nmea_longitude - current_longitude * 100) / 60.0;
        current_longitude *= (self.GNSS_data.longitude_side == 'W' ? -1.0 : 1.0); /**< signed longitude value */

        /** Decimal Day */
        Day += (Hours / EARTH_ROTATION_HOURS) + (Minutes / EARTH_ROTATION_MINS) + (Seconds / EARTH_ROTATION_SECS);

        current_JDN = C + Day + E + F - 1524.5;  /**< Julian Day right now */
        J0 = (((uint32_t)
                current_JDN) + 0.5); /**< Julian midnight */
        H = (current_JDN - J0) * EARTH_ROTATION_HOURS; /**< Hours since midnight */
        D0 = (J0 - 2451545.0); /**< Whole days since epoch */
        T = ((current_JDN - 2451545.0) / 36525.0); /** Centuries since epoch */

        /** Greenwich Mean Sideral Time */
        GMST_raw = 6.697374558 + (0.06570982441908 * D0) + (1.00273790935 * H) + (0.000026 * (T * T));

        /** Local Mean Sideral Time */
        get_as_ptime_type(&self.time_reference.LST_time, (GMST_raw + (current_longitude / 15.0)));

        self.time_reference.is_initialized = true; /** save future computation time */
    }
}

bool astro_is_at_target(void) {
    if (self.axises.RA.stp.info.position == self.axises.RA.stp.info.target_position) {
        return true;
    }

    return false;
}


static void astro_set_home(void) {

    self.axises.RA.stp.info.target_position = STEPPER_FULL_RANGE / 2;
    self.axises.DEC.stp.info.target_position = 0;

    if (self.GNSS_data.is_valid == VALID_DATA) {
        self.target_info.reachability = REACHABLE;

        self.target_info.current_target.position.declination.degrees = 0;
        self.target_info.current_target.position.declination.arc_minutes = 0;

        self.target_info.current_target.position.right_ascension.hours = self.time_reference.LST_time.hours;
        self.target_info.current_target.position.right_ascension.minutes = self.time_reference.LST_time.minutes;
    }
}


static reachability_t check_reachability(time__t target_RA) {

    bool is_set;
    bool is_risen;
    uint8_t rise_begin;
    uint8_t set_end;

    set_end = cyclic_modulus(self.time_reference.LST_time.hours + 6, EARTH_ROTATION_HOURS);
    rise_begin = cyclic_modulus(self.time_reference.LST_time.hours - 6, EARTH_ROTATION_HOURS);

    is_risen = (target_RA.hours >= rise_begin) && (target_RA.hours <= self.time_reference.LST_time.hours);
    is_set = (target_RA.hours >= set_end) && (target_RA.hours > self.time_reference.LST_time.hours);


    if (!is_risen) {
        return UNREACHABLE_UNRISEN;
    } else if (is_set) {
        return UNREACHABLE_SET;
    } else {
        return REACHABLE;
    }
}

static uint16_t moving_to_target_smoothen_period_update(stepper_t* stp) {

    return stepper_to_target_smoothen_period_update((int32_t)
                                                            stp->info.position - (int32_t)
                                                            stp->info.target_position);
}

static void update_stp_period(stepper_t* stp) {
    stp->timer_config.pwm_period = (self.target_info.current_movement == GOING_TO)
                                   ? moving_to_target_smoothen_period_update(stp)
                                   : TRACKING_SPEED_PULSE_PERIOD_duS;

    stp->timer_config.TIM->ARR = self.axises.RA.stp.timer_config.pwm_period * 2;
    stp->timer_config.TIM->CCR1 = self.axises.RA.stp.timer_config.pwm_period;
}

void astro_update_target(astro_target_t target) {

    int32_t target_steps_offset;
    double target_time_offset;

    self.target_info.current_target = target;

    if (self.GNSS_data.is_valid == VALID_DATA) {
        self.target_info.reachability = check_reachability(target.position.right_ascension);

        if (self.target_info.reachability == REACHABLE) {
            target_time_offset = signed_cyclic_distance(self.time_reference.LST_time.decimal_hours,
                                                        self.target_info.current_target.position.right_ascension.decimal_hours,
                                                        EARTH_ROTATION_HOURS);

            target_steps_offset = STEPPER_FULL_RANGE / target_time_offset;

            self.axises.RA.stp.info.target_position = (STEPPER_FULL_RANGE / 2) + target_steps_offset;
        }

        return;
    }

    self.target_info.reachability = UNKNOWN_REACH;
}

GNSS_data_t* astro_get_gnss_pointer(void) {

    return &self.GNSS_data;
}

static void astro_start_moving(void) {

    HAL_TIM_PWM_Start_IT(self.axises.RA.stp.timer_config.htim, self.axises.RA.stp.timer_config.TIM_CHANNEL);
    HAL_TIM_PWM_Start_IT(self.axises.DEC.stp.timer_config.htim, self.axises.RA.stp.timer_config.TIM_CHANNEL);
}

void astro_full_stop(void) {

    HAL_TIM_PWM_Stop_IT(self.axises.DEC.stp.timer_config.htim, self.axises.DEC.stp.timer_config.TIM_CHANNEL);
    HAL_TIM_PWM_Stop_IT(self.axises.RA.stp.timer_config.htim, self.axises.RA.stp.timer_config.TIM_CHANNEL);
}

void astro_release(void) {
    stepper_disable(&self.axises.DEC.stp);
    stepper_disable(&self.axises.RA.stp);
    self.axises.DEC.stp.info.on_status = false;
    self.axises.RA.stp.info.on_status = false;
}

void astro_engage(void) {
    stepper_enable(&self.axises.DEC.stp);
    stepper_enable(&self.axises.RA.stp);
    self.axises.DEC.stp.info.on_status = true;
    self.axises.RA.stp.info.on_status = true;
}

void astro_axis_add_fine_adjusts(void) {

    astro_update_raw_fine_adjusts();

    self.time_reference.RA_fine_period_adjust = (int16_t)(self.raw_fine_adjusts * STEPPER_PERIOD_FINE_ADJUST_SCALER);
}

void astro_start_tracking(void) {

    stepper_set_direction(&self.axises.RA.stp, clockwise);

    self.target_info.current_movement = TRACKING;

    astro_engage();
    astro_start_moving();
}

void astro_goto_target(void) {

    if (self.target_info.reachability == REACHABLE) {

        if (self.axises.RA.stp.info.target_position < self.axises.RA.stp.info.position) {
            stepper_set_direction(&self.axises.RA.stp, counter_clockwise);
            self.axises.RA.stp.info.direction = counter_clockwise;
        } else {
            stepper_set_direction(&self.axises.RA.stp, clockwise);
            self.axises.RA.stp.info.direction = clockwise;
        }

        if (self.axises.DEC.stp.info.target_position < self.axises.DEC.stp.info.position) {
            stepper_set_direction(&self.axises.DEC.stp, counter_clockwise);
            self.axises.DEC.stp.info.direction = counter_clockwise;
        } else {
            stepper_set_direction(&self.axises.DEC.stp, clockwise);
            self.axises.DEC.stp.info.direction = clockwise;
        }

        self.target_info.current_movement = GOING_TO;

        astro_engage();
        astro_start_moving();
    }
}

void astro_go_home(void) {
    astro_set_home();
    astro_goto_target();
}


void astro_RA_position_step_isr(void) {

    if (!self.axises.RA.stp.info.is_configured || !self.axises.RA.stp.info.on_status) {
        return;
    }

    self.axises.RA.stp.info.position = cyclic_modulus(
            ((int32_t)(self.axises.RA.stp.info.position)) + self.axises.RA.stp.info.direction,
            STEPPER_MAX_STEPS * 96);

    update_stp_period(&self.axises.RA.stp);

    if (astro_is_at_target()) {
        self.target_info.current_movement = TRACKING;
    }
}

void astro_DEC_position_step_isr(void) {
    if (!self.axises.DEC.stp.info.is_configured || !self.axises.DEC.stp.info.on_status) {
        return;
    }

    self.axises.DEC.stp.info.position = cyclic_modulus(
            ((int32_t)(self.axises.DEC.stp.info.position)) + self.axises.DEC.stp.info.direction,
            STEPPER_MAX_STEPS * 96);

    if (astro_is_at_target()) {
        self.target_info.current_movement = STOPPED;
        HAL_TIM_PWM_Stop_IT(self.axises.DEC.stp.timer_config.htim, self.axises.DEC.stp.timer_config.TIM_CHANNEL);
    } else {
        update_stp_period(&self.axises.DEC.stp);
    }
}