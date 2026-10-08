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
/*
 * 공통 규칙
 * [v1 구조] main이 wait_ms(TICK_MS)로 Tick을 만들고 Controller를 Tick마다 1회 호출하므로,
 *           Efferent 모듈이 한 번 호출되는 것 = 1 Tick 이다.
 *  - Move Forward / Move Backward : Enable/Disable 제어 신호를 받는다.
 *      Controller가 유지하고 싶은 Tick마다 SIG_ENABLE로 호출하고,
 *      SIG_DISABLE이면 명령을 보내지 않는다 -> 모터 정지.
 *  - Turn Left / Turn Right : Trigger로 시작해서 TURN_TICKS(5) Tick 동안 회전 명령을 보내고
 *      스스로 정지한다. 회전이 끝난 뒤의 호출(trigger=false)은 아무것도 하지 않는다.
 *      -> Trigger를 받은 Tick이 1번째 회전 Tick이다.
 *         Handle_Turn_Left/Right가 Turn_xxx(false)를 부르고 tick_count를 올려서
 *         tick_count == 5가 되는 Tick에는 회전이 이미 끝나 있으므로,
 *         그 Tick에 바로 Move_Forward(SIG_ENABLE)를 불러도 겹치지 않는다.
 */

static int turn_left_remaining  = 0;   /* Turn Left 남은 회전 Tick 수 (0 = 정지 상태) */
static int turn_right_remaining = 0;   /* Turn Right 남은 회전 Tick 수 (0 = 정지 상태) */

void Move_Forward(EnableSignal sig)
{
    /* 2.1.2: Enable인 Tick에만 Motor_Interface(MOTOR_FORWARD) 전송
              Disable이면 전송 안 함 -> 모터 정지 */
    if (sig == SIG_ENABLE)
        Motor_Interface(MOTOR_FORWARD);
}

void Turn_Left(bool trigger)
{
    /* 2.1.3: Trigger 받으면 회전 시작 (이 Tick이 1번째),
              회전 중인 동안 매 Tick MOTOR_LEFT 전송, TURN_TICKS Tick 후 스스로 정지 */
    if (trigger) {
        turn_left_remaining  = TURN_TICKS;
        turn_right_remaining = 0;          /* 반대 방향 회전이 남아 있었다면 취소 */
    }

    if (turn_left_remaining > 0) {
        Motor_Interface(MOTOR_LEFT);
        turn_left_remaining--;
    }
}

void Turn_Right(bool trigger)
{
    /* 2.1.4: Trigger 받으면 회전 시작 (이 Tick이 1번째),
              회전 중인 동안 매 Tick MOTOR_RIGHT 전송, TURN_TICKS Tick 후 스스로 정지 */
    if (trigger) {
        turn_right_remaining = TURN_TICKS;
        turn_left_remaining  = 0;          /* 반대 방향 회전이 남아 있었다면 취소 */
    }

    if (turn_right_remaining > 0) {
        Motor_Interface(MOTOR_RIGHT);
        turn_right_remaining--;
    }
}

void Move_Backward(EnableSignal sig)
{
    /* 2.1.5: Enable인 Tick에만 Motor_Interface(MOTOR_BACKWARD) 전송
              Disable이면 전송 안 함 -> 모터 정지 */
    if (sig == SIG_ENABLE)
        Motor_Interface(MOTOR_BACKWARD);
}

void Cleaner_Controller(CleanerCommand cmd)
{
    /* 2.1.6: Controller가 정한 Cleaner Command(On / Off / Power-Up)를 그대로 Cleaner Interface로 전달 */
    Cleaner_Interface(cmd);
}

/* ===== 유틸 ===== */

void wait_ms(int ms)
{
    /* TODO: 플랫폼별 sleep (Windows: Sleep(ms), POSIX: usleep(ms * 1000)) */
    (void)ms;
}