/**
 * @file Storage.cpp
 *
 * @brief Provides functions to manage data in a non-volatile memory
 *
 */

#include <Arduino.h>
#include <EEPROM.h>

#include "debugSerial.h"
#include "Storage.h"
#include "defaults.h"

#include "ISR.h"

#include <ArduinoJson.h>

#define STRUCT_MEMBER_SIZE(Type, Member) sizeof(((Type*)0)->Member)
#define EEPROM_SIZE 4096
#define JSON_BUFFER_SIZE 512


/**
 * @brief Validates if there is a valid JSON object in EEPROM.
 *
 * @return true if a valid JSON object is found, false otherwise.
 */
bool validateEEPROMData() {
    // Read the first byte of EEPROM
    uint8_t firstByte = EEPROM.read(0);

    // Check if it's the start of a JSON object
    if (firstByte == '{') {
        // Try to deserialize the JSON object
        StaticJsonDocument<JSON_BUFFER_SIZE> jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, EEPROM);

        // Check if deserialization was successful
        if (!error) {
            // Valid JSON object found in EEPROM
            return true;
        }
    }

    // No valid JSON object found in EEPROM
    return false;
}


/**
 * @brief Sets up the storage module by initializing the EEPROM and checking for valid data.
 *
 * This function initializes the EEPROM with a specified size, defined by EEPROM_SIZE.
 * If the initialization fails, false is returned. If it succeeds, the function proceeds
 * to check if the EEPROM contains valid JSON data using the validateEEPROMData() function.
 * If the data is not valid, an error message is printed and the function performs a factory reset
 * by calling storageFactoryReset(). After the reset, the function sets default values by calling
 * SetDefault(). If the EEPROM contains valid data or if the defaults are successfully set, true is returned.
 *
 * @return  true if setup succeeds or valid data is present, false if it fails or invalid data is found.
 */
bool storageSetup(void) {

    if (!EEPROM.begin(EEPROM_SIZE)) {
        debugPrintf("%s(): EEPROM initialization failed!\n", __func__);
        return false;
    }
    // Check if Storage contains valid JSON:
    if(!validateEEPROMData()) {
        debugPrintf("EEPROM does not contains valid JSON!\n");
        debugPrintf("Setting defaults\n");
        // First reset EEPROM
        storageFactoryReset();
        return SetDefault();
    }
    return true;
}


/**
 * @brief Resets all storage data.
 *
 * @return  0 - succeed
 *         <0 - failed
 */
int storageFactoryReset(void) {
    debugPrintf("%s(): reset all values\n", __func__);
    memset(EEPROM.getDataPtr(), 0xFF, EEPROM_SIZE);
    return storageCommit();
}


