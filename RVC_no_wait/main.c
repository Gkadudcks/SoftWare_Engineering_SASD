/*
 * main.c - Main 모듈 (Structured Chart 최상위)
 */
#include "rvc.h"

int main(void)
{
    Controller_Init();

    while (1) {               /* Iteration: Controller 반복 호출 (Tick 계산은 Controller 안에서) */
        Controller();
    }

    return 0;
}
