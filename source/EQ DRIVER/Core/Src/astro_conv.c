//
// Created by paulo on 22/09/2022.
//

#include "astro_conv.h"

void update_LST(mount_data_t* EQ_mount) {

    uint8_t Month;
    uint8_t Minutes;
    uint8_t Seconds;
    uint16_t Year;

    double A;
    double B;
    double C;
    double E;
    double F;
    double Hours;
    double Day;
    double current_JDN;
    double prev_midnight_JDN;
    double current_DUT;
    double current_longitude;
    double LMST;

    Hours = (EQ_mount->GNSS_data.nmea_utc / 10000);
    Minutes = ((EQ_mount->GNSS_data.nmea_utc / 100) - (Hours * 100));
    Seconds = (EQ_mount->GNSS_data.nmea_utc - (Minutes * 100) - (Hours * 10000));

    Day = (EQ_mount->GNSS_data.nmea_date / 10000);
    Month = ((EQ_mount->GNSS_data.nmea_date / 100) - (Day * 100));
    Year = (EQ_mount->GNSS_data.nmea_date - (Month * 100) - (Day * 10000)) + 2000;

    /** longitude filtering in degrees */
    current_longitude = (uint8_t)(EQ_mount->GNSS_data.nmea_longitude / 100);
    current_longitude += (EQ_mount->GNSS_data.nmea_longitude - current_longitude * 100) / 60;
    current_longitude *= (EQ_mount->GNSS_data.longitude_side == 'W' ? -1.0 : 1.0); /**< signed longitude value */

    /** julian calendar correction */
    if (Month < 3) {
        Month += 12;
        Year -= 1;
    }

    /**
     * Julian Day Number calculation (no decimals)
     * A = Y/100
     * B = A/4
     * C = 2-A+B
     * E = 365.25x(Y+4716)
     * F = 30.6001x(M+1)
     * JD= C+D+E+F-1524.5
     */
    A = (int16_t)(Year / 100);
    B = (int16_t)(A / 4);
    C = (int16_t)(2 - A + B);
    E = (int) (365.25 * (Year + 4716));
    F = (int16_t)(30.6001 * (Month + 1));

    //exact amount of day
    Day += (Hours / EARTH_ROTATION_HOURS) + (Minutes / EARTH_ROTATION_MINS) + (Seconds / EARTH_ROTATION_SECS);

    current_JDN = C + Day + E + F - 1524.5;
    prev_midnight_JDN = ((int) current_JDN) - 0.5;

    current_DUT = (prev_midnight_JDN - 2451545.0);

    /** Greenwich Mean sideral Time */
    double GMST = fmod((18.697375 + (24.065709824279 * current_DUT)), 24);
    GMST = fmod(GMST + 24, 24); /**< Ensure positive modulus */

    /** Local Mean Sideral Time */
    LMST = fmod(GMST + ((current_longitude) / 15), 24);
    LMST = fmod(LMST+ 24, 24); /**< Ensure positive modulus */

    EQ_mount->LMST = LMST;
}

void astro_start_tracking(mount_data_t* EQ_mount) {
    stepper_init(&EQ_mount->axis_stepper.RA);
    stepper_init(&EQ_mount->axis_stepper.DEC);

    EQ_mount->axis_stepper.RA.timer_config.TIMx->ARR = EQ_mount->axis_stepper.RA.auto_step_ticks;
    EQ_mount->axis_stepper.RA.timer_config.TIMx->CCR1 = EQ_mount->axis_stepper.RA.auto_step_ticks / 2;
    HAL_TIM_PWM_Start_IT(EQ_mount->axis_stepper.RA.timer_config.htimx, TIM_CHANNEL_1);
}
