#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Common.h"
//using namespace std;

//#include "Decode.h"
//#include "gnss_macro.h"
//#include "Polaris.h"


const double c = 299792458.0;
/*const int MAXCODE = 55;//max number of obs code*/

const double RANGE_MS = (c*0.001);
const double SC2RAD = 3.1415926535898;

const double P2_5 = 0.03125;             /* 2^-5 */
const double P2_6 = 0.015625;          /* 2^-6 */
const double P2_10 = 0.0009765625;
const double P2_11  = 4.882812500000000E-04; /* 2^-11 */
const double P2_15  = 3.051757812500000E-05; /* 2^-15 */
const double P2_17  = 7.629394531250000E-06; /* 2^-17 */
const double P2_19  = 1.907348632812500E-06; /* 2^-19 */
const double P2_20  = 9.536743164062500E-07; /* 2^-20 */
const double P2_21  = 4.768371582031250E-07; /* 2^-21 */
const double P2_23  = 1.192092895507810E-07; /* 2^-23 */
const double P2_24  = 5.960464477539063E-08; /* 2^-24 */
const double P2_27  = 7.450580596923828E-09; /* 2^-27 */
const double P2_29  = 1.862645149230957E-09; /* 2^-29 */
const double P2_30  = 9.313225746154785E-10; /* 2^-30 */
const double P2_31  = 4.656612873077393E-10; /* 2^-31 */
const double P2_32  = 2.328306436538696E-10; /* 2^-32 */
const double P2_33  = 1.164153218269348E-10; /* 2^-33 */
const double P2_34  = 5.820766091346741E-11;
const double P2_35  = 2.910383045673370E-11; /* 2^-35 */
const double P2_38  = 3.637978807091710E-12; /* 2^-38 */
const double P2_39  = 1.818989403545856E-12; /* 2^-39 */
const double P2_40  = 9.094947017729280E-13; /* 2^-40 */
const double P2_43  = 1.136868377216160E-13; /* 2^-43 */
const double P2_46  = 1.421085471520200E-14;
const double P2_48  = 3.552713678800501E-15; /* 2^-48 */
const double P2_50  = 8.881784197001252E-16; /* 2^-50 */
const double P2_55  = 2.775557561562891E-17; /* 2^-55 */
const double P2_59  = 1.734723475976807E-18;
const double P2_66  = 1.355252715606881E-20;


#define CODE_NONE   0                   /* obs code: none or unknown */
#define CODE_L1C    1                   /* obs code: L1C/A,G1C/A,E1C (GPS,GLO,GAL,QZS,SBS) */
#define CODE_L1P    2                   /* obs code: L1P,G1P    (GPS,GLO) */
#define CODE_L1W    3                   /* obs code: L1 Z-track (GPS) */
#define CODE_L1Y    4                   /* obs code: L1Y        (GPS) */
#define CODE_L1M    5                   /* obs code: L1M        (GPS) */
#define CODE_L1N    6                   /* obs code: L1codeless (GPS) */
#define CODE_L1S    7                   /* obs code: L1C(D)     (GPS,QZS) */
#define CODE_L1L    8                   /* obs code: L1C(P)     (GPS,QZS) */
#define CODE_L1E    9                   /* (not used) */
#define CODE_L1A    10                  /* obs code: E1A        (GAL) */
#define CODE_L1B    11                  /* obs code: E1B        (GAL) */
#define CODE_L1X    12                  /* obs code: E1B+C,L1C(D+P) (GAL,QZS) */
#define CODE_L1Z    13                  /* obs code: E1A+B+C,L1SAIF (GAL,QZS) */
#define CODE_L2C    14                  /* obs code: L2C/A,G1C/A (GPS,GLO) */
#define CODE_L2D    15                  /* obs code: L2 L1C/A-(P2-P1) (GPS) */
#define CODE_L2S    16                  /* obs code: L2C(M)     (GPS,QZS) */
#define CODE_L2L    17                  /* obs code: L2C(L)     (GPS,QZS) */
#define CODE_L2X    18                  /* obs code: L2C(M+L),B1I+Q (GPS,QZS,CMP) */
#define CODE_L2P    19                  /* obs code: L2P,G2P    (GPS,GLO) */
#define CODE_L2W    20                  /* obs code: L2 Z-track (GPS) */
#define CODE_L2Y    21                  /* obs code: L2Y        (GPS) */
#define CODE_L2M    22                  /* obs code: L2M        (GPS) */
#define CODE_L2N    23                  /* obs code: L2codeless (GPS) */
#define CODE_L5I    24                  /* obs code: L5/E5aI    (GPS,GAL,QZS,SBS) */
#define CODE_L5Q    25                  /* obs code: L5/E5aQ    (GPS,GAL,QZS,SBS) */
#define CODE_L5X    26                  /* obs code: L5/E5aI+Q/L5B+C (GPS,GAL,QZS,IRN,SBS) */
#define CODE_L7I    27                  /* obs code: E5bI,B2I   (GAL,CMP) */
#define CODE_L7Q    28                  /* obs code: E5bQ,B2Q   (GAL,CMP) */
#define CODE_L7X    29                  /* obs code: E5bI+Q,B2I+Q (GAL,CMP) */
#define CODE_L6A    30                  /* obs code: E6A        (GAL) */
#define CODE_L6B    31                  /* obs code: E6B        (GAL) */
#define CODE_L6C    32                  /* obs code: E6C        (GAL) */
#define CODE_L6X    33                  /* obs code: E6B+C,LEXS+L,B3I+Q (GAL,QZS,CMP) */
#define CODE_L6Z    34                  /* obs code: E6A+B+C    (GAL) */
#define CODE_L6S    35                  /* obs code: LEXS       (QZS) */
#define CODE_L6L    36                  /* obs code: LEXL       (QZS) */
#define CODE_L8I    37                  /* obs code: E5(a+b)I   (GAL) */
#define CODE_L8Q    38                  /* obs code: E5(a+b)Q   (GAL) */
#define CODE_L8X    39                  /* obs code: E5(a+b)I+Q (GAL) */
#define CODE_L2I    40                  /* obs code: B1I        (BDS) */
#define CODE_L2Q    41                  /* obs code: B1Q        (BDS) */
#define CODE_L6I    42                  /* obs code: B3I        (BDS) */
#define CODE_L6Q    43                  /* obs code: B3Q        (BDS) */
#define CODE_L3I    44                  /* obs code: G3I        (GLO) */
#define CODE_L3Q    45                  /* obs code: G3Q        (GLO) */
#define CODE_L3X    46                  /* obs code: G3I+Q      (GLO) */
#define CODE_L1I    47                  /* obs code: B1I        (BDS) */
#define CODE_L1Q    48                  /* obs code: B1Q        (BDS) */
#define CODE_L5A    49                  /* obs code: L5A SPS    (IRN) */
#define CODE_L5B    50                  /* obs code: L5B RS(D)  (IRN) */
#define CODE_L5C    51                  /* obs code: L5C RS(P)  (IRN) */
#define CODE_L9A    52                  /* obs code: SA SPS     (IRN) */
#define CODE_L9B    53                  /* obs code: SB RS(D)   (IRN) */
#define CODE_L9C    54                  /* obs code: SC RS(P)   (IRN) */
#define CODE_L9X    55                  /* obs code: SB+C       (IRN) */
#define MAXCODE     55                  /* max number of obs code */


