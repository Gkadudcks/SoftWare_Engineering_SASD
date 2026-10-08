#include "rvc.h"

/* ===== Library Modules ===== */

#include <stdio.h>
#include <stdlib.h>

/* Tick 번호는 0부터 시작한다. 네 센서가 모두 한 번씩 읽으면 다음 Tick으로 이동한다. */
static unsigned long sensor_tick = 0;
static unsigned int sensors_read_this_tick = 0;

bool Front_Sensor_Interface(void)
{
    FILE *fp = fopen("sensors.txt", "r");
    int front, left, right, dust;

    for (unsigned long i = 0; i <= sensor_tick; ++i) {
        if (fscanf(fp, "%d %d %d %d", &front, &left, &right, &dust) == EOF) {
            fclose(fp);
            puts("End of sensor data.");
            exit(EXIT_SUCCESS);
        }
    }
    fclose(fp);

    sensors_read_this_tick |= 1U;
    if (sensors_read_this_tick == 15U) {
        ++sensor_tick;
        sensors_read_this_tick = 0;
    }

    return front == 1;
}

bool Left_Sensor_Interface(void)
{
    FILE *fp = fopen("sensors.txt", "r");
    int front, left, right, dust;

    for (unsigned long i = 0; i <= sensor_tick; ++i) {
        if (fscanf(fp, "%d %d %d %d", &front, &left, &right, &dust) == EOF) {
            fclose(fp);
            puts("End of sensor data.");
            exit(EXIT_SUCCESS);
        }
    }
    fclose(fp);

    sensors_read_this_tick |= 2U;
    if (sensors_read_this_tick == 15U) {
        ++sensor_tick;
        sensors_read_this_tick = 0;
    }

    return left == 1;
}

bool Right_Sensor_Interface(void)
{
    FILE *fp = fopen("sensors.txt", "r");
    int front, left, right, dust;

    for (unsigned long i = 0; i <= sensor_tick; ++i) {
        if (fscanf(fp, "%d %d %d %d", &front, &left, &right, &dust) == EOF) {
            fclose(fp);
            puts("End of sensor data.");
            exit(EXIT_SUCCESS);
        }
    }
    fclose(fp);

    sensors_read_this_tick |= 4U;
    if (sensors_read_this_tick == 15U) {
        ++sensor_tick;
        sensors_read_this_tick = 0;
    }

    return right == 1;
}

bool Dust_Sensor_Interface(void)
{
    FILE *fp = fopen("sensors.txt", "r");
    int front, left, right, dust;

    for (unsigned long i = 0; i <= sensor_tick; ++i) {
        if (fscanf(fp, "%d %d %d %d", &front, &left, &right, &dust) == EOF) {
            fclose(fp);
            puts("End of sensor data.");
            exit(EXIT_SUCCESS);
        }
    }
    fclose(fp);

    sensors_read_this_tick |= 8U;
    if (sensors_read_this_tick == 15U) {
        ++sensor_tick;
        sensors_read_this_tick = 0;
    }

    return dust == 1;
}

void Motor_Interface(MotorCommand cmd)
{
    switch (cmd) {
    case MOTOR_FORWARD:  puts("Motor: Forward");  break;
    case MOTOR_BACKWARD: puts("Motor: Backward"); break;
    case MOTOR_LEFT:     puts("Motor: Left");     break;
    case MOTOR_RIGHT:    puts("Motor: Right");    break;
    }
}

void Cleaner_Interface(CleanerCommand cmd)
{
    switch (cmd) {
    case CLEANER_OFF:      puts("Cleaner: Off");      break;
    case CLEANER_ON:       puts("Cleaner: On");       break;
    case CLEANER_POWER_UP: puts("Cleaner: Power-Up"); break;
    }
}

/* ===== Afferent ===== */

ObstacleLocation Determine_Obstacle_Location(void)
{
    ObstacleLocation loc = { false, false, false };
    /* TODO 1.5: Front/Left/Right Sensor Interface 호출 -> Obstacle Location 구성 */
    return loc;
}

bool Determine_Dust_Existence(void)
{
    /* TODO 1.6: Dust Sensor Interface 호출 -> Controller로 전달 */
    return false;
}

/* ===== Transform Center ===== */

static RvcState state;      /* 현재 FSM 상태 (Tick 사이에도 유지) */
static int tick_count;      /* Turn Left/Right, Power Up의 Tick * 5 카운터 (상태 진입 시 0으로) */

void Controller_Init(void)
{
    /* TODO 전이 0: Enable "Move Forward", Cleaner command (On) */
    state = STATE_MOVE_FORWARD;
    tick_count = 0;
}