/**
 * @brief Loads the coffee_config struct from EEPROM.
 *
 * @param config Reference to the coffee_config struct to populate.
 * @return true if successful, false otherwise.
*/
bool loadCoffeeConfig(coffee_config& config) {

    // No Valid Config found in EEPROM
    if(!validateEEPROMData()) {
        return SetDefault();
        

    }

    // Read the JSON string from EEPROM
    String jsonString;
    for (size_t i = 0; i < EEPROM_SIZE; i++) {
        char c = EEPROM.read(i);
        if (c == '\0') {
            break;
        }
        jsonString += c;
    }

    // Deserialize the JSON string into the coffee_config struct
    DynamicJsonDocument jsonDoc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(jsonDoc, jsonString);

    if (error) {
        // Failed to deserialize JSON
        return false;
    }

    // Extract values from JSON object
    config.pidKpRegular = jsonDoc["pidKpRegular"].as<double>();
    config.pidTnRegular = jsonDoc["pidTnRegular"].as<double>();
    config.pidOn = jsonDoc["pidOn"].as<bool>();
    config.pidTvRegular = jsonDoc["pidTvRegular"].as<double>();
    config.pidIMaxRegular = jsonDoc["pidIMaxRegular"].as<double>();
    config.brewSetpoint = jsonDoc["brewSetpoint"].as<double>();
    config.brewTempOffset = jsonDoc["brewTempOffset"].as<double>();
    config.brewTimeMs = jsonDoc["brewTimeMs"].as<double>();
    config.preInfusionTimeMs = jsonDoc["preInfusionTimeMs"].as<double>();
    config.preInfusionPauseMs = jsonDoc["preInfusionPauseMs"].as<double>();
    config.pidBdOn = jsonDoc["pidBdOn"].as<bool>();
    config.pidKpBd = jsonDoc["pidKpBd"].as<double>();
    config.pidTnBd = jsonDoc["pidTnBd"].as<double>();
    config.pidTvBd = jsonDoc["pidTvBd"].as<double>();
    config.brewSwTimeSec = jsonDoc["brewSwTimeSec"].as<double>();
    config.brewPIDDelaySec = jsonDoc["brewPIDDelaySec"].as<double>();
    config.freeToUse10 = jsonDoc["freeToUse10"].as<bool>();
    config.brewDetectionThreshold = jsonDoc["brewDetectionThreshold"].as<double>();
    config.wifiCredentialsSaved = jsonDoc["wifiCredentialsSaved"].as<bool>();
    config.useStartPonM = jsonDoc["useStartPonM"].as<bool>();
    config.pidKpStart = jsonDoc["pidKpStart"].as<double>();
    config.softApEnabledCheck = jsonDoc["softApEnabledCheck"].as<bool>();
    config.pidTnStart = jsonDoc["pidTnStart"].as<double>();
    strlcpy(config.wifiSSID, jsonDoc["wifiSSID"].as<char*>(), sizeof(config.wifiSSID));
    strlcpy(config.wifiPassword, jsonDoc["wifiPassword"].as<char*>(), sizeof(config.wifiPassword));
    config.weightSetpoint = jsonDoc["weightSetpoint"].as<double>();
    config.steamkp = jsonDoc["steamkp"].as<double>();
    config.steamSetpoint = jsonDoc["steamSetpoint"].as<double>();
    config.standbyModeOn = jsonDoc["standbyModeOn"].as<bool>();
    config.standbyModeTime = jsonDoc["standbyModeTime"].as<double>();

    return true;
}

/**
 * @brief Saves the coffee_config struct to EEPROM.
 *
 * @param config Reference to the coffee_config struct to save.
 * @return true if successful, false otherwise.
 */
bool saveCoffeeConfig(const coffee_config& config) {
    // Create a JSON object from the coffee_config struct
    DynamicJsonDocument jsonDoc(JSON_BUFFER_SIZE);
    jsonDoc["pidKpRegular"] = config.pidKpRegular;
    jsonDoc["pidTnRegular"] = config.pidTnRegular;
    jsonDoc["pidOn"] = config.pidOn;
    jsonDoc["pidTvRegular"] = config.pidTvRegular;
    jsonDoc["pidIMaxRegular"] = config.pidIMaxRegular;
    jsonDoc["brewSetpoint"] = config.brewSetpoint;
    jsonDoc["brewTempOffset"] = config.brewTempOffset;
    jsonDoc["brewTimeMs"] = config.brewTimeMs;
    jsonDoc["preInfusionTimeMs"] = config.preInfusionTimeMs;
    jsonDoc["preInfusionPauseMs"] = config.preInfusionPauseMs;
    jsonDoc["pidBdOn"] = config.pidBdOn;
    jsonDoc["pidKpBd"] = config.pidKpBd;
    jsonDoc["pidTnBd"] = config.pidTnBd;
    jsonDoc["pidTvBd"] = config.pidTvBd;
    jsonDoc["brewSwTimeSec"] = config.brewSwTimeSec;
    jsonDoc["brewPIDDelaySec"] = config.brewPIDDelaySec;
    jsonDoc["brewDetectionThreshold"] = config.brewDetectionThreshold;
    jsonDoc["wifiCredentialsSaved"] = config.wifiCredentialsSaved;
    jsonDoc["useStartPonM"] = config.useStartPonM;
    jsonDoc["pidKpStart"] = config.pidKpStart;
    jsonDoc["softApEnabledCheck"] = config.softApEnabledCheck;
    jsonDoc["pidTnStart"] = config.pidTnStart;
    jsonDoc["wifiSSID"] = config.wifiSSID;
    jsonDoc["wifiPassword"] = config.wifiPassword;
    jsonDoc["weightSetpoint"] = config.weightSetpoint;
    jsonDoc["steamkp"] = config.steamkp;
    jsonDoc["steamSetpoint"] = config.steamSetpoint;
    jsonDoc["standbyModeOn"] = config.standbyModeOn;
    jsonDoc["standbyModeTime"] = config.standbyModeTime;

    // Serialize the JSON object to a string
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    // Write the JSON string to EEPROM
    for (size_t i = 0; i < jsonString.length(); i++) {
        EEPROM.write(i, jsonString[i]);
    }

    // Null-terminate the string
    EEPROM.write(jsonString.length(), '\0');

    // Commit changes to EEPROM
    EEPROM.commit();

    return true;
}

