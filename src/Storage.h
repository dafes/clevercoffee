/**
 * @file Storage.h
 *
 * @brief
 *
 */

#pragma once

#include <stdint.h>

enum EditableKind {
    kInteger,
    kUInt8,
    kDouble,
    kDoubletime,
    kCString
};

struct editable_t {
    String displayName;
    boolean hasHelpText;
    String helpText;
    EditableKind type;
    int section;                   // parameter section number
    int position;
    std::function<bool()> show;    // method that determines if we show this parameter (in the web interface)
    int minValue;
    int maxValue;
};

// coffee_config structure
struct coffee_config {
    editable_t pidKpRegular;
    editable_t pidTnRegular;
    bool pidOn;
    editable_t pidTvRegular;
    editable_t pidIMaxRegular;
    editable_t brewSetpoint;
    editable_t brewTempOffset;
    editable_t brewTimeMs;
    editable_t preInfusionTimeMs;
    editable_t preInfusionPauseMs;
    bool pidBdOn;
    editable_t pidKpBd;
    editable_t pidTnBd;
    editable_t pidTvBd;
    editable_t brewSwTimeSec;
    editable_t brewPIDDelaySec;
    bool freeToUse10;
    editable_t brewDetectionThreshold;
    bool wifiCredentialsSaved;
    bool useStartPonM;
    editable_t pidKpStart;
    bool softApEnabledCheck;
    double pidTnStart;
    editable_t wifiSSID[25 + 1];
    editable_t wifiPassword[25 + 1];
    editable_t weightSetpoint;
    editable_t steamkp;
    editable_t steamSetpoint;
    bool standbyModeOn;
    editable_t standbyModeTime;
};


bool validateEEPROMData();
bool storageSetup(void);
int storageFactoryReset(void);
bool loadCoffeeConfig(coffee_config& config);
bool saveCoffeeConfig(const coffee_config& config);
bool SetDefault(void);
int storageCommit(void);

typedef enum
{
  STO_ITEM_PID_ON,                  // PID on/off state
  STO_ITEM_PID_START_PONM,          // Use PonM for cold start phase (otherwise use normal PID and same params)
  STO_ITEM_PID_KP_START,            // PID P part at cold start phase
  STO_ITEM_PID_TN_START,            // PID I part at cold start phase
  STO_ITEM_PID_KP_REGULAR,          // PID P part at regular operation
  STO_ITEM_PID_TN_REGULAR,          // PID I part at regular operation
  STO_ITEM_PID_TV_REGULAR,          // PID D part at regular operation
  STO_ITEM_PID_I_MAX_REGULAR,       // PID Integrator upper limit
  STO_ITEM_PID_KP_BD,               // PID P part at brew detection phase
  STO_ITEM_PID_TN_BD,               // PID I part at brew detection phase
  STO_ITEM_PID_TV_BD,               // PID D part at brew detection phase
  STO_ITEM_BREW_SETPOINT,           // brew setpoint
  STO_ITEM_BREW_TEMP_OFFSET,        // brew temp offset
  STO_ITEM_USE_BD_PID,              // use separate PID for brew detection (otherwise continue with regular PID)
  STO_ITEM_BREW_TIME,               // brew time
  STO_ITEM_BREW_SW_TIME,            // brew software time
  STO_ITEM_BREW_PID_DELAY,          // brew PID delay
  STO_ITEM_BD_THRESHOLD,            // brew detection limit
  STO_ITEM_WIFI_CREDENTIALS_SAVED,  // flag for wifisetup
  STO_ITEM_PRE_INFUSION_TIME,       // pre-infusion time
  STO_ITEM_PRE_INFUSION_PAUSE,      // pre-infusion pause
  STO_ITEM_PID_KP_STEAM,            // PID P part at steam phase
  STO_ITEM_STEAM_SETPOINT,          // Setpoint for Steam mode
  STO_ITEM_SOFT_AP_ENABLED_CHECK,   // soft AP enable state
  STO_ITEM_WIFI_SSID,               // Wifi SSID
  STO_ITEM_WIFI_PASSWORD,           // Wifi password
  STO_ITEM_WEIGHTSETPOINT,          // Brew weight setpoint
  STO_ITEM_STANDBY_MODE_ON,         // Enable tandby mode
  STO_ITEM_STANDBY_MODE_TIME,       // Time until heater is turned off
  STO_ITEM_RESERVED_30,             // reserved
  STO_ITEM_RESERVED_21,             // reserved

  /* WHEN ADDING NEW ITEMS, THE FOLLOWING HAS TO BE UPDATED:
   * - storage structure:  sto_data_t
   * - item default value: itemDefaults
   * - item address/size:  getItemAddr()
   */

  STO_ITEM__LAST_ENUM       // must be the last one!
} sto_item_id_t;

