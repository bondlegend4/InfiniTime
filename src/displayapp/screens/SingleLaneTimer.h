#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include <FreeRTOS.h>
#include "portmacro_cmsis.h"

#include "systemtask/SystemTask.h"

namespace Pinetime::Applications::Screens {
  enum class SStates { Init, Running, Halted };

  struct STimeSeparated_t {
    int mins;
    int secs;
    int hundredths;
  };
  class SingleLaneTimer : public Screen {
  public:
    SingleLaneTimer(DisplayApp* app, System::SystemTask& systemTask);
    ~SingleLaneTimer() override;
    void Refresh() override;

    void playPauseBtnEventHandler(lv_event_t event);
    void stopLapBtnEventHandler(lv_event_t event);
    bool OnButtonPushed() override;

    void Reset();
    void Start();
    void Pause();

  private:
    Pinetime::System::SystemTask& systemTask;
    SStates currentState = SStates::Init;
    TickType_t startTime;
    TickType_t oldTimeElapsed = 0;
    static constexpr int maxLapCount = 20;
    TickType_t laps[maxLapCount + 1];
    static constexpr int displayedLaps = 2;
    int lapsDone = 0;
    lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap;
    lv_obj_t* lapText;

    lv_task_t* taskRefresh;
  };
}