bool SetDefault(void) {
    coffee_config config; 
    config.pidKpRegular = AGGKP;
    config.pidTnRegular = AGGTN;
    config.pidOn = false;
    config.pidTvRegular = AGGTV;
    config.pidIMaxRegular = AGGIMAX;
    config.brewSetpoint = SETPOINT;
    config.brewTempOffset = TEMPOFFSET;
    config.brewTimeMs = BREW_TIME;
    config.preInfusionTimeMs = PRE_INFUSION_TIME;
    config.preInfusionPauseMs = PRE_INFUSION_PAUSE_TIME;
    config.pidBdOn = false;
    config.pidKpBd = AGGBKP;
    config.pidTnBd = AGGBTN;
    config.pidTvBd = AGGBTV;
    config.brewSwTimeSec = BREW_SW_TIME;
    config.brewPIDDelaySec = BREW_PID_DELAY;
    config.brewDetectionThreshold = BD_SENSITIVITY;
    config.wifiCredentialsSaved = WIFI_CREDENTIALS_SAVED;
    config.useStartPonM = false;
    config.pidKpStart = STARTKP;
    config.softApEnabledCheck = false;
    config.pidTnStart = STARTTN;
    config.wifiSSID = "";
    config.wifiPassword = "";
    config.weightSetpoint = SCALE_WEIGHTSETPOINT;
    config.steamkp = STEAMKP;
    config.steamSetpoint = STEAMSETPOINT;
    config.standbyModeOn = false;
    config.standbyModeTime = STANDBY_MODE_TIME;

    return saveCoffeeConfig(config);
}

/**
 * @brief Writes all items to the storage medium.
 *
 * @return  0 - succeed
 *         <0 - failed
 */
int storageCommit(void) {
    debugPrintf("%s(): save all data to NV memory\n", __func__);

    //While Flash memory erase/write operations no other code must be executed from
    //Flash
    skipHeaterISR = true;

    // really write data to storage...
    int returnCode = EEPROM.commit() ? 0 : -1;

    skipHeaterISR = false;

    return returnCode;
}

