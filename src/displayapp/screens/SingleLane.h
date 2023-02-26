#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/Apps.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/widgets/StatusIcons.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SingleLane : public Screen {
      public:
        struct Applications {
          const char* icon;
          Pinetime::Applications::Apps application;
        };

        explicit SingleLane(DisplayApp* app,
                      Controllers::Settings& settingsController,
                      Controllers::Battery& batteryController,
                      Controllers::Ble& bleController,
                      Controllers::DateTime& dateTimeController);

        ~SingleLane() override;

        void UpdateScreen();
        void OnValueChangedEvent(lv_obj_t* obj, uint32_t buttonId);

      private:
        Controllers::DateTime& dateTimeController;

        lv_task_t* taskUpdate;

        lv_obj_t* label_time;
        lv_obj_t* btnm1;

        Widgets::PageIndicator pageIndicator;
        Widgets::StatusIcons statusIcons;

        const char* btnmMap[8];
        Pinetime::Applications::Apps apps[6];
      };
    }
  }
}
