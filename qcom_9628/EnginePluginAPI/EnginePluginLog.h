/******************************************************************************
    Copyright (c) 2007-2011, 2014, 2016, 2018 Qualcomm Technologies, Inc.
    All Rights Reserved.
    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************/

#ifndef ENGINE_PLUGIN_LOG_H
#define ENGINE_PLUGIN_LOG_H

#include <stdint.h>
#include <time.h>

/** Major File Version */
#define EP_LOG_INTERFACE_FILE_MAJOR_VERSION  1
/** Minor File Version */
#define EP_LOG_INTERFACE_FILE_MINOR_VERSION  1

/* Engine Msg callback */
#define EP_1C54_DIAG_VERSION 3
/* Measurement msg Diag version*/
#define EP_1C5E_DIAG_VERSION 3
/* Report status response Error info */
#define EP_1CBC_DIAG_VERSION 1

#ifdef __linux__
#define PACKED
#define PACKED_POST __attribute__((__packed__))
#endif
#define EP_DIAG_ID_LENGTH   128

#if defined(__linux__) || defined(USE_GLIB) || defined(__ANDROID__)
#define TYPEDEF_PACKED_STRUCT typedef PACKED struct PACKED_POST
#else
#define TYPEDEF_PACKED_STRUCT typedef struct
#endif

#ifdef _WIN32
#pragma pack(push, 1)
#endif
#ifdef _WIN32
#define int8    int8_t
#define uint8   uint8_t
#define uint32  uint32_t
#define int32   int32_t
#define uint16  uint16_t
#define int16   int16_t
#define uint64  uint64_t
#define int64   int64_t
#define boolean uint8_t
#define byte    uint8_t

typedef double    DOUBLE;
typedef float     FLOAT;
typedef long long INT64;
typedef unsigned long long UINT64;
typedef int INT32;
typedef unsigned int UINT32;
typedef short int INT16;
typedef unsigned short int UINT16;
typedef unsigned char UINT8;
#endif

#if !defined(USE_GLIB) && !defined(__ANDROID__)
#ifndef __LOG_HDR_TYPE__
#define __LOG_HDR_TYPE__
typedef struct PACKED_POST {
    uint16_t len;         /* Specifies the length, in bytes of the
                     entry, including this header. */

    uint16_t code;            /* Specifies the log code for the entry as
                     enumerated above. Note: This is
                     specified as word to guarantee size. */
                     /*upper 48 bits represent elapsed time since
                     6 Jan 1980 00:00:00 in 1.25 ms units. The
                     low order 16 bits represent elapsed time
                     since the last 1.25 ms tick in 1/32 chip
                     units (this 16 bit counter wraps at the
                     value 49152). */
    uint32_t ts_lo; /* Time stamp */
    uint32_t ts_hi;
  } log_hdr_type;
#endif
#endif


/** @brief EP API Diag logger header for all messages. */
TYPEDEF_PACKED_STRUCT {
    /** Used by Logging Module */
    log_hdr_type z_LogHeader;
    /** EP Message Version */
    uint8_t u_Version;
    /** Engine Plugin identifier */
    char epID[EP_DIAG_ID_LENGTH];
    /** Source Nname */
    char source[EP_DIAG_ID_LENGTH];
    /** Process identification */
    uint32_t u_Process_id;
    /** time at logging in milliseconds */
    uint64_t t_TimeTickMsec;

    uint8_t eventType;
    uint32_t reserved2;
} epDiagLogGenericHeader;

/** Type of Log Level */
typedef uint32_t epDiagLogLevelType;
/** Log Level as Error */
#define EP_DIAG_LOG_LEVEL_ERROR    ((epDiagLogLevelType)0x00000001)
/** Log Level as Info */
#define EP_DIAG_LOG_LEVEL_INFO     ((epDiagLogLevelType)0x00000002)
/** Log Level as Warning */
#define EP_DIAG_LOG_LEVEL_WARNING  ((epDiagLogLevelType)0x00000003)
/** Log Level as Debug */
#define EP_DIAG_LOG_LEVEL_DEBUG    ((epDiagLogLevelType)0x00000004)
/** Log Level as Verbose */
#define EP_DIAG_LOG_LEVEL_VERBOSE  ((epDiagLogLevelType)0x00000005)