void Controller(void)
{
    ObstacleLocation loc = Determine_Obstacle_Location();
    bool dust = Determine_Dust_Existence();

    switch (state) {
    case STATE_MOVE_FORWARD:  state = Handle_Move_Forward(loc, dust);  break;
    case STATE_TURN_LEFT:     state = Handle_Turn_Left(loc, dust);     break;
    case STATE_TURN_RIGHT:    state = Handle_Turn_Right(loc, dust);    break;
    case STATE_STOP:          state = Handle_Stop(loc, dust);          break;
    case STATE_MOVE_BACKWARD: state = Handle_Move_Backward(loc, dust); break;
    case STATE_POWER_UP:      state = Handle_Power_Up(loc, dust);      break;
    }
}

/* ----- Controller 내부: 상태별 처리 ----- */

RvcState Handle_Move_Forward(ObstacleLocation loc, bool dust)
{
    /* TODO 매 Tick 장애물/먼지 판단:
            전이 1: [F && !L]       -> Disable Forward, Cleaner Off, Trigger Turn Left  -> TURN_LEFT
            전이 2: [F && L && !R]  -> Disable Forward, Cleaner Off, Trigger Turn Right -> TURN_RIGHT
            전이 3: [F && L && R]   -> Disable Forward, Cleaner Off                     -> STOP
            전이 9: [D && !F]       -> Cleaner Power Up                                 -> POWER_UP
            전이 없음                -> Move_Forward(SIG_ENABLE) 유지 (계속 전진)         -> MOVE_FORWARD */
    (void)loc; (void)dust;
    return STATE_MOVE_FORWARD;
}

RvcState Handle_Turn_Left(ObstacleLocation loc, bool dust)
{
    /* TODO Turn_Left(false)로 Tick 전달, tick_count 증가
            전이 7: Tick * 5 -> Enable Forward, Cleaner On -> MOVE_FORWARD */
    (void)loc; (void)dust;
    return STATE_TURN_LEFT;
}

RvcState Handle_Turn_Right(ObstacleLocation loc, bool dust)
{
    /* TODO Turn_Right(false)로 Tick 전달, tick_count 증가
            전이 8: Tick * 5 -> Enable Forward, Cleaner On -> MOVE_FORWARD */
    (void)loc; (void)dust;
    return STATE_TURN_RIGHT;
}

RvcState Handle_Stop(ObstacleLocation loc, bool dust)
{
    /* TODO 전이 4: Tick -> Enable Move Backward -> MOVE_BACKWARD */
    (void)loc; (void)dust;
    return STATE_STOP;
}

RvcState Handle_Move_Backward(ObstacleLocation loc, bool dust)
{
    /* TODO 매 Tick 장애물 판단:
            전이 5: [!L]       -> Disable Backward, Trigger Turn Left  -> TURN_LEFT
            전이 6: [L && !R]  -> Disable Backward, Trigger Turn Right -> TURN_RIGHT
            전이 없음 [L && R]  -> Move_Backward(SIG_ENABLE) 유지 (계속 후진) -> MOVE_BACKWARD */
    (void)loc; (void)dust;
    return STATE_MOVE_BACKWARD;
}

RvcState Handle_Power_Up(ObstacleLocation loc, bool dust)
{
    /* TODO 전이 1: [F && !L]       -> Disable Forward, Cleaner Off, Trigger Turn Left  -> TURN_LEFT
            전이 2: [F && L && !R]  -> Disable Forward, Cleaner Off, Trigger Turn Right -> TURN_RIGHT
            전이 3: [F && L && R]   -> Disable Forward, Cleaner Off                     -> STOP
            전이 10: Tick * 5 (tick_count) -> Cleaner On                              -> MOVE_FORWARD
            전이 없음                -> Move_Forward(SIG_ENABLE) 유지 (흡입 강화하며 전진) -> POWER_UP */
    (void)loc; (void)dust;
    return STATE_POWER_UP;
}

/* ===== Efferent ===== */

void Move_Forward(EnableSignal sig)
{
    /* TODO 2.1.2: Enable을 받는 매 Tick마다 Motor_Interface(MOTOR_FORWARD) 전송
                   Disable이면 전송 안 함 -> 모터 정지 */
    (void)sig;
}

void Turn_Left(bool trigger)
{
    /* TODO 2.1.3: Trigger 받으면 MOTOR_LEFT 전송, 5 Tick 세고 스스로 정지 */
    (void)trigger;
}

void Turn_Right(bool trigger)
{
    /* TODO 2.1.4: Trigger 받으면 MOTOR_RIGHT 전송, 5 Tick 세고 스스로 정지 */
    (void)trigger;
}

void Move_Backward(EnableSignal sig)
{
    /* TODO 2.1.5: Enable을 받는 매 Tick마다 Motor_Interface(MOTOR_BACKWARD) 전송
                   Disable이면 전송 안 함 -> 모터 정지 */
    (void)sig;
}

void Cleaner_Controller(CleanerCommand cmd)
{
    /* TODO 2.1.6: Cleaner_Interface(cmd) 호출 */
    (void)cmd;
}

/* ===== 유틸 ===== */

void wait_ms(int ms)
{
    /* TODO: 플랫폼별 sleep (Windows: Sleep(ms), POSIX: usleep(ms * 1000)) */
    (void)ms;
}