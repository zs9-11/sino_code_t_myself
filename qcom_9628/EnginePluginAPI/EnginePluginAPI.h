/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

EnginePluginAPI.h

GENERAL DESCRIPTION
This is inference file for Engine Plug-in API

=============================================================================*/


#ifndef ENGINE_PLUGIN_API_H
#define ENGINE_PLUGIN_API_H

#include <stdint.h>
#include <time.h>

/**
    Engine Plug-in Interface file version is defined using major and minor version.
    A Major version could be associated with non-backward compatible update.
    A change to only Minor version is generally backward compatible to header
    file having same Major version number.

    The backward compatibility of APIs is achieved by have fixed size reserved array
    at end of each structure. The fixed size reserved array makes sure that size of
    structure is remains constant across the multiple version of APIs header files.
    Whenever new parameter is added in structure then reserved array size must be
    reduce by the size of new parameter.

    Clients shall verify version number for compatibility.
*/
/** Major File Version */
#define EP_INTERFACE_FILE_MAJOR_VERSION  3
/** Minor File Version */
#define EP_INTERFACE_FILE_MINOR_VERSION  3

/** Type of Log Level */
typedef uint32_t epLogLevelType;
/** Log Level as Error */
#define EP_LOG_LEVEL_ERROR    ((epLogLevelType)0x00000001)
/** Log Level as Info */
#define EP_LOG_LEVEL_INFO     ((epLogLevelType)0x00000002)
/** Log Level as Warning */
#define EP_LOG_LEVEL_WARNING  ((epLogLevelType)0x00000003)
/** Log Level as Debug */
#define EP_LOG_LEVEL_DEBUG    ((epLogLevelType)0x00000004)
/** Log Level as Verbose */
#define EP_LOG_LEVEL_VERBOSE  ((epLogLevelType)0x00000005)

/** Type of Engine log */
typedef uint32_t epLogType;
/** Log of debugging */
#define EP_LOG_TYPE_DEBUG            ((epLogType)0x00000001)
/** Log of Engine playback data */
#define EP_LOG_TYPE_PLAYBACK         ((epLogType)0x00000002)
/** Log of nmea sentence */
#define EP_LOG_TYPE_NMEA             ((epLogType)0x00000003)
/** Log of rtcm service status */
#define EP_LOG_TYPE_RTCM_STATUS      ((epLogType)0x00000004)

/** Type of aiding data */
typedef uint32_t epAidingdataMask;
/** All Aiding Data including Vendor */
#define EP_ALL_AIDING_DATA       ((epAidingdataMask)0x00000001)


/** Connection Type */
typedef uint32_t epConnectionStatus;

/** No Data Connection */
#define EP_NO_CONNECTION      ((epConnectionStatus)0x00000000)
/** Data Connection using Wifi */
#define EP_WIFI_CONNECTION    ((epConnectionStatus)0x00000001)
/** Data Connection using Mobile */
#define EP_MOBILE_CONNECTION  ((epConnectionStatus)0x00000002)
/** Data Connection using other than Wifi and Mobile */
#define EP_OTHER_CONNECTION   ((epConnectionStatus)0x00000004)

/** Engine Plugin Error */
typedef uint32_t epError;
/** Indicates Engine Plug-in interface file version mismatch. */
#define EP_ENGINE_PLUGIN_INTERFACE_FILE_VERSION_MISMATCH ((epError)0x00000001)
/** Engine Plug-in is not supporting Measurement. */
#define EP_SUBSCRIPTION_MEASUREMENT_REPORT_NOT_SUPPORTED ((epError)0x00000002)
/** Engine Plug-in is not Polynomial. */
#define EP_SUBSCRIPTION_POLYNOMIAL_REPORT_NOT_SUPPORTED  ((epError)0x00000004)
/** Engine Plug-in is not Ephemeris. */
#define EP_SUBSCRIPTION_EPHEMERIS_REPORT_NOT_SUPPORTED   ((epError)0x00000008)


/**  Engine Plug-in Report subscription Mask
     Clients shall use the mask bits to subscribe to specific reports
     of interest
*/
typedef uint64_t epReportSubscriptionMask;
/** Subscribe to 1Hz PVT report. Only the PVT reports aligned to integer
   second will be reported */
#define EP_SUBSCRIPTION_PVT_1HZ_REPORT ((epReportSubscriptionMask)0x00000001)
/** Subscribe to N-Hz PVT report. All PVT reports will be reported */
#define EP_SUBSCRIPTION_PVT_NHZ_REPORT ((epReportSubscriptionMask)0x00000002)
/** Subscribe to 1Hz measurement report. */
#define EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT ((epReportSubscriptionMask)0x00000004)
/** Subscribe to N-Hz measurement report. */
#define EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT ((epReportSubscriptionMask)0x00000008)
/** Subscribe to polynomial report. */
#define EP_SUBSCRIPTION_POLYNOMIAL_REPORT  ((epReportSubscriptionMask)0x00000010)
/** Subscribe to ephemeris report. */
#define EP_SUBSCRIPTION_EPHEMERIS_REPORT   ((epReportSubscriptionMask)0x00000020)
/** Subscribe Unpropagated 1Hz PVT report. */
#define EP_SUBSCRIPTION_UNPROPAGATED_PVT_1HZ_REPORT \
               ((epReportSubscriptionMask)0x00000040)
/** Subscribe Unpropagated N-Hz PVT report. */
#define EP_SUBSCRIPTION_UNPROPAGATED_PVT_NHZ_REPORT \
               ((epReportSubscriptionMask)0x00000080)

/** Subscribe all report. */
#define EP_SUBSCRIPTION_ALL_REPORT (EP_SUBSCRIPTION_PVT_1HZ_REPORT|\
      EP_SUBSCRIPTION_PVT_NHZ_REPORT| EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT|\
      EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT|EP_SUBSCRIPTION_POLYNOMIAL_REPORT|\
      EP_SUBSCRIPTION_EPHEMERIS_REPORT)

/** GNSS Signal Type and RF Band */
typedef uint32_t epGnssSignalTypeMask;
/*** Unknow Signal */
#define EP_GNSS_SIGNAL_UNKNOWN        ((epGnssSignalTypeMask)0x00000000)
/** GPS L1 CA-Code Signal */
#define EP_GNSS_SIGNAL_GPS_L1CA       ((epGnssSignalTypeMask)0x00000001)
/** GPS L1C RF Band */
#define EP_GNSS_SIGNAL_GPS_L1C        ((epGnssSignalTypeMask)0x00000002)
/** GPS L2 RF Band */
#define EP_GNSS_SIGNAL_GPS_L2C_L      ((epGnssSignalTypeMask)0x00000004)
/** GPS L5 Q-Code RF Band */
#define EP_GNSS_SIGNAL_GPS_L5_Q       ((epGnssSignalTypeMask)0x00000008)
/** GLONASS G1 (L1OF) RF Band */
#define EP_GNSS_SIGNAL_GLONASS_G1_CA  ((epGnssSignalTypeMask)0x00000010)
/** GLONASS G2 (L2OF) RF Band */
#define EP_GNSS_SIGNAL_GLONASS_G2_CA  ((epGnssSignalTypeMask)0x00000020)
/** GALILEO E1 MBOC Pilot RF Band */
#define EP_GNSS_SIGNAL_GALILEO_E1_C   ((epGnssSignalTypeMask)0x00000040)
/** GALILEO E5A BPSK Pilot RF Band */
#define EP_GNSS_SIGNAL_GALILEO_E5A_Q  ((epGnssSignalTypeMask)0x00000080)
/** GALILEO E5B RF Band */
#define EP_GNSS_SIGNAL_GALILIEO_E5B_Q ((epGnssSignalTypeMask)0x00000100)
/** BEIDOU B1_I RF Band */
#define EP_GNSS_SIGNAL_BEIDOU_B1_I    ((epGnssSignalTypeMask)0x00000200)
/** BEIDOU B1_C RF Band */
#define EP_GNSS_SIGNAL_BEIDOU_B1_C    ((epGnssSignalTypeMask)0x00000400)
/** BEIDOU B2_I RF Band */
#define EP_GNSS_SIGNAL_BEIDOU_B2_I    ((epGnssSignalTypeMask)0x00000800)
/** BEIDOU B2A_I RF Band */
#define EP_GNSS_SIGNAL_BEIDOU_B2A_I   ((epGnssSignalTypeMask)0x00001000)
/** QZSS L1CA RF Band */
#define EP_GNSS_SIGNAL_QZSS_L1CA      ((epGnssSignalTypeMask)0x00002000)
/** QZSS L1S RF Band */
#define EP_GNSS_SIGNAL_QZSS_L1S       ((epGnssSignalTypeMask)0x00004000)
/** QZSS L2 RF Band */
#define EP_GNSS_SIGNAL_QZSS_L2C_L     ((epGnssSignalTypeMask)0x00008000)
/** QZSS L5 RF Band */
#define EP_GNSS_SIGNAL_QZSS_L5_Q      ((epGnssSignalTypeMask)0x00010000)
/** SBAS L1 RF Band */
#define EP_GNSS_SIGNAL_SBAS_L1_CA     ((epGnssSignalTypeMask)0x00020000)