/** Type of Engine log */
typedef uint32_t epDiagLogType;
/** Log of debugging */
#define EP_DIAG_LOG_TYPE_DEBUG            ((epDiagLogType)0x00000001)
/** Log of Engine playback data */
#define EP_DIAG_LOG_TYPE_PLAYBACK         ((epDiagLogType)0x00000002)
/** Log of nmea sentence */
#define EP_DIAG_LOG_TYPE_NMEA             ((epDiagLogType)0x00000003)
/** Log of rtcm service status */
#define EP_DIAG_LOG_TYPE_RTCM_STATUS      ((epDiagLogType)0x00000004)

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    uint32_t moduleID;
    epDiagLogLevelType logLevel;
    epDiagLogType logType;
    uint32_t logLength;
    uint8_t logMessage[1];
} epDiagLogEngineMsg;

/** GNSS Signal Type and RF Band */
typedef uint32_t epDiagGnssSignalTypeMask;
/*** Unknow Signal */
#define EP_DIAG_GNSS_SIGNAL_UNKNOWN      ((epDiagGnssSignalTypeMask)0x00000000)
/** GPS L1CA Signal */
#define EP_DIAG_GNSS_SIGNAL_GPS_L1CA     ((epDiagGnssSignalTypeMask)0x00000001)
/** GPS L1C Signal */
#define EP_DIAG_GNSS_SIGNAL_GPS_L1C      ((epDiagGnssSignalTypeMask)0x00000002)
/** GPS L2 RF Band */
#define EP_DIAG_GNSS_SIGNAL_GPS_L2C_L       ((epDiagGnssSignalTypeMask)0x00000004)
/** GPS L5 RF Band */
#define EP_DIAG_GNSS_SIGNAL_GPS_L5_Q       ((epDiagGnssSignalTypeMask)0x00000008)
/** GLONASS G1 (L1OF) RF Band */
#define EP_DIAG_GNSS_SIGNAL_GLONASS_G1_CA   ((epDiagGnssSignalTypeMask)0x00000010)
/** GLONASS G2 (L2OF) RF Band */
#define EP_DIAG_GNSS_SIGNAL_GLONASS_G2_CA   ((epDiagGnssSignalTypeMask)0x00000020)
/** GALILEO E1 RF Band */
#define EP_DIAG_GNSS_SIGNAL_GALILEO_E1_C   ((epDiagGnssSignalTypeMask)0x00000040)
/** GALILEO E5A RF Band */
#define EP_DIAG_GNSS_SIGNAL_GALILEO_E5A_Q  ((epDiagGnssSignalTypeMask)0x00000080)
/** GALILEO E5B RF Band */
#define EP_DIAG_GNSS_SIGNAL_GALILIEO_E5B_Q ((epDiagGnssSignalTypeMask)0x00000100)
/** BEIDOU B1 RF Band */
#define EP_DIAG_GNSS_SIGNAL_BEIDOU_B1_I    ((epDiagGnssSignalTypeMask)0x00000200)
/** BEIDOU B1_C RF Band */
#define EP_DIAG_GNSS_SIGNAL_BEIDOU_B1_C    ((epDiagGnssSignalTypeMask)0x00000400)
/** BEIDOU B2 RF Band */
#define EP_DIAG_GNSS_SIGNAL_BEIDOU_B2_I    ((epDiagGnssSignalTypeMask)0x00000800)
/** BEIDOU B2A_I RF Band */
#define EP_DIAG_GNSS_SIGNAL_BEIDOU_B2A_I   ((epDiagGnssSignalTypeMask)0x00001000)
/** QZSS L1CA RF Band */
#define EP_DIAG_GNSS_SIGNAL_QZSS_L1CA      ((epDiagGnssSignalTypeMask)0x00002000)
/** QZSS L1S RF Band */
#define EP_DIAG_GNSS_SIGNAL_QZSS_L1S      ((epDiagGnssSignalTypeMask)0x00004000)
/** QZSS L2 RF Band */
#define EP_DIAG_GNSS_SIGNAL_QZSS_L2C_L      ((epDiagGnssSignalTypeMask)0x00008000)
/** QZSS L5 RF Band */
#define EP_DIAG_GNSS_SIGNAL_QZSS_L5_Q      ((epDiagGnssSignalTypeMask)0x00010000)
/** SBAS L1 RF Band */
#define EP_DIAG_GNSS_SIGNAL_SBAS_L1_CA      ((epDiagGnssSignalTypeMask)0x00020000)