// OLD FOO
#if 0
// set item defaults
static const sto_data_t itemDefaults PROGMEM = {
    AGGKP,                                    // STO_ITEM_PID_KP_REGULAR
    {0xFF, 0xFF},                             // reserved (maybe for structure version)
    AGGTN,                                    // STO_ITEM_PID_TN_REGULAR
    0,                                        // STO_ITEM_PID_ON
    0xFF,                                     // free to use
    AGGTV,                                    // STO_ITEM_PID_TV_REGULAR
    AGGIMAX,                                  // STO_ITEM_PID_I_MAX_REGULAR
    0xFF,                                     // free to use
    SETPOINT,                                 // STO_ITEM_BREW_SETPOINT
    TEMPOFFSET,                               // STO_ITEM_BREW_TEMP_OFFSET
    0xFF,                                     // free to use
    BREW_TIME,                                // STO_ITEM_BREW_TIME
    {0xFF, 0xFF},                             // free to use
    PRE_INFUSION_TIME,                        // STO_ITEM_PRE_INFUSION_TIME
    {0xFF, 0xFF},                             // free to use
    PRE_INFUSION_PAUSE_TIME,                  // STO_ITEM_PRE_INFUSION_PAUSE
    {   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF },       // free to use
    0,                                        // STO_ITEM_USE_PID_BD
    AGGBKP,                                   // STO_ITEM_PID_KP_BD
    {0xFF, 0xFF},                             // free to use
    AGGBTN,                                   // STO_ITEM_PID_TN_BD
    {0xFF, 0xFF},                             // free to use
    AGGBTV,                                   // STO_ITEM_PID_TV_BD
    {0xFF, 0xFF},                             // free to use
    BREW_SW_TIME,                             // STO_ITEM_BREW_SW_TIME
    BREW_PID_DELAY,                           // STO_ITEM_BREW_PID_DELAY
    0xFF,                                     // free to use
    BD_SENSITIVITY,                           // STO_ITEM_BD_THRESHOLD
    WIFI_CREDENTIALS_SAVED,                   // STO_ITEM_WIFI_CREDENTIALS_SAVED
    0,                                        // STO_ITEM_USE_START_PON_M
    STARTKP,                                  // STO_ITEM_PID_KP_START
    {0xFF, 0xFF},                             // free to use
    0,                                        // STO_ITEM_SOFT_AP_ENABLED_CHECK
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF},                  // free to use
    STARTTN,                                  // STO_ITEM_PID_TN_START
    {0xFF, 0xFF},                             // free to use
    "",                                       // STO_ITEM_WIFI_SSID
    "",                                       // STO_ITEM_WIFI_PASSWORD
    SCALE_WEIGHTSETPOINT,                     // STO_ITEM_WEIGHTSETPOINT
    STEAMKP,                                  // STO_ITEM_PID_KP_STEAM
    STEAMSETPOINT,                            // STO_ITEM_STEAM_SETPOINT
    STANDBY_MODE_ON,                          // STO_ITEM_STANDBY_MODE_ON
    STANDBY_MODE_TIME                         // STO_ITEM_STANDBY_MODE_TIME 
};

/**
 * @brief Returns the storage address of given item.
 *        Optionally the max. item storage size can be delivered.
 *
 * @param itemId      - storage item ID
 * @param maxItemSize - buffer for max. item storage size (optional, default=no buffer)
 *
 * @return >=0 - item storage address
 *          <0 - error
 */
