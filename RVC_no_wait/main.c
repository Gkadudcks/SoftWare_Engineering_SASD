/*
 * main.c - Main 모듈 (Structured Chart 최상위)
 */
#include "rvc.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("=== RVC FSM TEST: %s ===\n", argc > 1 ? argv[1] : "MANUAL");
    Controller_Init();

    while (1) {               /* Iteration: Controller 반복 호출 (Tick 계산은 Controller 안에서) */
        Controller();
    }

    return 0;
}
