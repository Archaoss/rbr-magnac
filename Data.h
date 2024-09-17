// Agrège les données sur les dernières 15 min et 24 h.

#ifndef DATA_H
#define DATA_H

<<<<<<< HEAD
#include "Heure.h"
#include "Watts.h"
#include "WiFiSerial.h"

namespace Data
{
  // constexpr auto resolution = 60; // sec
  // constexpr auto max_data = 3600 * 24 / resolution; // bytes
  constexpr auto res2    = 2; // sec
  constexpr auto res180  = 180; // sec
  constexpr auto size2   = 440;
  constexpr auto size180 = 480;

  unsigned long last_boot; // UNIX epoch time

  float buf_p1_2[size2];
  float buf_p2_2[size2];
  float buf_p1_180[size180];
  float buf_p2_180[size180];
  std::size_t ix2   = 0;
  std::size_t ix180 = 0;

  void taskData2(void*)
  {
    for (;;)
    {

      buf_p1_2[ix2] = Watts::power1;
      buf_p2_2[ix2] = Watts::power2;
      ix2 = (ix2 + 1) % size2;

      weblogf("task data2 %u unused from 3000\n",
        uxTaskGetStackHighWaterMark(nullptr));
      delay(res2 * 1000);
    }
  }

  void taskData180(void*)
  {
    for (;;)
    {
      buf_p1_180[ix180] = Watts::power1;
      buf_p2_180[ix180] = Watts::power2;
      ix180 = (ix180 + 1) % size180;

      weblogf("task data180 %u unused from 3000\n",
        uxTaskGetStackHighWaterMark(nullptr));
      delay(res180 * 1000);
    }
  }

  void begin()
  {
    while (!Heure::time_client.isTimeSet())
      delay(100);
    
    last_boot = Heure::time_client.getEpochTime()
      - Heure::time_offset;

    xTaskCreate(taskData2, "task data2",
      3000, nullptr, 3, nullptr);
    xTaskCreate(taskData180, "task data180",
      3000, nullptr, 3, nullptr);
  }
=======
namespace Data
{
  inline constexpr auto res2    = 2; // sec
  inline constexpr auto res180  = 180; // sec
  inline constexpr auto size2   = 512;
  inline constexpr auto size180 = 512;

  extern unsigned long last_boot;

  extern float buf_p1_2[size2];
  extern float buf_p2_2[size2];
  extern float buf_p1_180[size180];
  extern float buf_p2_180[size180];

  extern unsigned ix2;
  extern unsigned ix180;

  void taskData2(void*);
  void taskData180(void*);
  void begin();
>>>>>>> withcpp
}

#endif /* DATA_H */