static inline int32_t getItemAddr(sto_item_id_t itemId, uint16_t* maxItemSize = NULL) {
    int32_t addr;
    uint16_t size;

    switch (itemId) {
        case STO_ITEM_PID_KP_REGULAR:
            addr = offsetof(sto_data_t, pidKpRegular);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidKpRegular);
            break;

        case STO_ITEM_PID_TN_REGULAR:
            addr = offsetof(sto_data_t, pidTnRegular);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidTnRegular);
            break;

        case STO_ITEM_PID_TV_REGULAR:
            addr = offsetof(sto_data_t, pidTvRegular);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidTvRegular);
            break;

        case STO_ITEM_PID_I_MAX_REGULAR:
            addr = offsetof(sto_data_t, pidIMaxRegular);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidIMaxRegular);
            break;

        case STO_ITEM_BREW_SETPOINT:
            addr = offsetof(sto_data_t, brewSetpoint);
            size = STRUCT_MEMBER_SIZE(sto_data_t, brewSetpoint);
            break;

        case STO_ITEM_BREW_TEMP_OFFSET:
            addr = offsetof(sto_data_t, brewTempOffset);
            size = STRUCT_MEMBER_SIZE(sto_data_t, brewTempOffset);
            break;

        case STO_ITEM_BREW_TIME:
            addr = offsetof(sto_data_t, brewTimeMs);
            size = STRUCT_MEMBER_SIZE(sto_data_t, brewTimeMs);
            break;

        case STO_ITEM_PRE_INFUSION_TIME:
            addr = offsetof(sto_data_t, preInfusionTimeMs);
            size = STRUCT_MEMBER_SIZE(sto_data_t, preInfusionTimeMs);
            break;

        case STO_ITEM_PRE_INFUSION_PAUSE:
            addr = offsetof(sto_data_t, preInfusionPauseMs);
            size = STRUCT_MEMBER_SIZE(sto_data_t, preInfusionPauseMs);
            break;

        case STO_ITEM_BREW_PID_DELAY:
            addr = offsetof(sto_data_t, brewPIDDelaySec);
            size = STRUCT_MEMBER_SIZE(sto_data_t, brewPIDDelaySec);
            break;

        case STO_ITEM_USE_BD_PID:
            addr = offsetof(sto_data_t, pidBdOn);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidBdOn);
            break;

        case STO_ITEM_PID_KP_BD:
            addr = offsetof(sto_data_t, pidKpBd);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidKpBd);
            break;

        case STO_ITEM_PID_TN_BD:
            addr = offsetof(sto_data_t, pidTnBd);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidTnBd);
            break;

        case STO_ITEM_PID_TV_BD:
            addr = offsetof(sto_data_t, pidTvBd);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidTvBd);
            break;

        case STO_ITEM_BREW_SW_TIME:
            addr = offsetof(sto_data_t, brewSwTimeSec);
            size = STRUCT_MEMBER_SIZE(sto_data_t, brewSwTimeSec);
            break;

        case STO_ITEM_BD_THRESHOLD:
            addr = offsetof(sto_data_t, brewDetectionThreshold);
            size = STRUCT_MEMBER_SIZE(sto_data_t, brewDetectionThreshold);
            break;

        case STO_ITEM_WIFI_CREDENTIALS_SAVED:
            addr = offsetof(sto_data_t, wifiCredentialsSaved);
            size = STRUCT_MEMBER_SIZE(sto_data_t, wifiCredentialsSaved);
            break;

        case STO_ITEM_PID_START_PONM:
            addr = offsetof(sto_data_t, useStartPonM);
            size = STRUCT_MEMBER_SIZE(sto_data_t, useStartPonM);
            break;

        case STO_ITEM_PID_KP_START:
            addr = offsetof(sto_data_t, pidKpStart);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidKpStart);
            break;

        case STO_ITEM_PID_TN_START:
            addr = offsetof(sto_data_t, pidTnStart);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidTnStart);
            break;

        case STO_ITEM_SOFT_AP_ENABLED_CHECK:
            addr = offsetof(sto_data_t, softApEnabledCheck);
            size = STRUCT_MEMBER_SIZE(sto_data_t, softApEnabledCheck);
            break;

        case STO_ITEM_WIFI_SSID:
            addr = offsetof(sto_data_t, wifiSSID);
            size = STRUCT_MEMBER_SIZE(sto_data_t, wifiSSID);
            break;

        case STO_ITEM_WIFI_PASSWORD:
            addr = offsetof(sto_data_t, wifiPassword);
            size = STRUCT_MEMBER_SIZE(sto_data_t, wifiPassword);
            break;

        case STO_ITEM_PID_ON:
            addr = offsetof(sto_data_t, pidOn);
            size = STRUCT_MEMBER_SIZE(sto_data_t, pidOn);
            break;

        case STO_ITEM_PID_KP_STEAM:
            addr = offsetof(sto_data_t, steamkp);
            size = STRUCT_MEMBER_SIZE(sto_data_t, steamkp);
            break;

        case STO_ITEM_WEIGHTSETPOINT:
            addr = offsetof(sto_data_t, weightSetpoint );
            size = STRUCT_MEMBER_SIZE(sto_data_t,weightSetpoint);
            break;

        case STO_ITEM_STEAM_SETPOINT:
            addr = offsetof(sto_data_t, steamSetpoint);
            size = STRUCT_MEMBER_SIZE(sto_data_t,steamSetpoint);
            break;

        case STO_ITEM_STANDBY_MODE_ON:
            addr = offsetof(sto_data_t, standbyModeOn);
            size = STRUCT_MEMBER_SIZE(sto_data_t,standbyModeOn);
            break;

        case STO_ITEM_STANDBY_MODE_TIME:
            addr = offsetof(sto_data_t, standbyModeTime);
            size = STRUCT_MEMBER_SIZE(sto_data_t,standbyModeTime);
            break;

        default:
            debugPrintf("%s(): invalid item ID %i!\n", __func__, itemId);
            addr = -1;
            size = 0;
            break;
    }

    if (maxItemSize) *maxItemSize = size;

    return addr;
}