#if 0

// storage items
typedef enum
{
  STO_ITEM_PID_ON,                  // PID on/off state
  STO_ITEM_PID_START_PONM,          // Use PonM for cold start phase (otherwise use normal PID and same params)
  STO_ITEM_PID_KP_START,            // PID P part at cold start phase
  STO_ITEM_PID_TN_START,            // PID I part at cold start phase
  STO_ITEM_PID_KP_REGULAR,          // PID P part at regular operation
  STO_ITEM_PID_TN_REGULAR,          // PID I part at regular operation
  STO_ITEM_PID_TV_REGULAR,          // PID D part at regular operation
  STO_ITEM_PID_I_MAX_REGULAR,       // PID Integrator upper limit
  STO_ITEM_PID_KP_BD,               // PID P part at brew detection phase
  STO_ITEM_PID_TN_BD,               // PID I part at brew detection phase
  STO_ITEM_PID_TV_BD,               // PID D part at brew detection phase
  STO_ITEM_BREW_SETPOINT,           // brew setpoint
  STO_ITEM_BREW_TEMP_OFFSET,        // brew temp offset
  STO_ITEM_USE_BD_PID,              // use separate PID for brew detection (otherwise continue with regular PID)
  STO_ITEM_BREW_TIME,               // brew time
  STO_ITEM_BREW_SW_TIME,            // brew software time
  STO_ITEM_BREW_PID_DELAY,          // brew PID delay
  STO_ITEM_BD_THRESHOLD,            // brew detection limit
  STO_ITEM_WIFI_CREDENTIALS_SAVED,  // flag for wifisetup
  STO_ITEM_PRE_INFUSION_TIME,       // pre-infusion time
  STO_ITEM_PRE_INFUSION_PAUSE,      // pre-infusion pause
  STO_ITEM_PID_KP_STEAM,            // PID P part at steam phase
  STO_ITEM_STEAM_SETPOINT,          // Setpoint for Steam mode
  STO_ITEM_SOFT_AP_ENABLED_CHECK,   // soft AP enable state
  STO_ITEM_WIFI_SSID,               // Wifi SSID
  STO_ITEM_WIFI_PASSWORD,           // Wifi password
  STO_ITEM_WEIGHTSETPOINT,          // Brew weight setpoint
  STO_ITEM_STANDBY_MODE_ON,         // Enable tandby mode
  STO_ITEM_STANDBY_MODE_TIME,       // Time until heater is turned off
  STO_ITEM_RESERVED_30,             // reserved
  STO_ITEM_RESERVED_21,             // reserved

  /* WHEN ADDING NEW ITEMS, THE FOLLOWING HAS TO BE UPDATED:
   * - storage structure:  sto_data_t
   * - item default value: itemDefaults
   * - item address/size:  getItemAddr()
   */

  STO_ITEM__LAST_ENUM       // must be the last one!
} sto_item_id_t;

// Functions
bool storageSetup(void);

int storageCommit(void);
int storageFactoryReset(void);

int storageGet(sto_item_id_t itemId, float& itemValue);
int storageGet(sto_item_id_t itemId, double& itemValue);
int storageGet(sto_item_id_t itemId, int8_t& itemValue);
int storageGet(sto_item_id_t itemId, int16_t& itemValue);
int storageGet(sto_item_id_t itemId, int32_t& itemValue);
int storageGet(sto_item_id_t itemId, uint8_t& itemValue);
int storageGet(sto_item_id_t itemId, uint16_t& itemValue);
int storageGet(sto_item_id_t itemId, uint32_t& itemValue);
int storageGet(sto_item_id_t itemId, const char** itemValue);
int storageGet(sto_item_id_t itemId, String& itemValue);

int storageSet(sto_item_id_t itemId, float itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, double itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, int8_t itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, int16_t itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, int32_t itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, uint8_t itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, uint16_t itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, uint32_t itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, const char* itemValue, bool commit=false);
int storageSet(sto_item_id_t itemId, String& itemValue, bool commit=false);
#endif