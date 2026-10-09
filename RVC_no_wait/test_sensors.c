/* 선택 사항: 다른 TODO 모듈을 구현하기 전 Library Module만 확인하는 테스트 코드 */
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "rvc.h"
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
static uint64_t test_now_ms(void) { return (uint64_t)GetTickCount64(); }
#else
#include <time.h>
static uint64_t test_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}
#endif

int main(void)
{
    Controller_Init();
    uint64_t start = test_now_ms();
    uint64_t next_tick_ms = TICK_MS;

    for (;;) {
        uint64_t elapsed = test_now_ms() - start;
        bool front = Front_Sensor_Interface();

        if (elapsed >= next_tick_ms) {
            bool left = Left_Sensor_Interface(true);
            bool right = Right_Sensor_Interface(true);
            bool dust = Dust_Sensor_Interface(true);
            printf("[Tick %llu ms] F=%d L=%d R=%d D=%d\n",
                   (unsigned long long)next_tick_ms,
                   front, left, right, dust);
            next_tick_ms += TICK_MS;
        }
    }
}