/** GNSS Constellation type */
typedef uint16_t epDiagGnssConstellationTypeMask;
/** Unknow Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_UNKNOWN ((epGnssConstellationTypeMask) 0x0000)
/** GPS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_GPS ((epDiagGnssConstellationTypeMask) 0x0001)
/** Galileo Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_GALILEO ((epDiagGnssConstellationTypeMask) 0x0002)
/** SBAS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_SBAS ((epDiagGnssConstellationTypeMask) 0x0004)
/** COMPASS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_COMPASS ((epDiagGnssConstellationTypeMask) 0x0008)
/** GLONASS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_GLONASS ((epDiagGnssConstellationTypeMask) 0x0010)
/** BEIDOU Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_BEIDOU ((epDiagGnssConstellationTypeMask) 0x0020)
/** QZSS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_QZSS ((epDiagGnssConstellationTypeMask) 0x0040)

/** Max number of GNSS SV measurement */
//#define EP_DIAG_GNSS_MAX_MEAS                  176
#define EP_DIAG_GNSS_MAX_MEAS                  64


/** GNSS Measurement Status Mask */
typedef uint64_t epDiagGNSSMeasurementStatusMask;
/** Satellite time in sub-millisecond (code phase) is known */
#define EP_DIAG_MEAS_STATUS_SM_VALID              ((epDiagGNSSMeasurementStatusMask)0x00000001)
/** Satellite sub-bit time is known */
#define EP_DIAG_MEAS_STATUS_SB_VALID              ((epDiagGNSSMeasurementStatusMask)0x00000002)
/** Satellite time in milliseconds is known */
#define EP_DIAG_MEAS_STATUS_MS_VALID              ((epDiagGNSSMeasurementStatusMask)0x00000004)
/** Signal bit edge is confirmed  */
#define EP_DIAG_MEAS_STATUS_BE_CONFIRM            ((epDiagGNSSMeasurementStatusMask)0x00000008)
/** Satellite Doppler is measured */
#define EP_DIAG_MEAS_STATUS_VELOCITY_VALID        ((epDiagGNSSMeasurementStatusMask)0x00000010)
/** Status as Reserved */
#define EP_DIAG_MEAS_STATUS_RESERVED              ((epDiagGNSSMeasurementStatusMask)0x00000020)
/** TRUE/FALSE : Lock Point is valid/invalid.*/
#define EP_DIAG_MEAS_STATUS_LP_VALID              ((epDiagGNSSMeasurementStatusMask)0x00000040)
/** TRUE/FALSE : Lock Point is positive/negative*/
#define EP_DIAG_MEAS_STATUS_LP_POS_VALID          ((epDiagGNSSMeasurementStatusMask)0x00000080)
/** Range update from satellite differences is measured */
#define EP_DIAG_MEAS_STATUS_FROM_RNG_DIFF         ((epDiagGNSSMeasurementStatusMask)0x00000200)
/** Doppler update from satellite differences is measured */
#define EP_DIAG_MEAS_STATUS_FROM_VE_DIFF          ((epDiagGNSSMeasurementStatusMask)0x00000400)
/** Fresh GNSS measurement observed in last second  */
#define EP_DIAG_MEAS_STATUS_GNSS_FRESH_MEAS_VALID ((epDiagGNSSMeasurementStatusMask)0x08000000)