/** GNSS Constellation type */
typedef uint16_t epGnssConstellationTypeMask;
/** Unknow Constellation */
#define EP_GNSS_CONSTELLATION_UNKNOWN ((epGnssConstellationTypeMask) 0x0000)
/** GPS Constellation */
#define EP_GNSS_CONSTELLATION_GPS ((epGnssConstellationTypeMask) 0x0001)
/** Galileo Constellation */
#define EP_GNSS_CONSTELLATION_GALILEO ((epGnssConstellationTypeMask) 0x0002)
/** SBAS Constellation */
#define EP_GNSS_CONSTELLATION_SBAS ((epGnssConstellationTypeMask) 0x0004)
/** COMPASS Constellation */
#define EP_GNSS_CONSTELLATION_COMPASS ((epGnssConstellationTypeMask) 0x0008)
/** GLONASS Constellation */
#define EP_GNSS_CONSTELLATION_GLONASS ((epGnssConstellationTypeMask) 0x0010)
/** BEIDOU Constellation */
#define EP_GNSS_CONSTELLATION_BEIDOU ((epGnssConstellationTypeMask) 0x0020)
/** QZSS Constellation */
#define EP_GNSS_CONSTELLATION_QZSS ((epGnssConstellationTypeMask) 0x0040)

/** Max number of GNSS SV measurement */
#ifdef MULTIBAND_SUPPORTED
#define EP_GNSS_MAX_MEAS                  176
#else
#define EP_GNSS_MAX_MEAS                  64
#endif

/** Set applicable bits */
typedef uint32_t epPositionFlags;
/** Position Report uses GNSS signal */
#define EP_POSITION_FLAG_GNSS_USED      ((epPositionFlags)0x00000001)
/** Position Report uses WIFI */
#define EP_POSITION_FLAG_WIFI_USED      ((epPositionFlags)0x00000002)
/** Position Report uses Cell-ID */
#define EP_POSITION_FLAG_CELLID_USED    ((epPositionFlags)0x00000004)
/** Position Report is DGNSS corrected */
#define EP_POSITION_FLAG_DGNSS_CORR     ((epPositionFlags)0x00000008)
/** Position Report is RTK corrected */
#define EP_POSITION_FLAG_RTK_CORR       ((epPositionFlags)0x00000010)
/** Position Report is PPP corrected */
#define EP_POSITION_FLAG_PPP_CORR       ((epPositionFlags)0x00000020)
/** Position Report uses Sensors  */
#define EP_POSITION_FLAG_DR             ((epPositionFlags)0x00000040)
/** Position Report uses SBAS Iono correction  */
#define EP_POSITION_FLAG_SBAS_IONO_CORR ((epPositionFlags)0x00000080)
/** Position Report uses SBAS Fast correction */
#define EP_POSITION_FLAG_SBAS_FAST_CORR ((epPositionFlags)0x00000100)
/** Position Report uses SBAS Long correction */
#define EP_POSITION_FLAG_SBAS_LONG_CORR ((epPositionFlags)0x00000200)
/** Position report is unpropagated.
    Sent when client subscribes for UNPROPAGATED_PVT report */
#define EP_POSITION_FLAG_UNPROPAGATED_PVT ((epPositionFlags)0x00000400)


/** Position mode DR only */
#define EP_POSITION_FLAG_DR_ONLY  (EP_POSITION_FLAG_DR)
/** Position mode GNSS+DR */
#define EP_POSITION_FLAG_GNSS_DR (EP_POSITION_FLAG_GNSS_USED | EP_POSITION_FLAG_DR)
/** Position mode GNSS only */
#define EP_POSITION_FLAG_GNSS_ONLY  (EP_POSITION_FLAG_GNSS_USED)
/** GNSS Unpropagated Position Report */
#define EP_POSITION_FLAG_GNSS_UNPROPAGATED_PVT\
           (EP_POSITION_FLAG_UNPROPAGATED_PVT | EP_POSITION_FLAG_GNSS_USED)
/** Position mode PPE only */
#define EP_POSITION_FLAG_PPE (EP_POSITION_FLAG_DGNSS_CORR |\
                                EP_POSITION_FLAG_RTK_CORR |\
                                EP_POSITION_FLAG_PPP_CORR)

/** Indicates PVT report calculated by which Position engine */
typedef uint8_t epPVTSourceType;
/** Position Engine source is unknown */
#define EP_POSITION_SOURCE_UNKNOWN ((epPVTSourceType)0x00)
/** Position source is GNSS Modem Engine */
#define EP_POSITION_SOURCE_SPE     ((epPVTSourceType)0x01)
/** Position source is precise position engine */
#define EP_POSITION_SOURCE_PPE     ((epPVTSourceType)0x02)
/** Position source is Dead-Reckoning Engine */
#define EP_POSITION_SOURCE_DRE     ((epPVTSourceType)0x03)

/** GNSS Measurement Report Status Bit Mask */
typedef uint16_t epGnssMeasUsageStatusBitMask;
/** Used in fix */
#define EP_GNSS_MEAS_USED_IN_PVT ((epGnssMeasUsageStatusBitMask)0x00000001)
/** Measurement is Bad */
#define EP_GNSS_MEAS_USAGE_STATUS_BAD_MEAS ((epGnssMeasUsageStatusBitMask)0x00000002)
/** Measurement has too low C/N */
#define EP_GNSS_MEAS_USAGE_STATUS_CNO_TOO_LOW ((epGnssMeasUsageStatusBitMask)0x00000004)
/** Measurement has too low elevation */
#define EP_GNSS_MEAS_USAGE_STATUS_ELEVATION_TOO_LOW ((epGnssMeasUsageStatusBitMask)0x00000008)
/** No ephemeris available for this measurement */
#define EP_GNSS_MEAS_USAGE_STATUS_NO_EPHEMERIS ((epGnssMeasUsageStatusBitMask)0x00000010)
/** No corrections available for the measurement */
#define EP_GNSS_MEAS_USAGE_STATUS_NO_CORRECTIONS ((epGnssMeasUsageStatusBitMask)0x00000020)
/** Corrections has timed out for the measurement */
#define EP_GNSS_MEAS_USAGE_STATUS_CORRECTION_TIMEOUT ((epGnssMeasUsageStatusBitMask)0x00000040)
/** Measurement is unhealthy */
#define EP_GNSS_MEAS_USAGE_STATUS_UNHEALTHY ((epGnssMeasUsageStatusBitMask)0x00000080)
/** Configuration is disabled for this measurement */
#define EP_GNSS_MEAS_USAGE_STATUS_CONFIG_DISABLED ((epGnssMeasUsageStatusBitMask)0x00000100)
/** Measurement not used for other reasons */
#define EP_GNSS_MEAS_USAGE_STATUS_OTHER ((epGnssMeasUsageStatusBitMask)0x00000200)

/** MAX Size of SV Polynomial Velocity Coefficient  */
#define EP_SV_POLY_VELOCITY_COEF_MAX_SIZE         12
/** SV Polynomial Zeroth order Coefficient  */
#define EP_SV_POLY_XYZ_0_TH_ORDER_COEFF_MAX_SIZE  3
/** SV Polynomial nth order Coefficient  */
#define EP_SV_POLY_XYZ_N_TH_ORDER_COEFF_MAX_SIZE  9
/** SV Clock-bias Coefficient  */
#define EP_SV_POLY_SV_CLKBIAS_COEFF_MAX_SIZE      4

/** Polynomials Status Bit Mask */
typedef uint16_t epPolyStatusMask;
/** Polynomials based on XTRA */
#define EP_SV_POLY_SRC_ALM_CORR    ((epPolyStatusMask)0x01)
/** GLONASS string 4 has been received */
#define EP_SV_POLY_GLO_STR4        ((epPolyStatusMask)0x02)

