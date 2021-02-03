/*----------------------------------------------------------------------------
  Copyright (c) 2020-2020 SinoGNSS Technologies, Inc.
  All Rights Reserved.
  Designed by ZHANG Mingkai.
  Date in July. 2020.
----------------------------------------------------------------------------*/
#pragma once
#include <math.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include "Common.h"
#include "EnginePluginAPI.h"
//#include "EnginePluginLog.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN_DLL
#define EXPORT __declspec(dllexport) /* for Windows DLL */
#else
#define EXPORT
#endif

/* common value */
//#define PI          3.1415926535897932  /* pi */
#define D2R         (PI/180.0)          /* deg to rad */
#define R2D         (180.0/PI)          /* rad to deg */
#define MAXLEAPS    64                  /* max number of leap seconds table */
#define CLIGHT      299792458.0         /* speed of light (m/s) */
#define RE_WGS84    6378137.0           /* earth semimajor axis (WGS84) (m) */
#define FE_WGS84    (1.0/298.257223563) /* earth flattening (WGS84) */

/* all gnss signal number of qualcomm */
#define SIGNA               0           /* index number of none */
#define SIGL1               1           /* index number of L1 */
#define SIGL2               2           /* index number of L2 */
#define SIGL3               3           /* index number of L3 */

/* all gnss system index number of qualcomm */
#define IDNAN               0           /* index number of none */
#define IDGPS               1           /* index number of gps */
#define IDGLO               2           /* index number of glonass */
#define IDSBS               3           /* index number of sbas */
#define IDQZS               4           /* index number of qzss-L1CA */
#define IDBDS               5           /* index number of bds */
#define IDGAL               6           /* index number of galileo */

#define SYS_NONE    0x00                /* navigation system: none */
#define SYS_GPS     0x01                /* navigation system: GPS */
#define SYS_SBS     0x02                /* navigation system: SBAS */
#define SYS_GLO     0x04                /* navigation system: GLONASS */
#define SYS_GAL     0x08                /* navigation system: Galileo */
#define SYS_QZS     0x10                /* navigation system: QZSS */
#define SYS_BDS     0x20                /* navigation system: BeiDou */
#define SYS_IRN     0x40                /* navigation system: IRNS */
#define SYS_LEO     0x80                /* navigation system: LEO */
#define SYS_ALL     0xFF                /* navigation system: all */

/* all gnss satellite number of qualcomm */
#define MINSATGPS           1           /* mininum sat number start of gps */
#define MAXSATGPS          32           /* maxinum sat number start of gps */
#define MINSATGLO          65           /* mininum sat number start of glonass */
#define MAXSATGLO          96           /* maxinum sat number start of glonass */
#define MINSATSBS         120           /* mininum sat number start of sbas */
#define MAXSATSBS         151           /* maxinum sat number start of sbas */
//#define MINSATSBS         183           /* mininum sat number start of sbas */
//#define MAXSATSBS         192           /* maxinum sat number start of sbas */
#define MINSATQZS         193           /* mininum sat number start of qzss-L1CA */
#define MAXSATQZS         197           /* maxinum sat number start of qzss-L1CA */
#define MINSATBDS         201           /* mininum sat number start of bds */
#define MAXSATBDS         237           /* maxinum sat number start of bds */
#define MINSATGAL         301           /* mininum sat number start of galileo */
#define MAXSATGAL         336           /* maxinum sat number start of galileo */

/* all gnss satellite number of sino */
#define MIN_SAT_GPS         1           /* mininum sat number start of gps */
#define MAX_SAT_GPS        37           /* maxinum sat number start of gps */
#define MIN_SAT_GLO        38           /* mininum sat number start of glonass */
#define MAX_SAT_GLO        61           /* maxinum sat number start of glonass */
#define MIN_SAT_GAL        71           /* mininum sat number start of galileo */
#define MAX_SAT_GAL       106           /* maxinum sat number start of galileo */
#define MIN_SAT_BDS       141           /* mininum sat number start of bds */
#define MAX_SAT_BDS       203           /* maxinum sat number start of bds */
#define MAXSATS           204           /* total sat number of all */
    
//#define NSATGPS     (MAXPRNGPS-MINPRNGPS+1)    /* sat number of gps */
//#define NSATGLO     (MAXPRNGLO-MINPRNGLO+1)    /* sat number of glo */
//#define NSATGAL     (MAXPRNGAL-MINPRNGAL+1)    /* sat number of gal */
//#define NSATBDS     (MAXPRNBDS-MINPRNBDS+1)    /* sat number of bds */
//#define MAXSATS     (NSATGPS+NSATGLO+NSATGAL+NSATBDS) /* total sat number of all */
//#define MAXSATS           204           /* total sat number of all */

#define NFREQ       3                   /* max number of obs frequency */