#define GPS  1
#define GLO  2
#define GAL  3
#define BDS  4
#define QZSS 5

const unsigned int MessageTypeNo = 4500;

typedef struct {                    /* multi-signal-message header type */
	unsigned char iod;              /* issue of data station */
	unsigned char time_s;           /* cumulative session transmitting time */
	unsigned char clk_str;          /* clock steering indicator */
	unsigned char clk_ext;          /* external clock indicator */
	unsigned char smooth;           /* divergence free smoothing indicator */
	unsigned char tint_s;           /* soothing interval */
	unsigned char nsat,nsig;        /* number of satellites/signals */
	unsigned char sats[64];         /* satellites */
	unsigned char sigs[32];         /* signals */
	unsigned char cellmask[64];     /* cell mask */
	unsigned char satmask[64];
	unsigned char sigmask[64];
} msm_h_t;

typedef struct {                   
	msm_h_t h;
	double r[64],pr[64],cp[64],cnr[64],r_m[64],tow;
	int i,j,type,sync,iod,ncell,rng,rng_m,prv,cpv,lock[64],half[64],RfR[64],RfRm[64],PRs[64],PhsR[64];
	const char *sig[32];
} msm_b_t;
const int MSM_B_SIZE = sizeof(msm_b_t);

extern unsigned int getbitu(const unsigned char *buff, int pos, int len);
extern int getbits(const unsigned char *buff, int pos, int len);

 unsigned int decode_type1005(FILE *m_fp1005, unsigned char *buff, unsigned int Meslen, Coordinate* pCoord);
 unsigned int decode_type1019(FILE *m_fp1019, unsigned char *buff, unsigned int Meslen, ServerEphemeris* pEphem);
 unsigned int decode_type1020(FILE *m_fp1020, unsigned char *buff, unsigned int Meslen, ServerGloEphemeris* pEphem, int nLeaps = LEAP_SECONDS);
 unsigned int decode_type1033(FILE *m_fp1033, unsigned char *buff, unsigned int Meslen);
 unsigned int decode_type1042(FILE *m_fp1042, unsigned char *buff, unsigned int Meslen, ServerEphemeris* pEphem);
 unsigned int decode_type1045(FILE *m_fp1045, unsigned char *buff, unsigned int Meslen);
 //unsigned int decode_type1046(FILE *m_fp1046, unsigned char *buff, unsigned int Meslen);
 unsigned int decode_type1046(FILE *m_fp1046, unsigned char *buff, unsigned int Meslen, ServerEphemeris* pEphem);
 int decode_msm_head(FILE *m_fp, unsigned char *buff, int sys, int *sync, int *iod, msm_h_t *h, int *hsize, double * tow, int nLeaps = LEAP_SECONDS);
 int decode_msm1(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys);
 int decode_msm2(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys);
 int decode_msm3(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys);
 int decode_msm4(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys, WangEpoch *pEpoch, int nLeaps = LEAP_SECONDS);
 int decode_msm5(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys);
 int decode_msm6(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys);
 int decode_msm7(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys, WangEpoch *pEpoch, int nLeaps = LEAP_SECONDS);
// int decode_msm7(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys, WangEpoch *pEpoch,msm_b_t *pTemp, int nLeaps = LEAP_SECONDS);

 int iInt(const double d);
 int iYMDtoSumDay(int _iYMD);
 void vGetGlonassUTC(int Tb, int M_Nt, int M_N4, int UTC[]);
 bool bFindObsIndex(WangEpoch *pEpoch, int id, const char* strShort, int& iIndex, int& iType);

extern unsigned int crc24q(const unsigned char *buff, int len);