/** GNSS Measurement Status Mask */
typedef uint64_t epGNSSMeasurementStatusMask;
/** Satellite time in sub-millisecond (code phase) is known */
#define EP_MEAS_STATUS_SM_VALID              ((epGNSSMeasurementStatusMask)0x00000001)
/** Satellite sub-bit time is known */
#define EP_MEAS_STATUS_SB_VALID              ((epGNSSMeasurementStatusMask)0x00000002)
/** Satellite time in milliseconds is known */
#define EP_MEAS_STATUS_MS_VALID              ((epGNSSMeasurementStatusMask)0x00000004)
/** Signal bit edge is confirmed  */
#define EP_MEAS_STATUS_BE_CONFIRM            ((epGNSSMeasurementStatusMask)0x00000008)
/** Satellite Doppler is measured */
#define EP_MEAS_STATUS_VELOCITY_VALID        ((epGNSSMeasurementStatusMask)0x00000010)
/** Status as Reserved */
#define EP_MEAS_STATUS_RESERVED              ((epGNSSMeasurementStatusMask)0x00000020)
/** TRUE/FALSE : Lock Point is valid/invalid.*/
#define EP_MEAS_STATUS_LP_VALID              ((epGNSSMeasurementStatusMask)0x00000040)
/** TRUE/FALSE : Lock Point is positive/negative*/
#define EP_MEAS_STATUS_LP_POS_VALID          ((epGNSSMeasurementStatusMask)0x00000080)
/** Range update from satellite differences is measured */
#define EP_MEAS_STATUS_FROM_RNG_DIFF         ((epGNSSMeasurementStatusMask)0x00000200)
/** Doppler update from satellite differences is measured */
#define EP_MEAS_STATUS_FROM_VE_DIFF          ((epGNSSMeasurementStatusMask)0x00000400)
/** Fresh GNSS measurement observed in last second  */
#define EP_MEAS_STATUS_GNSS_FRESH_MEAS_VALID ((epGNSSMeasurementStatusMask)0x08000000)

/** GNSS SV Status Mask */
typedef uint16_t epGnssSvStatusMask;
/** GNSS SV has Ephemeris */
#define EP_GNSS_SV_OPTIONS_HAS_EPHEMERIS_BIT ((epGnssSvStatusMask)0x01)
/** GNSS SV has Almanac */
#define EP_GNSS_SV_OPTIONS_HAS_ALMANAC_BIT ((epGnssSvStatusMask)0x02)
/** GNSS SV used in fix */
#define EP_GNSS_SV_OPTIONS_USED_IN_FIX_BIT ((epGnssSvStatusMask)0x04)

/** GNSS SV Carrier phase ambiguity Type */
typedef uint8_t epCarrierPhaseAmbiguityType;
/** Carrier phase ambiguity not present */
#define EP_CARRIER_PHASE_AMBIGUITY_RESOLUTION_NONE ((epCarrierPhaseAmbiguityType)0)
/** Carrier phase ambiguity is present and float */
#define EP_CARRIER_PHASE_AMBIGUITY_RESOLUTION_FLOAT ((epCarrierPhaseAmbiguityType)1)
/** Carrier phase ambiguity is present and fix */
#define EP_CARRIER_PHASE_AMBIGUITY_RESOLUTION_FIXED ((epCarrierPhaseAmbiguityType)2)

/** Position fix status */
typedef uint8_t epSolutionStatus;
/** No Position fix */
#define EP_NO_FIX ((epSolutionStatus) 0)
/** Position fix is valid */
#define EP_VALID_FIX ((epSolutionStatus) 1)


/** GNSS SV search status */
typedef uint8_t epGnssMeasSvSearchState;
/** SV is not being actively searched */
#define EP_GNSS_SV_SRCH_STATE_IDLE (1)
/** The system is searching for this SV */
#define EP_GNSS_SV_SRCH_STATE_SEARCH (2)
/** SV is being tracked */
#define EP_GNSS_SV_SRCH_STATE_TRACK (3)

/** GNSS SV Info Mask */
typedef uint8_t epGnssMeasSvInfoMask;
/** Ephemeris is available for this SV */
#define EP_GNSS_SVINFO_MASK_HAS_EPHEMERIS ((epGnssMeasSvInfoMask)0x01)
/** Almanac is available for this SV */
#define EP_GNSS_SVINFO_MASK_HAS_ALMANAC ((epGnssMeasSvInfoMask)0x02)

/** Path for configuration files */
#define PPE_FILES_PATH “/data/vendor/location-partner/ppe/”
#define DRE_FILES_PATH “/data/vendor/location-partner/dre/”

#define EP_SVID_OFFSET_GLONASS     (64)
#define EP_SVID_OFFSET_BDS         (200)
#define EP_SVID_OFFSET_GALILEO     (300)
#define EP_SVID_OFFSET_QZSS        (192)
#define EP_SVID_INVALID_VALUE_FOR_GLONASS (255)


/** Feature status */
typedef enum {
    /** No license found */
    EP_FEATURE_STATUS_NONE = 0,
    /** Valid license found */
    EP_FEATURE_STATUS_OK,
    /** Expired license found */
    EP_FEATURE_STATUS_EXPIRED
} epFeatureStatus;


typedef enum {
    /** No license found */
    EP_INSTALL_LICENSE_STATUS_NONE = 0,
    /** Valid license found */
    EP_INSTALL_LICENSE_STATUS_OK,
    /** Invalid license Parametr
        Report this error to license server */
    EP_ERROR_LICENSE_INVAILD_PARAMS,
    /** Invalid license Status
        Retry after reboot */
    EP_ERROR_LICENSE_INVAILD_STATUS,
    /** Invalid license No Memory
        Retry after reboot */
    EP_ERROR_LICENSE_NO_MEMORY,
    /** Expired license found
        Report this error to license server */
    EP_ERROR_LICENSE_CERTIFICATE_EXPIRED,
    /** Incorrect OEM ID for License certificate
        Report this error to license server */
    EP_ERROR_LICENSE_CERTIFICATE_OEM,
    /** Incorrect HW version for License certificate
        Report this error to license server */
    EP_ERROR_LICENSE_CERTIFICATE_HWVERSION,
    /** Incorrect Device ID for License certificate
        Report this error to license server */
    EP_ERROR_LICENSE_CERTIFICATE_DEVICEID,
    /** Incorrect PFM for License certificate
        Report this error to license server */
    EP_ERROR_INVALID_PFM_EXTENSION,
    /** Incorrect PFMFILER for License certificate
        Retry after reboot */
    EP_ERROR_PFMFILER_FAILD
} epInstallLicenseStatus;

/** GNSS Session control commands */
typedef enum {
    /** Session start request */
    EP_COMMAND_START = 0,
    /** Session stop request */
    EP_COMMAND_STOP,
    /** Session position mode set request */
    EP_COMMAND_SET_POSITION_MODE,
    /** Engine reset request
        Recommended to be used when engine library is not responding */
    EP_COMMAND_RESET,
    /** Delete aiding data stored in the engine */
    EP_COMMAND_DELETE_AIDING_DATA,
    /** Max enum, add new enum above this line */
    EP_COMMAND_MAX = UINT8_MAX
} epCommandType;

typedef enum {
   /** License type is unknow */
   EP_LICENSE_UNDEFINED,
   /** The evaluation license is provided */
   EP_LICENSE_EVALUATION,
   /** The License is provided for the commercial purpose */
   EP_LICENSE_PRODUCTION
} epLicenseType;

typedef enum {
    /** Attestation token is created successfully */
    EP_ATTESTATION_STATUS_OK = 0,
    /** The security has been compromised
        Report this error to license server */
    EP_ATTESTATION_INVAILD_STATUS,
    /** Error no memory
        Retry after reboot */
    EP_ATTESTATION_NO_MEMORY
} epAttestationStatus;

typedef enum {
   /** Update broadcast ephemeris for this satellite */
   UPDATE_EPH = 0,
   /** Delete this broadcast ephemeris that has been sent before */
   DELETE_EPH
} ephEphemerisUpdateAction;

/** GALILEO Signal Type */
typedef enum {
/** GALILEO E1 MBOC Pilot RF Band */
   E1B,
/** GALILEO E5A BPSK Pilot RF Band */
   E5A,
/** GALILEO E5B RF Band */
   E5B
} epGalSVDataSource;

