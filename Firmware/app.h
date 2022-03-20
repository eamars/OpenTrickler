#ifndef APP_H_
#define APP_H_


#define TricklerState_t OpenTricklerStateFlag_e


typedef enum 
{
    UNDEFINED_STATE,
    MAIN_MENU,

    CLEANUP_TRICKLER,
    CLEANUP_TRICKLER_WAIT_FOR_COMPLETE,

    CLEANUP_THROWER,
    CLEANUP_THROWER_WAIT_FOR_COMPLETE,

    CALIBRATION_MODE_MENU,
    CALIBRATION_MODE_TUNE_FINE_TRICKLER,
    CALIBRATION_MODE_TUNE_COARSE_TRICKLER,

    SCALE_SETUP_MODE_MENU,

    CHARGE_MODE_SELECT_WEIGHT,
    CHARGE_MODE_POWDER_THROW,
    CHARGE_MODE_POWDER_THROW_WAIT_FOR_COMPLETE,
    CHARGE_MODE_ZERO_SCALE,
    CHARGE_MODE_COARSE_TRICKLE,
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

typedef enum
{
    COARSE_MODE_DIABLED,
    COARSE_MODE_USE_POWDER_MEASURE,
    COARSE_MODE_USE_COARSE_TRICKLER,
} CoarseMode_e;


typedef enum
{
    FINE_MODE_DIABLED,
    FINE_MODE_USE_FINE_TRICKLER,
    FINE_MODE_USE_VIBRATORY_TRICKLER,
} FineMode_e;


typedef union {
    struct __attribute__((__packed__)){
        char header[2];
        char comma;
        char data[9];
        char unit[3];
        char terminator[2];
    };
    char packet[17];
} ScaleStandardOutputDataFormat_t;


#endif  // APP_H_