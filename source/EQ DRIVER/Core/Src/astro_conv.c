//
// Created by paulo on 22/09/2022.
//

#include "astro_conv.h"

void update_LST(mount_data_t *EQ_mount) {

    uint16_t M, Y, MN, S;
    double A, B, C, D, E, F, H, current_JDN, prev_midnight_JDN, current_DUT, current_longitude, LST;

    H = (EQ_mount->GNSS_data.nmea_utc / 10000);
    MN = ((EQ_mount->GNSS_data.nmea_utc / 100) - (H * 100));
    S = (EQ_mount->GNSS_data.nmea_utc - (MN * 100) - (H * 10000));

    D = (EQ_mount->GNSS_data.nmea_date / 10000);
    M = ((EQ_mount->GNSS_data.nmea_date / 100) - (D * 100));
    Y = (EQ_mount->GNSS_data.nmea_date - (M * 100) - (D * 10000)) + 2000;

    current_longitude = (uint8_t) (EQ_mount->GNSS_data.nmea_longitude / 100);
    current_longitude += (EQ_mount->GNSS_data.nmea_longitude - current_longitude * 100) / 60; //longitude in degrees
    current_longitude *= (EQ_mount->GNSS_data.longitude_side == 'W' ? -1.0 : 1.0); /** true longitude value */

    if (M < 3) {
        M += 12;
        Y -= 1;
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
    A = (int16_t) (Y / 100);
    B = (int16_t) (A / 4);
    C = (int16_t) (2 - A + B);
    D += (H / EARTH_ROTATION_HOURS) + (MN / EARTH_ROTATION_MINS) + (S / EARTH_ROTATION_SECS); //exact amount of day
    E = (int) (365.25 * (Y + 4716));
    F = (int16_t) (30.6001 * (M + 1));
    current_JDN = C + D + E + F - 1524.5;
    prev_midnight_JDN = ((int) current_JDN) - 0.5;

    current_DUT = (prev_midnight_JDN - 2451545.0);
    LST = fmod((18.697375 + (24.065709824279 * current_DUT)), 24);

    int LSTint;
    LSTint = (int) LST;
    LSTint /= 24;
    LST += ((current_longitude) / 15);
    LST = LSTint + (LST - LSTint * 24);

    EQ_mount->LMST = LST;
}