/** Validity bits for fields in epPVTReport */
typedef struct
{
    uint64_t isUtcTimestampMsValid : 1;
    uint64_t isLatitudeValid : 1;
    uint64_t isLongitudeValid : 1;
    uint64_t isAltitudeWrtEllipsoidValid : 1;
    uint64_t isAltitudeWrtMeanSeaLevelValid : 1;
    uint64_t isNorthStdDeviationValid : 1;
    uint64_t isEastStdDeviationValid : 1;
    uint64_t isAltitudeStdDeviationValid : 1;
    uint64_t isHorizontalSpeedValid : 1;
    uint64_t isHorizontalSpeedUncValid : 1;
    uint64_t isNorthVelocityValid : 1;
    uint64_t isEastVelocityValid : 1;
    uint64_t isUpVelocityValid : 1;
    uint64_t isNorthVelocityStdDeviationValid : 1;
    uint64_t isEastVelocityStdDeviationValid : 1;
    uint64_t isUpVelocityStdDeviationValid : 1;
    uint64_t isHeadingDegValid : 1;
    uint64_t isHeadingUncDegValid : 1;
    uint64_t isMagneticDeviationDegValid : 1;
    uint64_t isHorizontalEllipticalUncSemiMajorAxisValid : 1;
    uint64_t isHorizontalEllipticalUncSemiMinorAxisValid : 1;
    uint64_t isHorizontalEllipticalUncAZValid : 1;
    uint64_t isClockbiasValid : 1;
    uint64_t isClockBiasStdDeviationValid : 1;
    uint64_t isClockDriftValid : 1;
    uint64_t isClockDriftStdDeviationValid : 1;
    uint64_t isPdopValid : 1;
    uint64_t isHdopValid : 1;
    uint64_t isVdopValid : 1;
    uint64_t isGdopValid : 1;
    uint64_t isTdopValid : 1;
    uint64_t isLongAccelValid : 1;
    uint64_t isLatAccelValid : 1;
    uint64_t isVertAccelValid : 1;
    uint64_t isYawRateValid : 1;
    uint64_t isPitchRadValid : 1;
    uint64_t isReferenceStationValid : 1;
    uint64_t isLongAccelUncValid : 1;
    uint64_t isLatAccelUncValid : 1;
    uint64_t isVertAccelUncValid : 1;
    uint64_t isYawRateUncValid : 1;
    uint64_t isPitchRadUncValid : 1;
    uint64_t isPpsLocaltimeStampValid: 1;
    uint64_t isHeadingRateDegValid: 1;
    uint64_t : 20;
}  epPVTReportValidity;

/** Flags to indicate valid fields in epGnssSystemTimeStructType */
typedef struct {
    uint16_t isSystemWeekValid : 1;
    uint16_t isSystemWeekMsecValid : 1;
    uint16_t isSystemClkTimeBiasValid : 1;
    uint16_t isSystemClkTimeUncMsValid : 1;
    uint16_t isRefFCountValid : 1;
    uint16_t isNumClockResetsValid : 1;
    uint16_t : 10;
}epSystemTimeValidity;

/** Flags to indicate valid fields in epGnssLocGloTimeStructType */
typedef uint32_t epGloTimeValidityM;
typedef struct {
    uint16_t isGloClkTimeBiasValid : 1;
    uint16_t isGloClkTimeUncMsValid : 1;
    uint16_t isRefFCountValid : 1;
    uint16_t isNumClockResetsValid : 1;
    uint16_t : 12;
}epGloTimeValidity;

/** Validity bits for fields in epPVTReport */
typedef struct
{
    uint32_t isIodeValid : 1;
    uint32_t isT0Valid : 1;
    uint32_t isPolyCoeffXYZ0Valid : 1;
    uint32_t isPolyCoefXYZNValid :  1;
    uint32_t isPolyCoefOtherValid : 1;
    uint32_t isSvPosUncValid: 1;
    uint32_t isIonoDelayValid : 1;
    uint32_t isIonoDotValid : 1;
    uint32_t isSbasIonoDelayValid : 1;
    uint32_t isSbasIonoDotValid : 1;
    uint32_t isTropoDelayValid : 1;
    uint32_t isElevationValid : 1;
    uint32_t isElevationDotValid : 1;
    uint32_t isElevationUncValid : 1;
    uint32_t isVelCoefValid : 1;
    uint32_t isEnhancedIODValid : 1;
    uint32_t : 16;
}  epPolyReportValidity;


/** Flags to indicate valid fields in epGnssSVMeasurementStructType */
typedef struct {
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
} gnssSVMeasurementValidity;

/** Flags to indicate SV LLI */
typedef struct {
    uint8_t lossOfContinousLock : 1;
    uint8_t halfCycleAmbiguity : 1;
    uint8_t : 6;
} epGnssMeasSvLLI;


/** Flags to indicate valid fields in epGnssMeasurementReport */
typedef struct {
    uint16_t isLeapSecValid : 1;
    uint16_t isClockDriftValid : 1;
    uint16_t isClockDriftStdDeviationValid : 1;
    uint16_t isGpsSystemTimeValid : 1;
    uint16_t isGalSystemTimeValid : 1;
    uint16_t isBdsSystemTimeValid : 1;
    uint16_t isQzssSystemTimeValid : 1;
    uint16_t isGloSystemTimeValid : 1;
    uint16_t : 8;
} epGnssMeasurementReportValidity;

/** Flags to indicate valid fields in epGnssSv */
typedef struct {
    uint8_t isGloFrequencyValid : 1;
    uint8_t isElevationValid : 1;
    uint8_t isAzimuthValid : 1;
    uint8_t : 5;
} epGnssSvValidity;

/** UTC Time stamp. Units: Milliseconds since Jan. 1, 1970 */
typedef uint64_t epUtcTimestampMs;

/** Subscription request command parameters */
typedef struct {
    /** Subscribe Constellation Type
        Mandatory Field */
    epGnssConstellationTypeMask constellationType;

    uint8_t reserve[16];
} epSubscriptionModifier;

/** Subscribe required reports from Engine Plug-in */
typedef struct {
    /** Report subscription mask
        Mandatory Field */
    epReportSubscriptionMask flags;
    /** Modifier applicable to subscription of measurement,
        ephemeris, polynomial */

    /** Subscription modifier for Measurement. It is valid only
        when EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT set */
    epSubscriptionModifier subscriptionModMeasurement;
    /** Subscription modifier for Measurement It is valid only
        when EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT set */
    epSubscriptionModifier subscriptionModMeasurementNHz;
    /** Subscription modifier for Ephemeris It is valid only
        when EP_SUBSCRIPTION_EPHEMERIS_REPORT set */
    epSubscriptionModifier subscriptionModEphemeris;
    /** Subscription modifier for Polynomial It is valid only
        when EP_SUBSCRIPTION_POLYNOMIAL_REPORT set */
    epSubscriptionModifier subscriptionModPolynomial;

    uint8_t reserve [16];
} epSubscriptionInfo;

/** Position mode command parameters  */
typedef struct {
    /** GNSS session interval in millisecond
        Value-0 indicates Single shot session
        Non zero value indicates tracking session with interval specified
        Valid only when command is
        COMMAND_SET_POSITION_MODE command
        Mandatory Field */
    unsigned int interval;
    uint8_t reserve[32];
} epSetPositionModeStructType;


/** Delete Aiding command parameters  */
typedef struct {
    /** Flag indicating aiding data to delete
        Valid in case of COMMAND_DELETE_AIDING_DATA command
        Mandatory Field */
    epAidingdataMask flags;
    uint8_t reserve[32];
} epDeleteAidingDataStructType;


/** Engine Plug-in Command parameters */
typedef struct {
    /** Session control command which includes start, stop, deleteAidData
        Mandatory Field */
    epCommandType command;
    /** Command parameters for set position mode
        and delete Aiding data commands */
    union {
      /** Parameters for Set position command */
      epSetPositionModeStructType  positionModeInfo;
      /** Parameters for Delete Aiding Data */
      epDeleteAidingDataStructType deleteAidingData;
    } cmdReqParameter;
    uint8_t reserve[32];
} epCommandRequest;

/** Indicates AP time stamp  */
typedef struct {
    /** Boot time received from pps-ktimer
        Mandatory Field */
    timespec apTimeStamp;
    /** Time-stamp uncertainty in Milliseconds
        Mandatory Field */
    float apTimeStampUncertaintyMs;
    uint8_t reserve[32];
} epGnssApTimeStampStructType;

/** GNSS System Time */
typedef struct {
    /** Validity mask for below fields */
    epSystemTimeValidity validityMask;
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
    uint8_t reserve[16];
} epGnssSystemTimeStructType;


/** GNSS GLONASS Time */
typedef struct {
    /** GLONASS day number in four years. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 65535
        Mandatory Field */
    uint16_t gloDays;
    /** GLONASS time of day in Millisecond. Refer to GLONASS ICD.
        Units: Millisecond
        Check for gloClkTimeUncMs before use */
    /** Validity mask for below fields */
    epGloTimeValidity validityMask;
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
    uint8_t reserve[16];
} epGnssGloTimeStructType;


