#ifndef APP_H_
#define APP_H_


#define TricklerState_t OpenTricklerStateFlag_e


typedef enum 
{
    UNDEFINED_STATE,
    MAIN_MENU,

    CLEANUP_MODE_MENU,
    CLEANUP_MODE_WAIT_FOR_COMPLETE,

    CALIBRATION_MODE_MENU,

    SCALE_SETUP_MODE_MENU,

    CHARGE_MODE_SELECT_WEIGHT,
    CHARGE_MODE_POWDER_THROW,
    CHARGE_MODE_POWDER_THROW_WAIT_FOR_COMPLETE,
    CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_COMPLETE,
    CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_REMOVAL,
    CHARGE_MODE_POWDER_TRICKLE_WAIT_FOR_CUP_RETURNED,
} OpenTricklerStateFlag_e;


typedef struct 
{
    TricklerState_t state;
    TricklerState_t (*func)(void);
} OpenTricklerState_t;


typedef enum 
{
    SCALE_HEADER_STABLE,
    SCALE_HEADER_UNSTABLE,
    SCALE_HEADER_OVERLOAD,
    SCALE_HEADER_STABLE_COUNTING,
} ScaleMeasurementHeader_e;

typedef enum
{
    SCALE_UNIT_GRAM,
    SCALE_UNIT_GRAIN,
} ScaleMesasurementUnit_e;

typedef struct {
    float measurement;
    ScaleMeasurementHeader_e header;
    ScaleMesasurementUnit_e unit; 
} ScaleMeasurement_t;

// Configs
extern const int cfg_thrower_microstepping;


#endif  // APP_H_