/**
 * @brief Checks if an item storage area is considered as "empty", which means
 *        no value was stored up to now.
 *
 * @param storageBuf     - item storage
 * @param storageBufSize - item storage size
 * @param emptyPattern   - empty pattern (optional, default=Flash erase value)
 *
 * @return true  - item storage is "empty" (no value was stored)
 *         false - item storage is not "empty" (value was stored)
 */
static inline bool isEmpty(const void* storageBuf, uint16_t storageBufSize, uint8_t emptyPattern = 0xFF) {
    uint16_t idx;

    for (idx = 0; idx < storageBufSize; idx++) { // check all item value bytes...
        if (((const uint8_t*)storageBuf)[idx] != emptyPattern)  // other than "empty" value?
            break;           // yes -> abort loop (item storage not empty)
    }

    return (idx == storageBufSize);
}

/**
 * @brief Checks if a buffer contains a C string null terminator.
 *
 * @param buf     - buffer to check
 * @param bufSize - buffer size
 *
 * @return true  - buffer contains a C string (string may be empty)
 *         false - buffer does not contain a C string
 */
static inline bool isString(const void* buf, uint16_t bufSize) {
    uint16_t idx;

    for (idx = 0; idx < bufSize; idx++) {     // check all buffer bytes...
        if (((const uint8_t*)buf)[idx] == 0)  // null terminator?
        return true;                          // yes -> string exist
    }

    return false;
}



/**
 * @brief Internal template function to read a number of any type.
 *
 * @param itemId    - storage item ID
 * @param itemValue - buffer for item value
 *
 * @return  0 - succeed
 *         <0 - failed
 */
template <typename T>
static inline int getNumber(sto_item_id_t itemId, T& itemValue) {
    uint16_t maxItemSize;
    int32_t itemAddr = getItemAddr(itemId, &maxItemSize);

    if (itemAddr < 0) {
        debugPrintf("%s(): invalid item address!\n", __func__);
        return -1;
    }

    debugPrintf("%s(): addr=%i size=%u/%u\n", __func__, itemAddr, sizeof(itemValue), maxItemSize);

    if (sizeof(itemValue) != maxItemSize) {
        debugPrintf("%s(): invalid item size (wrong data type)!\n", __func__);
        return -2;
    }

    EEPROM.get(itemAddr, itemValue);

    if (isEmpty(&itemValue, sizeof(itemValue))) { // item storage empty?
        debugPrintf("%s(): storage empty -> returning default\n", __func__);

        memcpy_P(&itemValue, (PGM_P)&itemDefaults + itemAddr, sizeof(itemValue));  // set default value
    }

    return 0;
}

/**
 * @brief Internal template function to set a number of any type.
 *        The value is set in the RAM only and still not written to the
 *        non-volatile memory!
 *
 * @param itemId    - storage item ID
 * @param itemValue - item value to set
 * @param commit    - true=write current data to medium (optional, default=false)
 *
 * @return  0 - succeed
 *         <0 - failed
 */