/** GNSS Measurement Usage Info */
typedef struct {
    /** Specifies one GNSS signal type
        Mandatory Field*/
    epGnssSignalTypeMask gnssSignalType;
    /** Specifies one GNSS Constellation Type
        Mandatory Field */
    epGnssConstellationTypeMask gnssConstellation;
    /**  GNSS SV ID.
         For GPS:      1 to 32
         For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
         For SBAS:     120 to 151
         For QZSS-L1CA:193 to 197
         For BDS:      201 to 237
         For GAL:      301 to 336
         Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 8.
        Valid only for a GLONASS system and
        is to be ignored for all other systems.
        Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Carrier phase ambiguity type.
        Mandatory Field */
    epCarrierPhaseAmbiguityType carrierPhaseAmbiguityType;
    /** Specifies measurement usage status
        Mandatory Field */
    epGnssMeasUsageStatusBitMask measUsageStatusMask;
    uint8_t reserve[34];
} epMeasUsageInfo;


/** Position velocity Time Info */
typedef struct {
    /** Specifies position flags.
        Mandatory field */
    epPositionFlags posFlags;
    /** Status of PVT report.
        Mandatory field */
    epSolutionStatus statusOfFix;
    /** Time applicability of PVT report */
    struct {
      /** Specifies GNSS system time reported.
          Mandatory field */
      epGnssConstellationTypeMask gnssSystemTimeSrc;
      /** Reporting of GPS system time is recommended.
         If GPS time is unknown & other satellite system time is known,
         it should be reported.
         Mandatory field
      */
      union {
          epGnssSystemTimeStructType gpsSystemTime;
          epGnssSystemTimeStructType galSystemTime;
          epGnssSystemTimeStructType bdsSystemTime;
          epGnssSystemTimeStructType qzssSystemTime;
          epGnssGloTimeStructType gloSystemTime;
      }u;
    } gnssSystemTime;
    /** UTC time stamp. Mandatory, if Leap Second is known.
        Unit: Milliseconds since Jan. 1, 1970 */
    epUtcTimestampMs utcTimestampMs;
    /** Local time of last PPS pulse, associated with this PVT report
        SPE provides PPS TimeStamp, it is optional for other Engines */
    epGnssApTimeStampStructType lastPpsLocaltimeStamp;
    /** Validity bits for fields in epPVTReport */
    epPVTReportValidity validityMask;
    /** Latitude (specified in WGS84 datum).
        Units: Degrees
        Range: -90.0 to 90.0
        Positive values indicate northern latitude
        Negative values indicate southern latitude */
    double latitudeDeg;
    /** Longitude (specified in WGS84 datum).
        Units: Degrees
        Range: -180.0 to 180.0
        Positive values indicate eastern longitude
        Negative values indicate western longitude */
    double longitudeDeg;
    /** Altitude with respect to the WGS84 ellipsoid.
        Unit:Meters. Range: -500 to 15883 */
    float altitudeWrtEllipsoid;
    /** Altitude with respect to mean sea level.
        Unit: Meters */
    float altitudeWrtMeanSeaLevel;
    /** North standard deviation.
        Unit: Meters */
    float northStdDeviation;
    /** East standard deviation.
        Unit: Meters */
    float eastStdDeviation;
    /** Altitude standard deviation.
        Unit: Meters */
    float altitudeStdDeviation;
    /** Horizontal Speed.
        Unit: Meters/sec */
    float horizontalSpeed;
    /** Horizontal Speed uncertainty
        Unit: Meters/sec */
    float horizontalSpeedUnc;
    /** North Velocity.
        Unit: Meters/sec */
    float northVelocity;
    /** East Velocity.
        Unit: Meters/sec */
    float eastVelocity;
    /** Up Velocity.
        Unit: Meters/sec */
    float upVelocity;
    /** North Velocity standard deviation.
        Unit: Meters/sec */
    float northVelocityStdDeviation;
    /** East Velocity standard deviation.
        Unit: Meters/sec */
    float eastVelocityStdDeviation;
    /** Up Velocity standard deviation
        Unit: Meters/sec */
    float upVelocityStdDeviation;
    /** Heading angle, referenced to North (0 deg) and positive clock-wise.
        Range: 0 to 359.999. Unit: Degrees.
        Check for headingUnc before use */
    float headingDeg;
    /** Heading uncertainty. Range: 0 to 359.999. Unit: Degrees */
    float headingUncDeg;
    /** Difference between the bearing to true north and the bearing shown
        on a magnetic compass. The deviation is positive when the magnetic
        north is east of true north.
        Unit: Degrees */
    float magneticDeviationDeg;
    /** Semi-major axis of horizontal elliptical uncertainty.
        Unit: Meters */
    float horUncEllipseSemiMajor;
    /** Semi-minor axis of horizontal elliptical uncertainty.
        Unit: Meters */
    float horUncEllipseSemiMinor;
    /** Elliptical horizontal uncertainty azimuth of orientation.
        Range: 0 to 180
        Units: Decimal degrees */
    float horUncEllipseOrientAzDeg;
    /** Estimated clock bias. Unit: Meter */
    float clockbiasMeter;
    /** Estimated clock bias std deviation. Unit: Meter */
    float clockBiasStdDeviationMeter;
    /** Estimated clock drift. Unit: Meters/sec */
    float clockDrift;
    /** Estimated clock drift std deviation. Unit: Meters/sec */
    float clockDriftStdDeviation;
    /** Position dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float pDop;
    /** Horizontal dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float hDop;
    /** Vertical dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float vDop;
    /** Geometric  dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float gDop;
    /** Time dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float tDop;
    /** Forward Acceleration in body frame
        Unit: Meter/sec^2 */
    float longAccel;
    /** Side-ward Acceleration in body frame
        Unit: Meter/sec^2 */
    float latAccel;
    /** Vertical Acceleration in body frame
        Unit: Meter/sec^2 */
    float vertAccel;
    /**  Yaw rate is in body frame
        Unit: Radian/sec */
    float yawRate;
    /** Body pitch
        Unit: Radian */
    float pitchRad;
    /** Number of valid reference stations. Range:[0-4] */
    uint8_t numValidRefStations;
    /** Reference station(s) number */
    uint16_t referenceStation[4];
    /** Number of measurements received for use in fix.
        Shall be used as maximum index in-to svUsageInfo[].
        Set to 0, if svUsageInfo reporting is not supported.
        Range: 0-EP_GNSS_MAX_MEAS */
    uint8_t numOfMeasReceived;
    /** Measurement Usage Information */
    epMeasUsageInfo measUsageInfo[EP_GNSS_MAX_MEAS];
    /** Engine information generating this PVT.
        Shall be filled EnginePlugin component.
        Mandatory field */
    epPVTSourceType pvtSource;
    /** Uncertainty of Forward Acceleration in body frame */
    float longAccelUnc;
    /** Uncertainty of Side-ward Acceleration in body frame */
    float latAccelUnc;
    /** Uncertainty of Vertical Acceleration in body frame */
    float vertAccelUnc;
    /** Uncertainty of Heading Rate */
    float yawRateUnc;
    /** Uncertainty of Body pitch */
    float pitchRadUnc;
    /** Heading Rate is in NED frame.
        Range: 0 to 359.999.
        Unit: Degrees per Seconds */
    float headingRateDeg;
    uint8_t reserve[39];
} epPVTReport;


/** Represents GPS Navigation model info */
typedef struct {
    uint32_t validityMask;
    /**  GNSS SV ID.
         For GPS:      1 to 32
         Mandatory Field */
    uint16_t gnssSvId;
    /** Ephemeris Action
        Mandatory Field */
    ephEphemerisUpdateAction emhprAction;
    /** The six-bit health indication as defined in GPS ICD, for L1/L2 only
        Mandatory Field */
    uint8_t svHealthMask;
    /** Issue of Data (ephemeris)
        Mandatory Field */
    uint8_t iode;
    /** Square root of the semi-major axis
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radians/s
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radians
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radians
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radians
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radians
        Mandatory Field */
    double omegaARad;
    /** Rate of right ascension
        Unit: radians/s
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radians/s
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUsRad;
    /** Amplitude of the cosine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRcMeter;
    /** Amplitude of the sine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRsMeter;
    /** Amplitude of the cosine harmonic correction term
        to the angle of inclination
        Unit: radians/s
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radians)
        Unit: radians/s
        Mandatory Field */
    double cIsRad;
    /** Reference time ephemeris
        Unit: second
        Mandatory Field */
    double toeS;
    /** Clock data reference time
        Unit: s
        Mandatory Field */
    double tocS;
    /** Clock bias correction coefficient
        Unit: s
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: seconds/s
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: seconds/s^2
        Mandatory Field */
    double af2;
    /** User Range Accuracy Index(4bit)
        Mandatory Field */
    uint8_t urai;
    /** codes on L2 channel
        Mandatory Field */
    uint8_t codeL2;
    /** L2 P data flag
        Mandatory Field */
    uint8_t L2PData;
    /** Indicates the curve-fit interval used by the CS
        Unit: hours
        Range: 0 = 0-4 hours
               1 = greater than 4 hours
        Mandatory Field */
    uint8_t fitInterval;
    /** Time of Group delay
        Unit:seconds
        Mandatory Field */
    double  tgd;
    uint8_t reserve[64];
} epGpsSatelliteEphemerisData;

