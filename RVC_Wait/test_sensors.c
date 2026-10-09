#include "rvc.h"
#include <stdio.h>

int main(void)
{
    for (int tick = 0; ; ++tick) {
        int front = Front_Sensor_Interface();
        int left  = Left_Sensor_Interface();
        int right = Right_Sensor_Interface();
        int dust  = Dust_Sensor_Interface();
        printf("Tick %d: F=%d L=%d R=%d D=%d\n",
               tick, front, left, right, dust);
    }
}