template <typename T>
static inline int setNumber(sto_item_id_t itemId, const T& itemValue, bool commit = false) {
    uint16_t maxItemSize;
    int32_t itemAddr = getItemAddr(itemId, &maxItemSize);

    if (itemAddr < 0) {
        debugPrintf("%s(): invalid item address!\n", __func__);
        return -1;
    }

    debugPrintf("%s(): addr=%i size=%u/%u commit=%u\n", __func__, itemAddr, sizeof(itemValue), maxItemSize, commit);

    if (sizeof(itemValue) != maxItemSize) {
        debugPrintf("%s(): invalid item size (wrong data type)!\n", __func__);
        return -2;
    }

    if (isEmpty(&itemValue, sizeof(T))) {
        debugPrintf("%s(): invalid item value!\n", __func__);
        return -3;
    }

    EEPROM.put(itemAddr, itemValue);

    if (commit) return storageCommit();

    return 0;
}

/**
 * @brief Reads the value of a storage item.
 *
 * @param itemId    - storage item ID
 * @param itemValue - buffer for item value
 *
 * @return  0 - succeed
 *         <0 - failed
 */
int storageGet(sto_item_id_t itemId, float& itemValue) {
    int retCode = getNumber(itemId, itemValue);

    if (retCode != 0) return retCode;

    debugPrintf("%s(): item=%i value=%.1f\n", __func__, itemId, itemValue);

    return 0;
}

int storageGet(sto_item_id_t itemId, double& itemValue) {
    int retCode = getNumber(itemId, itemValue);

    if (retCode != 0) return retCode;

    debugPrintf("%s(): item=%i value=%.1f\n", __func__, itemId, itemValue);

    return 0;
}

int storageGet(sto_item_id_t itemId, int8_t& itemValue) {
    int retCode = getNumber(itemId, itemValue);

    if (retCode != 0) return retCode;

    debugPrintf("%s(): item=%i value=%i\n", __func__, itemId, itemValue);

    return 0;
}

int storageGet(sto_item_id_t itemId, int16_t& itemValue) {
    int retCode = getNumber(itemId, itemValue);

    if (retCode != 0) return retCode;

    debugPrintf("%s(): item=%i value=%i\n", __func__, itemId, itemValue);

    return 0;
}

int storageGet(sto_item_id_t itemId, int32_t& itemValue) {
    int retCode = getNumber(itemId, itemValue);

    if (retCode != 0) return retCode;

    debugPrintf("%s(): item=%i value=%i\n", __func__, itemId, itemValue);

    return 0;
}

int storageGet(sto_item_id_t itemId, uint8_t& itemValue) {
    int retCode = getNumber(itemId, itemValue);

    if (retCode != 0) return retCode;

    debugPrintf("%s(): item=%i value=%u\n", __func__, itemId, itemValue);

    return 0;
}

int storageGet(sto_item_id_t itemId, uint16_t& itemValue) {
    int retCode = getNumber(itemId, itemValue);
    if (retCode != 0) return retCode;
    debugPrintf("%s(): item=%i value=%u\n", __func__, itemId, itemValue);

    return 0;
}

int storageGet(sto_item_id_t itemId, uint32_t& itemValue) {
    int retCode = getNumber(itemId, itemValue);

    if (retCode != 0) return retCode;

    debugPrintf("%s(): item=%i value=%u\n", __func__, itemId, itemValue);

    return 0;
}

