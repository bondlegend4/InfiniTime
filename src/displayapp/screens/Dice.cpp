#include "displayapp/screens/Dice.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  DTimeSeparated_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int hundredths = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = (timeElapsedCentis / 100) / 60;
    return DTimeSeparated_t {mins, secs, hundredths};
  }

  void play_pause_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* dice = static_cast<Dice*>(obj->user_data);
    dice->playPauseBtnEventHandler(event);
  }

  void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* dice = static_cast<Dice*>(obj->user_data);
    dice->stopLapBtnEventHandler(event);
  }
}

Dice::Dice(DisplayApp* app, System::SystemTask& systemTask) : Screen(app), systemTask {systemTask} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(time, "00:00");
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_text_font(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(msecTime, "00");
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 3);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_set_size(btnPlayPause, 115, 50);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text_static(txtPlayPause, Symbols::play);

  btnStopLap = lv_btn_create(lv_scr_act(), nullptr);
  btnStopLap->user_data = this;
  lv_obj_set_event_cb(btnStopLap, stop_lap_event_handler);
  lv_obj_set_size(btnStopLap, 115, 50);
  lv_obj_align(btnStopLap, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  txtStopLap = lv_label_create(btnStopLap, nullptr);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);

  lapText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lapText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(lapText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 30);
  lv_label_set_text_static(lapText, "");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Dice::~Dice() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void Dice::Reset() {
  currentState = DStates::Init;
  oldTimeElapsed = 0;
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(msecTime, "00");

  lv_label_set_text_static(lapText, "");
  lapsDone = 0;
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void Dice::Start() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  lv_label_set_text_static(txtPlayPause, Symbols::pause);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);
  startTime = xTaskGetTickCount();
  currentState = DStates::Running;
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void Dice::Pause() {
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed = laps[lapsDone];
  currentState = DStates::Halted;
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void Dice::Refresh() {
  if (currentState == DStates::Running) {
    laps[lapsDone] = oldTimeElapsed + xTaskGetTickCount() - startTime;

    DTimeSeparated_t currentTimeSeparated = convertTicksToTimeSegments(laps[lapsDone]);
    lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
    lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
  }
}

void Dice::playPauseBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (currentState == DStates::Init) {
    Start();
  } else if (currentState == DStates::Running) {
    Pause();
  } else if (currentState == DStates::Halted) {
    Start();
  }
}

void Dice::stopLapBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  // If running, then this button is used to save laps
  if (currentState == DStates::Running) {
    lv_label_set_text(lapText, "");
    lapsDone = std::min(lapsDone + 1, maxLapCount);
    for (int i = lapsDone - displayedLaps; i < lapsDone; i++) {
      if (i < 0) {
        lv_label_ins_text(lapText, LV_LABEL_POS_LAST, "\n");
        continue;
      }
      DTimeSeparated_t times = convertTicksToTimeSegments(laps[i]);
      char buffer[16];
      sprintf(buffer, "#%2d   %2d:%02d.%02d\n", i + 1, times.mins, times.secs, times.hundredths);
      lv_label_ins_text(lapText, LV_LABEL_POS_LAST, buffer);
    }
  } else if (currentState == DStates::Halted) {
    Reset();
  }
}

bool Dice::OnButtonPushed() {
  if (currentState == DStates::Running) {
    Pause();
    return true;
  }
  return false;
}