/** Represents Glonass Navigation model info */
typedef struct {
    uint32_t validityMask;
   /**  GNSS SV ID.
         For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown,
         set as 255.
         Mandatory Field */
    uint16_t gnssSvId;
    /** Ephemeris Action
        Mandatory Field */
    ephEphemerisUpdateAction emhprAction;
    /** Satellite health. 0 means broadcasting satellite is good and 1 means not
        Mandatory Field */
    uint8_t bnHealth;
    /** Ln SV health flages for GLONASS-M. 0 means broadcasting satellite is good
        and 1 means not
        Mandatory Field */
    uint8_t lnHealth;
    /** Index of a time interval within current day according to UTC(SU)+03 hours 00 min
        Mandatory Field */
    uint8_t tb;
    /** User Range Accuracy index.  P32 ICD Glonass for value of Ft.
        Mandatory Field */
    uint8_t FT;
    /** Glonass vehicle type. M=1 means type M
        Mandatory Field */
    uint8_t M;
    /** Age of current information in days
        Unit: days
        Mandatory Field */
    uint8_t En;
    /** Carrier frequency number + 8
        Range: 1-14
        Mandatory Field */
    uint8_t feqNo;
    /** Time interval between adjacent values of tb
        Unit: minutes
        Mandatory Field */
    uint8_t P1;
    /** if tb is odd and 0 if tb is even
        Mandatory Field */
    uint8_t P2;
    /** Time difference between navigation RF signal transmission in G2 and G1
        Unit: second
        Mandatory Field */
    float deltaTau;
    /** X component of SV position
        Unit: meter
        Mandatory Field */
    double x;
    /** Y component of SV position
        Unit: meter
        Mandatory Field */
    double y;
    /** Z component of SV position
        Unit: meter
        Mandatory Field */
    double z;
    /** X component of SV velocity
        Unit: meter/sec
        Mandatory Field */
    double vx;
    /** Y component of SV velocity
        Unit: meter/sec
        Mandatory Field */
    double vy;
    /** Z component of SV velocity
        Unit: meter/sec
        Mandatory Field */
    double vz;
    /** X component of SV sola-luni acceleration
        Unit: meter/sec^2
        Mandatory Field */
    double ax;
    /** Y component of SV sola-luni acceleration
        Unit: meter/sec^2
        Mandatory Field */
    double lsy;
    /** Z component of SV sola-luni acceleration
        Unit: meter/sec^2
        Mandatory Field */
    double lsz;
    /** SV clock bias, offset between GLONASS system time and the satellite clock
        Unit: second
        Mandatory Field */
    float tauN;
    /** SV clock frequency error ratio
        Mandatory Field */
    float gamma;
    /** Current date, calendar number of day within four-year interval.
        Starting from the 1st of January in a leap year
        Unit: days
        Mandatory Field */
    uint8_t nt;
    uint8_t reserve[64];
} epGlonassSatelliteEphemerisData;


/** BDS Navigation model Info */
typedef struct {
    uint64_t validityMask;
    /**  GNSS SV ID.
         For BDS:      201 to 237
         Mandatory Field */
    uint16_t gnssSvId;
    /** Ephemeris Action
        Mandatory Field */
    ephEphemerisUpdateAction emhprAction;
    /** Autonomous satellite Health flag applied for both B1 and B2 (SatH1)
        Mandatory Field */
    uint8_t svHealthMask;
    /** Age of Data, Ephemeris
        Mandatory Field */
    uint8_t aode;
    /** Age of Data, Clock
        Mandatory Field */
    uint8_t aodc;
    /** Square root of the semi-major axis
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radians/s
        Mandatory Field */
    double deltaNRadPerSec;
   /** Mean anomaly at reference time
        Unit: radians
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radians
        Mandatory Field */
    double omegaORad;
   /** Inclination angle at reference time
        Unit: radians
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radians
        Mandatory Field */
    double omegaARad;
    /** Rate of right ascension
        Unit: radians/s
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radians/s
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUsRad;
    /** Amplitude of the cosine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRcMeter;
    /** Amplitude of the sine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRsMeter;
    /** Amplitude of the cosine harmonic correction term
        to the angle of inclination
        Unit: radians/s
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radians)
        Unit: radians/s
        Mandatory Field */
    double cIsRad;
  /** Reference time ephemeris
        Unit: second
        Mandatory Field */
    double toeSec;
   /** Clock data reference time
        Unit: second
        Mandatory Field */
    double tocSec;
    /** Clock bias correction coefficient
        Unit: second
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: seconds/s
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: seconds/s^2
        Mandatory Field */
    double af2;
    /** Equipment group delay differential for the B1 signal
        Unit: nenoseconds
        Mandatory Field */
    double tgd1Sec;
    /** Equipment group delay differential for the B2 signal
        Unit: nenoseconds
        Mandatory Field */
    double tgd2Sec;
    /** User Range Accuracy Index(4bit)
        Mandatory Field */
    uint8_t urai;
    uint8_t reserve[64];
} epBDSSatelliteEphemerisData;

/** GALILEO Navigation model Info */
typedef struct {
    uint32_t validityMask;
    /**  GNSS SV ID.
         For GAL:      301 to 336
         Mandatory Field */
    uint16_t gnssSvId;
    /** Ephemeris Action
        Mandatory Field */
    ephEphemerisUpdateAction emhprAction;
    /** SV Health Status of signal identified by dataSource.
        Mandatory Field */
    uint8_t svHealth;
    /** GALILEO Signal Type
        Mandatory Field */
    epGalSVDataSource dataSource;
    /** Ephemeris and clock correction issue of data
        Mandatory Field */
    uint16_t iodNav;
    /** Mean motion difference
        Unit: radians/s
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radians
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Square root of the semi-major axis
        Mandatory Field */
    double aSqrt;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radians
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radians
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radians
        Mandatory Field */
    double omegaARad;
    /** Rate of right ascension
        Unit: radians/s
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radians/s
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUsRad;
    /** Amplitude of the cosine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRcMeter;
    /** Amplitude of the sine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRsMeter;
    /** Amplitude of the cosine harmonic correction term
        to the angle of inclination
        Unit: radians/s
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radians)
        Unit: radians/s
        Mandatory Field */
    double cIsRad;
    /** Reference time ephemeris
        Unit: second
        Mandatory Field */
    double toeS;
    /** Clock data reference time
        Unit: s
        Mandatory Field */
    double tocS;
    /** SV Clock bias correction coefficient
        Unit: seconds
        Mandatory Field */
    double af0Sec;
    /** SV Clock drift correction coefficient
        Unit: seconds/s
        Mandatory Field */
    double af1;
    /** SV Clock drift rate correction coefficient
        Unit: seconds/s^2
        Mandatory Field */
    double af2;
    /** E1-E5ab group delay from F/Nav (E5a).
        Unit: seconds
        Mandatory Field */
    double bgdE1E5a;
    /** E1-E5b group delay from I/Nav (E1B or E5b). For E1B or E5b signal,
        both bgdE1E5a and bgdE1E5b are vailid. For E5a signal,
        only bgdE1E5a is valid. Signal source identified by dataSource.
        Unit: seconds
        Mandatory Field */
    double bgdE1E5b;
    /** Signal-in-space accuracy index for dual frequency
        E1-E5b/E5a depending on dataSource
        Mandatory Field */
    uint8_t sisaIndex;
    uint8_t reserve[64];
} epGalileoSatelliteEphemerisData;

/** Represents QZSS Navigation model info */
typedef struct {
    uint32_t validityMask;
    /**  QZSS SV ID.
         For QZSS:      193 to 202
         Mandatory Field */
    uint16_t gnssSvId;
    /** Ephemeris Action
        Mandatory Field */
    ephEphemerisUpdateAction emhprAction;
    /** SV Health
        Mandatory Field */
    uint8_t svHealthMask;
    /** Issue of Data (ephemeris)
        Mandatory Field */
    uint8_t iode;
    /** Square root of the semi-major axis
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radians/s
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radians
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radians
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radians
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radians
        Mandatory Field */
    double omegaARad;
    /** Rate of right ascension
        Unit: radians/s
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radians/s
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUsRad;
    /** Amplitude of the cosine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRcMeter;
    /** Amplitude of the sine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRsMeter;
    /** Amplitude of the cosine harmonic correction term
        to the angle of inclination
        Unit: radians
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radians)
        Unit: radians
        Mandatory Field */
    double cIsRad;
    /** Reference time ephemeris
        Unit: s
        Mandatory Field */
    double toeS;
    /** Clock data reference time
        Unit: s
        Mandatory Field */
    double tocS;
    /** Clock bias correction coefficient
        Unit: seconds
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: seconds/s
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: seconds/s^2
        Mandatory Field */
    double af2;
    /** User Range Accuracy Index(4bit)
        Mandatory Field */
    uint8_t urai;
    /** codes on L2 channel
        Mandatory Field */
    uint8_t codeL2;
    /** L2 P data flag
        Mandatory Field */
    uint8_t L2PData;
    /** Indicates the curve-fit interval used by the CS
        Unit: hours
        Range: 0 = 0-4 hours
               1 = greater than 4 hours
        Mandatory Field */
    uint8_t fitInterval;
    /** Time of Group delay
        Unit:seconds
        Mandatory Field */
    double  tgd;
    uint8_t reserve[64];
} epQzssSatelliteEphemerisData;