/** GNSS SV search status */
typedef uint8_t epDiagGnssMeasSvSearchState;
/** SV is not being actively searched */
#define EP_DIAG_GNSS_SV_SRCH_STATE_IDLE (1)
/** The system is searching for this SV */
#define EP_DIAG_GNSS_SV_SRCH_STATE_SEARCH (2)
/** SV is being tracked */
#define EP_DIAG_GNSS_SV_SRCH_STATE_TRACK (3)

/** GNSS SV Info Mask */
typedef uint8_t epDiagGnssMeasSvInfoMask;
/** Ephemeris is available for this SV */
#define EP_DIAG_GNSS_SVINFO_MASK_HAS_EPHEMERIS ((epDiagGnssMeasSvInfoMask)0x01)
/** Almanac is available for this SV */
#define EP_DIAG_GNSS_SVINFO_MASK_HAS_ALMANAC ((epDiagGnssMeasSvInfoMask)0x02)

/** Flags to indicate valid fields in epDiagGnssSystemTimeStructType */
TYPEDEF_PACKED_STRUCT {
    uint16_t isSystemWeekValid : 1;
    uint16_t isSystemWeekMsecValid : 1;
    uint16_t isSystemClkTimeBiasValid : 1;
    uint16_t isSystemClkTimeUncMsValid : 1;
    uint16_t isRefFCountValid : 1;
    uint16_t isNumClockResetsValid : 1;
    uint16_t : 10;
}epDiagSystemTimeValidity;

/** Flags to indicate valid fields in epGnssLocGloTimeStructType */
typedef uint32_t epDiagGloTimeValidityM;
TYPEDEF_PACKED_STRUCT {
    uint16_t isGloClkTimeBiasValid : 1;
    uint16_t isGloClkTimeUncMsValid : 1;
    uint16_t isRefFCountValid : 1;
    uint16_t isNumClockResetsValid : 1;
    uint16_t : 12;
}epDiagGloTimeValidity;

/** Flags to indicate valid fields in epDiagGnssSVMeasurementStructType */
TYPEDEF_PACKED_STRUCT {
    uint32_t isHealthStatusValid : 1;
    uint32_t isSvInfoMaskValid : 1;
    uint32_t isLossOfLockValid : 1;
    uint32_t isFineSpeedValid : 1;
    uint32_t isFineSpeedUncValid : 1;
    uint32_t isCarrierPhaseValid : 1;
    uint32_t isCarrierPhaseUncValid : 1;
    uint32_t isCycleSlipCountValid : 1;
    uint32_t isPseudorangeValid : 1;
    uint32_t isPseudorangeUncValid : 1;
    uint32_t : 22;
} gnssDiagSVMeasurementValidity;

/** Flags to indicate SV LLI */
TYPEDEF_PACKED_STRUCT {
    uint8_t lossOfContinousLock : 1;
    uint8_t halfCycleAmbiguity : 1;
    uint8_t : 6;
} epDiagGnssMeasSvLLI;


/** Flags to indicate valid fields in epDiagGnssMeasurementReport */
TYPEDEF_PACKED_STRUCT {
    uint16_t isLeapSecValid : 1;
    uint16_t isClockDriftValid : 1;
    uint16_t isClockDriftStdDeviationValid : 1;
    uint16_t isGpsSystemTimeValid : 1;
    uint16_t isGalSystemTimeValid : 1;
    uint16_t isBdsSystemTimeValid : 1;
    uint16_t isQzssSystemTimeValid : 1;
    uint16_t isGloSystemTimeValid : 1;
    uint16_t : 8;
} epDiagGnssMeasurementReportValidity;

typedef struct {
    uint32_t tv_sec;
    uint32_t tv_nsec;
} timeSpec;

/** Indicates AP time stamp  */
TYPEDEF_PACKED_STRUCT {
    /** Boot time received from pps-ktimer
        Mandatory Field */
    timeSpec apTimeStamp;
    /** Time-stamp uncertainty in Milliseconds
        Mandatory Field */
    float apTimeStampUncertaintyMs;
} epDiagGnssApTimeStampStructType;

