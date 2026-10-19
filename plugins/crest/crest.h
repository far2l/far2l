#pragma once

#include <stdint.h>

typedef struct _CROptions{
  int   Enabled;
  int   Color;
  int   CenterColor;
  int   RulerColor;
  int   TempShow;
  int   LockShow;
  int   Flags;
  uint64_t   RgbColor;
  uint64_t   RgbCenterColor;
  uint64_t   RgbRulerColor;
} CROptions;

#ifdef __cplusplus
extern "C" {
#endif
extern void RestoreConfig(CROptions *Options);
extern void SaveConfig(const CROptions *Options);
#ifdef __cplusplus
} // extern "C"
#endif
