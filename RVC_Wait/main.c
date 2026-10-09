#include "rvc.h"

int main(void)
{
    Controller_Init();

    while (1) {               /* Iteration: Tick마다 Controller 반복 호출 */
        Controller();
        wait_ms(TICK_MS);
    }

    return 0;
}
