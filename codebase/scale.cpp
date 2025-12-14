//
//    FILE: HX_kitchen_scale.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: HX711 demo
//     URL: https://github.com/RobTillaart/HX711


#include "HX711.h"
#include "scale.h"
#include "config.h"

HX711 scale;
float w1, w2, previous = 0;

void scale_init()
{
  scale.begin(SCALE_DATA_PIN, SCALE_CLK_PIN);
  scale.set_scale(SCALE_CALIBRATION_VAL);
  scale.tare();
}

float scale_read_delta()
{
    // read until stable
    float w1 = scale.get_units(10);
    delay(100);
    float w2 = scale.get_units();

    while (fabs(w1 - w2) > 10)
    {
        w1 = w2;
        w2 = scale.get_units();
        delay(100);
    }

    // If the scale is basically empty, don't report delta
    if (w1 < 100)
    {
        previous = 0;   // reset so next weight is correct
        return 0;       // or return NAN;
    }

    float delta = w1 - previous;
    previous = w1;
    return delta;
}

float scale_read_weight()
{
    float w1 = scale.get_units(10);
    delay(100);
    float w2 = scale.get_units();

    while (fabs(w1 - w2) > 10)
    {
        w1 = w2;
        w2 = scale.get_units();
        delay(100);
    }

    return w2;
}

//  -- END OF FILE --