/** GPS Navigation model for each SV */
typedef struct {
    uint16_t numOfEphemeris;
    epGpsSatelliteEphemerisData gpsEphemerisData[32];
    uint8_t reserve[64];
} epGpsEphemerisResponse;

/** GLONASS Navigation model for each SV */
typedef struct {
    uint16_t numOfEphemeris;
    epGlonassSatelliteEphemerisData gloEphemerisData[32];
    uint8_t reserve[64];
} epGlonassEphemerisResponse;

/** BDS Navigation model for each SV */
typedef struct {
    uint16_t numOfEphemeris;
    epBDSSatelliteEphemerisData bdsEphemerisData[32];
    uint8_t reserve[64];
} epBdsEphemerisResponse;

/** GALILEO Navigation model for each SV */
typedef struct {
    uint16_t numOfEphemeris;
    epGalileoSatelliteEphemerisData galEphemerisData[32];
    uint8_t reserve[64];
} epGalileoEphemerisResponse;

/** QZSS Navigation model for each SV */
typedef struct {
    uint16_t numOfEphemeris;
    epQzssSatelliteEphemerisData qzssEphemerisData[32];
    uint8_t reserve[64];
} epQzssEphemerisResponse;


/** Navigation model */
typedef struct {
    /** Indicates GNSS Constellation Type
        Mandatory field */
    epGnssConstellationTypeMask gnssConstellation;
    union {
       /** GPS Ephemeris */
       epGpsEphemerisResponse gpsEphemeris;
       /** GLONASS Ephemeris */
       epGlonassEphemerisResponse glonassEphemeris;
       /** BDS Ephemeris */
       epBdsEphemerisResponse bdsEphemeris;
       /** GALILEO Ephemeris */
       epGalileoEphemerisResponse galileoEphemeris;
       /** QZSS Ephemeris */
       epQzssEphemerisResponse qzssEphemeris;
    } ephInfo;
    uint8_t reserve[64];
} epGnssEphemerisReport;


/** GNSS SV Polynomial Info */
typedef struct {
    /** GNSS SV ID.
        For GPS:      1 to 32
        For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
        For SBAS:     120 to 151
        For QZSS-L1CA:193 to 197
        For BDS:      201 to 237
        For GAL:      301 to 336
        0: Invalid,
        All others are reserved
        Mandatory field */
    uint16_t gnssSvId;
    /** Freq index, only valid if u_SysInd is GLO */
    int8_t freqNum;
    /** Polynomial status mask
        Mandatory field */
    epPolyStatusMask polyStatus;
    /** Validity bits for following fields */
    epPolyReportValidity validityMask;
    /** Ephemeris reference time
        GPS:Issue of Data Ephemeris used [unit less].
        GLO: Tb 7-bit, refer to ICD02 */
    uint16_t iode;
    /** Reference time for polynomial calculations
        GPS: Secs in week.
        GLO: Full secs since Jan/01/96
    */
    double T0;
    /** C0X, C0Y, C0Z */
    double polyCoeffXYZ0[EP_SV_POLY_XYZ_0_TH_ORDER_COEFF_MAX_SIZE];
    /** C1X, C2X ... C2Z, C3Z */
    double polyCoefXYZN[EP_SV_POLY_XYZ_N_TH_ORDER_COEFF_MAX_SIZE];
    /** C0T, C1T, C2T, C3T */
    float polyCoefOther[EP_SV_POLY_SV_CLKBIAS_COEFF_MAX_SIZE];
    /** SV position uncertainty [m]. */
    float svPosUnc;
    /** Ionospheric delay at d_T0 [m]. */
    float ionoDelay;
    /** Ionospheric delay rate [m/s].  */
    float ionoDot;
    /** SBAS Ionospheric delay at d_T0 [m]. */
    float sbasIonoDelay;
    /** SBAS Ionospheric delay rate [m/s].  */
    float sbasIonoDot;
    /** Tropospheric delay [m]. */
    float tropoDelay;
    /** Elevation [rad] at d_T0 */
    float elevation;
    /** Elevation rate [rad/s] */
    float elevationDot;
    /** SV elevation [rad] uncertainty */
    float elevationUnc;
    /** Coefficients of velocity polynomial */
    double velCoef[EP_SV_POLY_VELOCITY_COEF_MAX_SIZE];
    /** Enhanced Reference Time */
    uint32_t enhancedIOD;
    uint8_t reserve[32];
} epGnssSvPolynomial;


/** GNSS SV Info */
typedef struct {
    /** Specifies GNSS signal type
        Mandatory Field */
    epGnssSignalTypeMask gnssSignalType;
    /** Specifies GNSS Constellation Type
        Mandatory Field */
    epGnssConstellationTypeMask gnssConstellation;
    /**  GNSS SV ID.
         For GPS:      1 to 32
         For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
         For SBAS:     120 to 151
         For QZSS-L1CA:193 to 197
         For BDS:      201 to 237
         For GAL:      301 to 336
         Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 8.
        Valid only for a GLONASS system and
        is to be ignored for all other systems.
        Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Validity bits for following fields */
    epGnssSvValidity validityMask;
    /** signal strength
        Mandatory field */
    float cN0dbHz;
    /** SV elevation Unit: degrees */
    float elevation;
    /** SV azimuth Unit: degrees */
    float azimuth;
    /** SV status. Mandatory Field */
    epGnssSvStatusMask gnssSvStatusMask;
    uint8_t reserve[64];
} epGnssSv;

/** GNSS SV Notification */
typedef struct {
    /** Number of SVs in the gnssSvs array
        Mandatory field */
    uint8_t svCount;
    /** SV Information, Mandatory field */
    epGnssSv gnssSvs[EP_GNSS_MAX_MEAS];
    uint8_t reserve[64];
} epGnssSvNotification;


/** GNSS SV Measurement Info */
typedef struct {
    /** Specifies GNSS signal type. Mandatory field */
    epGnssSignalTypeMask gnssSignal;
    /** Specifies GNSS system. Mandatory field */
    epGnssConstellationTypeMask gnssSystem;
    /** Specifies GNSS SV ID.
             For GPS:             1 to 32
             For GLONASS:     65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
             For SBAS:           120 to 151
             For QZSS-L1CA:  193 to 197
             For BDS:             201 to 237
             For GAL:              301 to 336
             Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 8.
         Valid only for a GLONASS system and
         is to be ignored for all other systems.
             - Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Specifies satellite search state. Mandatory field */
    epGnssMeasSvSearchState svSearchState;
    /**  Carrier to Noise ratio
         - Units: 0.1 dBHz
         Mandatory field */
    float cN0dbHz;
    /**  Bit-mask indicating SV measurement status.
         Valid bit-masks:
         If any MSB bit in 0xFFC0000000000000 DONT_USE is set, the measurement
         must not be used by the client.
         Mandatory field */
    epGNSSMeasurementStatusMask  measurementStatus;
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
    gnssSVMeasurementValidity validityMask;
    /** Health status.
        Range: 0 to 2;
        0 = unhealthy, 1 = healthy, 2 = unknown */
    uint8_t healthStatus;
    /** Indicates whether almanac and ephemeris information is available. */
    epGnssMeasSvInfoMask svInfoMask;
    /** Loss of signal lock indicator */
    epGnssMeasSvLLI lossOfLock;
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
    uint8_t reserve[32];
} epGnssSVMeasurementStructType;