/** GNSS System Time */
TYPEDEF_PACKED_STRUCT {
    /** Validity mask for below fields */
    epDiagSystemTimeValidity validityMask;
    /** Extended week number at reference tick.
    Unit: Week.
    Set to 65535 if week number is unknown.
    For GPS:
      Calculated from midnight, Jan. 6, 1980.
      OTA decoded 10 bit GPS week is extended to map between:
      [NV6264 to (NV6264 + 1023)].
      NV6264: Minimum GPS week number configuration.
      Default value of NV6264: 1738
    For BDS:
      Calculated from 00:00:00 on January 1, 2006 of Coordinated Universal Time (UTC).
    For GAL:
      Calculated from 00:00 UT on Sunday August 22, 1999 (midnight between August 21 and August 22).
   */
    uint16_t systemWeek;
    /** Time in to the current week at reference tick.
       Unit: Millisecond. Range: 0 to 604799999.
       Check for systemClkTimeUncMs before use */
    uint32_t systemMsec;
    /** System clock time bias (sub-millisecond)
        Units: Millisecond
        Note: System time (TOW Millisecond) = systemMsec - systemClkTimeBias.
        Check for systemClkTimeUncMs before use. */
    float systemClkTimeBias;
    /** Single sided maximum time bias uncertainty
        Units: Millisecond */
    float systemClkTimeUncMs;
    /** FCount (free running HW timer) value. Don't use for relative time purpose
         due to possible discontinuities.
         Unit: Millisecond */
    uint32_t refFCount;
    /** Number of clock resets/discontinuities detected, affecting the local hardware counter value. */
    uint32_t numClockResets;
} epDiagGnssSystemTimeStructType;


/** GNSS GLONASS Time */
TYPEDEF_PACKED_STRUCT {
    /** GLONASS day number in four years. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 65535
        Mandatory Field */
    uint16_t gloDays;
    /** GLONASS time of day in Millisecond. Refer to GLONASS ICD.
        Units: Millisecond
        Check for gloClkTimeUncMs before use */
    /** Validity mask for below fields */
    epDiagGloTimeValidity validityMask;
    uint32_t gloMsec;
    /** GLONASS clock time bias (sub-millisecond)
        Units: Millisecond
        Note: GLO time (TOD Millisecond) = gloMsec - gloClkTimeBias.
        Check for gloClkTimeUncMs before use. */
    float gloClkTimeBias;
    /** Single sided maximum time bias uncertainty
        Units: Millisecond */
    float gloClkTimeUncMs;
    /** FCount (free running HW timer) value. Don't use for relative time purpose
        due to possible discontinuities.
        Unit: Millisecond */
    uint32_t  refFCount;
    /** Number of clock resets/discontinuities detected, affecting the local hardware counter value. */
    uint32_t numClockResets;
    /** GLONASS four year number from 1996. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 255
        Mandatory Field */
    uint8_t gloFourYear;
} epDiagGnssGloTimeStructType;