#define FREQ1       1.57542E9           /* L1/E1  frequency (Hz) */
#define FREQ2       1.22760E9           /* L2     frequency (Hz) */
#define FREQ5       1.17645E9           /* L5/E5a frequency (Hz) */
#define FREQ6       1.27875E9           /* E6/LEX frequency (Hz) */
#define FREQ7       1.20714E9           /* E5b    frequency (Hz) */
#define FREQ8       1.191795E9          /* E5a+b  frequency (Hz) */
#define FREQ9       2.492028E9          /* S      frequency (Hz) */
#define FREQ1_GLO   1.60200E9           /* GLONASS G1 base frequency (Hz) */
#define DFRQ1_GLO   0.56250E6           /* GLONASS G1 bias frequency (Hz/n) */
#define FREQ2_GLO   1.24600E9           /* GLONASS G2 base frequency (Hz) */
#define DFRQ2_GLO   0.43750E6           /* GLONASS G2 bias frequency (Hz/n) */
#define FREQ3_GLO   1.202025E9          /* GLONASS G3 frequency (Hz) */
#define FREQ1_BDS   1.561098E9          /* BeiDou B1 frequency (Hz) */
#define FREQ2_BDS   1.20714E9           /* BeiDou B2 frequency (Hz) */
#define FREQ3_BDS   1.26852E9           /* BeiDou B3 frequency (Hz) */

/* solution status: no solution */
#define SOLQ_NONE   0                   /* solution status: no solution */
#define SOLQ_FIX    1                   /* solution status: fix */
#define SOLQ_FLOAT  2                   /* solution status: float */
#define SOLQ_SBAS   3                   /* solution status: SBAS */
#define SOLQ_DGPS   4                   /* solution status: DGPS/DGNSS */
#define SOLQ_SINGLE 5                   /* solution status: single */
#define SOLQ_PPP    6                   /* solution status: PPP */
#define SOLQ_DR     7                   /* solution status: dead reconing */
#define MAXSOLQ     7                   /* max number of solution status */

/* struct define */
typedef struct {        /* ppe struct */
    int init;           /* mark if has been initialized (0:no,1:yes) */
    double lam[MAXSATS][NFREQ]; /* carrier wave lengths (m) */
    double phaseb[MAXSATS][NFREQ]; /* carrier cycle shift for approximately equal to psuedorange (cycle) */
    char gpggabuf[200]; /* gpgga message */
    WangEpoch wobs;     /* WangEpoch obss */
} ppe_t;

typedef struct {
    uint16_t sys;

    union {
        ServerEphemeris ephggb[40];
        ServerGloEphemeris ephglo[30];
    } ephInfo;
}ephcom_t;

typedef struct {        /* ppe struct */
    double dsmooth_R[MAXSATS];
    double dlast_L1[MAXSATS];
    UINT32 smoothcount[MAXSATS];
    UINT32 lli[MAXSATS];
} smooth_t;


/* global variables ----------------------------------------------------------*/
extern ppe_t ppes;             /* default positioning options */
extern smooth_t m_smooth;

/* function define */
EXPORT void syschr(BYTE sysid, char *id);
EXPORT BYTE init_ppe(ppe_t *ppe);
//EXPORT BYTE ephqumcom(const epGnssEphemerisReport *ephemerisResport, ephcom_t *ephs);
EXPORT BYTE ppeephcvt(const epGnssEphemerisReport *ephemerisResport, ephcom_t *ephs);
EXPORT BYTE obsqumcom(const epGnssMeasurementReport *msrReport, ppe_t *ppe);
//EXPORT BYTE epposout_gga(const epPVTReport *posrep, ppe_t *ppe);
EXPORT BYTE epposout_gga(const epPVTReport *posrep, char *gpggabuf);

//EXPORT BYTE rtk_report(const RTK_result *rtkres, epPVTReport *posrep);
//EXPORT BYTE ppeout_rtk(const RTK_result *rtkres, epPVTReport *posrep);
EXPORT BYTE ppeout_rtk(const RTK_result *rtkres, int week, epPVTReport *posrep);
EXPORT BYTE ppeout_gga(const RTK_result *rtkres, int week, char *buffer);
EXPORT BYTE ppeout_rmc(const RTK_result *rtkres, int week, char *buffer);

/* debug print ep */
//EXPORT BYTE ephprintep(char* buffer, const epGnssEphemerisReport* ephrep);
EXPORT uint32_t ephprintep(char* buffer, const int buflen, const epGnssEphemerisReport *ephemerisResport);
EXPORT uint32_t obsprintep(char* buffer, BYTE sysid, const epGnssMeasurementReport* obsrep);

/* debug print ppe */
EXPORT uint32_t obsprintppe(char* buffer, const WangEpoch* obsrep);

#ifdef __cplusplus
}
#endif /* ppecom.h */