/** GNSS Measurement Report */
typedef struct {
    /** Receiver clock Drift
        Units: meter per sec */
    float clockDrift;
    /** Receiver clock drift std deviation
        Units: Meter per sec */
    float clockDriftStdDeviation;
    /** GPS system time information. Check validity before using */
    epGnssSystemTimeStructType gpsSystemTime;
    /** GAL system time information. Check validity before using */
    epGnssSystemTimeStructType galSystemTime;
    /** BDS system time information. Check validity before using */
    epGnssSystemTimeStructType bdsSystemTime;
    /** QZSS system time information. Check validity before using */
    epGnssSystemTimeStructType qzssSystemTime;
    /** GLO system time information. Check validity before using */
    epGnssGloTimeStructType gloSystemTime;
    /** Validity mask */
    epGnssMeasurementReportValidity validityMask;
    /** GPS time leap second delta to UTC time. Check validity before using
        Units: sec */
    uint8_t leapSec;
    /** Number of Measurements in this report block.
        Mandatory field */
    uint8_t numMeas;
    /** Satellite measurement Information
        Mandatory field */
    epGnssSVMeasurementStructType svMeasurement[EP_GNSS_MAX_MEAS];
    /** Local time of last PPS pulse, associated with GNSS Measurement report
        Mandatory field */
    epGnssApTimeStampStructType lastPpsLocaltimeStamp;
    uint8_t reserve[128];
} epGnssMeasurementReport;


/** Below APIs shall be called by the plugged-in engine clients
    (Engine plug-in <-- Client Engine)
    APIs will run in the context of Engine Plug-in
*/

struct EPCallbacks {
    /**
      * @brief Report computed Position
      *
      * @param[in]  epPVTReport: Position Report.
      *
      * @return none.
      */
    void (* epReportPositionCb)(const epPVTReport *pvtReport);
    /**
      * @brief  Report engine status response.
      *
      * @param[in] status: Engine Status.
      *
      * @return none
      *
      */
    void (* epReportStatusRspCb)(uint32_t status);

    /**
      * @brief Log a message
      *
      * @param[in]  moduleID: Valid Module ID.
      * @param[in]  logLevel: Log level
      * @param[in]  logType:  Log Type
      * @param[in]  logLength: Log length
      * @param[in]  logMessage: Log message pointer.
      *
      * @return none.
      *
      */
    void (* epReportLogMessageCb)(uint32_t moduleID, epLogLevelType logLevel,
                     epLogType logType, uint32_t logLength, const uint8_t* logMessage);

    /**
      * @brief Report SV Status
      *
      * @param[in] epGnssSvNotification: SV Status
      *
      * @return none.
      *
      */
    void (* epReportSvStatusCb)(const epGnssSvNotification *svNotify);

    /**
      * @brief Subscribe to reports of interest.
      *        Engine plug-in sends error if engine plug-in is not supporting
      *        any of requested subscription.
      * @param[in] epSubscriptionInfo:  Subscription Mask Info
      *
      * @return none.
      *
      */
    void (* epRequestReportSubscription)(const epSubscriptionInfo *requestedSubscription);

    /**
      * @brief  API of query feature Status by Engine Client.
      *
      * @return none.
      *
      */
    void  (* epRequestFeatureStatusCb)(void);

    /**
      * @brief Request to create attestation statement with user data and nonce.
      *        Put user data and the nonce into the attestation statement and
      *        return to the client in epProvideAttestationStatement().
      *
      * @param[in] nonce: Transaction Info provided by the server.
      * @param[in] nonceLen: Length of Transaction Info provided by the server.
      * @param[in] data:  Application-defined data.
      * @param[in] lengthOfData: Length of the data buffer.
      *
      * @return none.
      *
      */
    void (* epRequestAttestationStatementCb) (uint8_t *nonce, size_t lengthOfNonce,
                   uint8_t *data, uint32_t lengthOfData);
    /**
      * @brief Request to Install License.
      *
      * @param[in] licenseBuffer: License Buffer.
      * @param[in] lengthOfLicenseBuffer: Length of the license buffer.
      *
      * @return none.
      *
      */
    void (* epRequestInstallLicenseCb) (uint8_t *licenseBuffer,
                   uint32_t lengthOfLicenseBuffer);

    uint8_t reserve[52];
};


/** Below APIs shall be called by Engine plug-in
    (Engine plug-in --> Client Engine)
    APIs will run in the context of called engine
*/
struct EPInterface {
    /** Provide Engine Plug-in interface header file
        Major version (EP_INTERFACE_FILE_MAJOR_VERSION) to engine Plug-in.
        Note: Using this information, Engine Plug-in detects of any major version
        mismatch. If mismatch is found, Engine Plug-in responds with an error as
        EP_ENGINE_PLUGIN_INTERFACE_FILE_VERSION_MISMATCH  */
    uint16_t  epInterfaceFileMajorVersion;

    /** Provide Engine Plug-in Interface file minor version
        (EP_INTERFACE_FILE_MINOR_VERSION) to engine Plug-in. */
    uint16_t  epInterfaceFileMinorVersion;

    /**
      * @brief Send Engine Plug-in Error.
      *
      * @param[in] epError: Data connection status
      *
      * @return none.
      */
    void  (* epSendEnginePluginError) (epError error);

    /**
     * @brief Request Version information of Engine.
     *        This is blocking Calling.
     * @param[out] engineVersion:  Version info Buffer
     * @param[in]  maxBuffSizeVer: Version info Max Size (128 bytes)
     * @param[out] engineId: Identification info Buffer
     * @param[in]  maxBuffSizeId: Identification info Max Size (128 bytes)
     *
     * @return bool: True: In case of success, False: Otherwise.
     *
     */
    bool  (* epRequestVersionInfo)(uint8_t *engineVersion, size_t maxBuffSizeVer,
                           uint8_t *engineId, size_t maxBuffSizeId);

    /**
     * @brief Session Control Command.
     *
     * @param[in] epCommandRequest: Session Control Command request
     *
     * @return bool: True: In case of success, False: Otherwise.
     */
    bool  (* epSessionContrlCommand)(const epCommandRequest *cmdParameters);

    /**
      * @brief Provide GNSS Ephemeris.
      *
      * @param[in] epGnssEphemerisReport: GNSS Ephemeris Info.
      *
      * @return none.
      */
    void  (* epProvideGnssEphemeris)(const epGnssEphemerisReport *ephemerisResport);

    /**
      * @brief Provide GNSS Measurement
      *
      * @param[in] epGnssMeasurementReport: Provide GNSS Measurement
      *
      * @return none.
      */
    void  (* epProvideGnssSvMeasurement)(const epGnssMeasurementReport *msrReport);

    /**
      * @brief Provide Position, Velocity, Time (PVT)
      *
      * @param[in] epPVTReport: Provide Position Velocity Time.
      *
      * @return none.
      *
      */
    void  (* epProvidePosition)(const epPVTReport *positionReport);

    /**
      * @brief Provide SV Polynomial.
      *
      * @param[in] epGnssSvPolynomial: Provide GNSS SV Polynomial Data.
      *
      * @return void.
      *
      */
    void (* epProvideSvPolynomial)(const epGnssSvPolynomial *svPolynomial);

    /**
      * @brief Provide Device Data Connection Status.
      *
      * @param[in] epConnectionStatus: Data connection status
      *
      * @return none.
      */
    void  (* epProvideConnectionStatus) (epConnectionStatus connStatus);

    /**
      * @brief Provide SV Info
      *
      * @param[in] epGnssSvNotification: SV Status Info
      *
      * @return none.
      *
      */
    void (* epProvideSvInfo)(const epGnssSvNotification *svNotify);
    /**
      * @brief Clean-up API
      *
      * @param[in] none
      *
      * @return none.
      *
      */
    void  (* cleanup)(void);

    /**
      * @brief API for Provide attestation statement
      *
      * @param[in] attestationStatement: Buffer of attestation statement
      * @param[in] sizeOfAttestationStatement: size of attestation statement
      * @param[in] status: Provide Status of Attestation Statement
      *
      * @return none.
      *
      */
    void  (* epProvideAttestationStatement)(uint8_t *attestationStatement,
               uint32_t sizeOfAttestationStatement, epAttestationStatus status);
    /**
      * @brief API for Provide status of Install License
      *
      * @param[in] status: License installation status.
      *
      * @return none.
      *
      */
     void (* epProvideInstallLicenseStatus)(epInstallLicenseStatus status);
    /**
      * @brief  API of Provides feature Status from Engine Client.
      *
      * @param[In] status: Licence status
      * @param[In] licenseType: Licence Type
      *
      * @return none.
      *
      */
      void  (* epProvideFeatureStatus)(epFeatureStatus status,
                                       epLicenseType licenseType);

    /**
      * API backward compatibility achieved using reserved array.
      * API versioning detailed below file header.
      *
      */
    uint8_t reserve[56];
};


/**
 * @brief Get Engine API Interface.
 *        This is blocking Call.
 *
 * @param[in] callbacks: Engine Callbacks.
 *
 * @return EPInterface: API Interface of Engine.
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const struct EPInterface* get_ep_interface(const struct EPCallbacks* callbacks);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /** ENGINE_PLUGIN_API_H */