/** GNSS SV Measurement Info */
TYPEDEF_PACKED_STRUCT {
    /** Specifies GNSS signal type. Mandatory field */
    epDiagGnssSignalTypeMask gnssSignal;
    /** Specifies GNSS system. Mandatory field */
    epDiagGnssConstellationTypeMask gnssSystem;
    /** Specifies GNSS SV ID.
             For GPS:             1 to 32
             For GLONASS:     65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
             For SBAS:           120 to 151
             For QZSS-L1CA:  193 to 197
             For BDS:             201 to 237
             For GAL:              301 to 336
             Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 7.
         Valid only for a GLONASS system and
         is to be ignored for all other systems.
             - Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Specifies satellite search state. Mandatory field */
    epDiagGnssMeasSvSearchState svSearchState;
    /**  Carrier to Noise ratio
         - Units: 0.1 dBHz
         Mandatory field */
    float cN0dbHz;
    /**  Bit-mask indicating SV measurement status.
         Valid bit-masks:
         If any MSB bit in 0xFFC0000000000000 DONT_USE is set, the measurement
         must not be used by the client.
         Mandatory field */
    epDiagGNSSMeasurementStatusMask  measurementStatus;
    /**  Satellite time Millisecond.
         When measurementStatus bit-0 (sub-ms valid),
         1 (sub-bit known) & 2 (SV time known) are set, for:
         GPS, BDS, GAL, QZSS: Range of 0 through (604800000-1).
         GLO: Range of 0 through (86400000-1).
         Unit:Millisecond
         Note: All SV times in the current measurement block are already
               propagated to common reference time epoch.
         Mandatory field */
    uint32_t svMs;
    /**  Satellite time sub-millisecond.
         When measurementStatus bit-0 "Sub-millisecond is valid" is set,
         Range of 0 through 0.99999 [Milliseconds]
         Total SV Time = svMs + svSubMs
         - Units: Millisecond
        Mandatory field */
    float svSubMs;
    /**  Satellite Time uncertainty
         - Units: Millisecond
         Mandatory field */
    float svTimeUncMs;
    /** Satellite Doppler
        - Units: meter per sec
        Mandatory field */
    float dopplerShift;
    /** Satellite Doppler uncertainty
        - Units: meter per sec */
    float dopplerShiftUnc;
    /** Validity mask for below fields */
    gnssDiagSVMeasurementValidity validityMask;
    /** Health status.
        Range: 0 to 2;
        0 = unhealthy, 1 = healthy, 2 = unknown */
    uint8_t healthStatus;
    /** Indicates whether almanac and ephemeris information is available. */
    epDiagGnssMeasSvInfoMask svInfoMask;
    /** Loss of signal lock indicator */
    epDiagGnssMeasSvLLI lossOfLock;
    /** Increments when a CSlip is detected */
    uint8_t cycleSlipCount;
    /** Carrier phase derived speed
        Units: m/s */
    /** Estimated pseudorange uncertainty
        Unit: Meters */
    float pseudorange_uncertinty;
    /** Carrier phase measurement [Carrier cycles]  */
    double carrierPhase;
    /** Carrier phase measurement Unc [Carrier cycles]  */
    double carrierPhaseUnc;
    /** Estimated pseudorange
        Unit: Meters */
    double pseudorange;
} epDiagGnssSVMeasurementStructType;

/** GNSS Measurement Report */
TYPEDEF_PACKED_STRUCT {
    /** Receiver clock Drift
        Units: meter per sec */
    float clockDrift;
    /** Receiver clock drift std deviation
        Units: Meter per sec */
    float clockDriftStdDeviation;
    /** GPS system time information. Check validity before using */
    epDiagGnssSystemTimeStructType gpsSystemTime;
    /** GAL system time information. Check validity before using */
    epDiagGnssSystemTimeStructType galSystemTime;
    /** BDS system time information. Check validity before using */
    epDiagGnssSystemTimeStructType bdsSystemTime;
    /** QZSS system time information. Check validity before using */
    epDiagGnssSystemTimeStructType qzssSystemTime;
    /** GLO system time information. Check validity before using */
    epDiagGnssGloTimeStructType gloSystemTime;
    /** Validity mask */
    epDiagGnssMeasurementReportValidity validityMask;
    /** GPS time leap second delta to UTC time. Check validity before using
        Units: sec */
    uint8_t leapSec;
    /** Number of Measurements in this report block.
        Mandatory field */
    uint8_t numMeas;
    /** Satellite measurement Information
        Mandatory field */
    epDiagGnssSVMeasurementStructType svMeasurement[EP_DIAG_GNSS_MAX_MEAS];
    /** Local time of last PPS pulse, associated with GNSS Measurement report
        Mandatory field */
    epDiagGnssApTimeStampStructType lastPpsLocaltimeStamp;
} epDiagGnssMeasurementReportStruct;

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    uint8_t segmentNo;
    uint8_t maxSegmentNo;
    epDiagGnssMeasurementReportStruct msrInfo;
} epDiagGnssMeasurementReport;

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    uint32_t status;
} epDiagReportStatusRespInfo;

#ifdef _WIN32
#pragma pack(pop)
#endif
#endif /*ENGINE_PLUGIN_LOG_H */
