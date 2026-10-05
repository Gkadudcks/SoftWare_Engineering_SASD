#include "rvc.h"

/* ===== Library Modules ===== */

bool Front_Sensor_Interface(void)
{
    /* TODO 1.1: 전방 센서 아날로그 값 읽기 -> True/False 변환 (Interrupt) */
    return false;
}

bool Left_Sensor_Interface(void)
{
    /* TODO 1.2: 좌측 센서 주기적 읽기 -> True/False 변환 */
    return false;
}

bool Right_Sensor_Interface(void)
{
    /* TODO 1.3: 우측 센서 주기적 읽기 -> True/False 변환 */
    return false;
}

bool Dust_Sensor_Interface(void)
{
    /* TODO 1.4: Tick마다 먼지 센서 raw 신호 읽기 -> Dust Existence */
    return false;
}

void Motor_Interface(MotorCommand cmd)
{
    /* TODO 2.2: Motor Command -> 모터가 요구하는 Direction 신호로 변환 */
    (void)cmd;
}

void Cleaner_Interface(CleanerCommand cmd)
{
    /* TODO 2.3: Cleaner Command(On/Off/Power-Up)대로 청소기 동작 */
    (void)cmd;
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