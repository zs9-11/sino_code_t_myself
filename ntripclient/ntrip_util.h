#ifndef NTRIPLIB_NTRIP_UTIL_H_
#define NTRIPLIB_NTRIP_UTIL_H_

#include <string>


namespace libntrip {

constexpr char kCasterAgent[] = "NTRIP NTRIPCaster/20191018";
constexpr char kClientAgent[] = "NTRIP NTRIPClient/20191018";
constexpr char kServerAgent[] = "NTRIP NTRIPServer/20191018";

void PrintCharArray(const char *src, const int &len);
void PrintCharArrayHex(const char *src, const int &len);
int BccCheckSumCompareForGGA(const char *src);
int Base64Encode(const char *src, char *result);
int Base64Decode(const char *src, char *user, char *passwd);
int GetSourcetable(const char *path, char *data, const int &data_len);
int GetGGAFrameData(double const& latitude, double const& longitude,
                    double const& altitude, std::string* const gga_str);

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_UTIL_H_