int storageGet(sto_item_id_t itemId, String& itemValue) {
    uint16_t maxItemSize;
    int32_t itemAddr = getItemAddr(itemId, &maxItemSize);

    if (itemAddr < 0) {
        debugPrintf("%s(): invalid item address!\n", __func__);
        return -1;
    }


    // The ESP32 EEPROM class does not support getConstDataPtr()!
    uint8_t buf[maxItemSize];
    EEPROM.readBytes(itemAddr, buf, maxItemSize);

    if (isString(buf, maxItemSize)) { // exist a null terminator?
        itemValue = String((const char*)buf);
    } else {
        debugPrintf("%s(): storage empty -> returning default\n", __func__);
        itemValue = String((PGM_P)&itemDefaults + itemAddr);  // set default string
    }

    debugPrintf("%s(): addr=%i size=%u item=%i value=\"%s\"\n", __func__, itemAddr, itemValue.length() + 1, itemId, itemValue.c_str());

    return 0;
}

/**
 * @brief Sets a value of a storage item.
 *        The value is set in the RAM only! Use 'commit=true' or call
 *        storageCommit() to write the RAM content to the non-volatile memory!
*
*  @param itemId    - storage item ID
 * @param itemValue - item value to set
 * @param commit    - true=write current RAM content to NV memory (optional, default=false)
 *
 * @return  0 - succeed
 *         <0 - failed
 */
int storageSet(sto_item_id_t itemId, float itemValue, bool commit) {
    debugPrintf("%s(): item=%i value=%.1f\n", __func__, itemId, itemValue);
    return setNumber(itemId, itemValue, commit);
}

int storageSet(sto_item_id_t itemId, double itemValue, bool commit) {
    debugPrintf("%s(): item=%i value=%.1f\n", __func__, itemId, itemValue);
    return setNumber(itemId, itemValue, commit);
}

int storageSet(sto_item_id_t itemId, int8_t itemValue, bool commit) {
    debugPrintf("%s(): item=%i value=%i\n", __func__, itemId, itemValue);
    return setNumber(itemId, itemValue, commit);
}

int storageSet(sto_item_id_t itemId, int16_t itemValue, bool commit) {
    debugPrintf("%s(): item=%i value=%i\n", __func__, itemId, itemValue);
    return setNumber(itemId, itemValue, commit);
}

int storageSet(sto_item_id_t itemId, int32_t itemValue, bool commit) {
    debugPrintf("%s(): item=%i value=%i\n", __func__, itemId, itemValue);
    return setNumber(itemId, itemValue, commit);
}

int storageSet(sto_item_id_t itemId, uint8_t itemValue, bool commit) {
    debugPrintf("%s(): item=%i value=%u\n", __func__, itemId, itemValue);
    return setNumber(itemId, itemValue, commit);
}

int storageSet(sto_item_id_t itemId, uint16_t itemValue, bool commit) {
    debugPrintf("%s(): item=%i value=%u\n", __func__, itemId, itemValue);
    return setNumber(itemId, itemValue, commit);
}

int storageSet(sto_item_id_t itemId, uint32_t itemValue, bool commit) {
    debugPrintf("%s(): item=%i value=%u\n", __func__, itemId, itemValue);
    return setNumber(itemId, itemValue, commit);
}

int storageSet(sto_item_id_t itemId, const char* itemValue, bool commit) {
    uint16_t maxItemSize;
    size_t valueSize;
    int32_t itemAddr = getItemAddr(itemId, &maxItemSize);

    if (itemAddr < 0) {
        debugPrintf("%s(): invalid item address!\n", __func__);
        return -1;
    }

    valueSize = strlen(itemValue) + 1;
    debugPrintf("%s(): item=%i value=\"%s\" addr=%i size=%u/%u\n", __func__, itemId, itemValue, itemAddr, valueSize, maxItemSize);

    if (valueSize > maxItemSize) { // invalid value size?
        debugPrintf("%s(): string too large!\n", __func__);
        return -2;
    }

    memcpy(EEPROM.getDataPtr() + itemAddr, itemValue, valueSize);  // copy value to data structure in RAM

    if (commit) return storageCommit();

    return 0;
}

int storageSet(sto_item_id_t itemId, String& itemValue, bool commit) {
    return storageSet(itemId, itemValue.c_str(), commit);
}




#endif