/*
 * rvc.c - 모듈 스텁 (본문은 TODO)
 */
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "rvc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

/* CPU 사용 시간이 아닌 실제 경과 시간을 사용하는 단조 시계 */
static uint64_t monotonic_ms(void)
{
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

static uint64_t start_ms;

/* ===== Library Modules ===== */

bool Front_Sensor_Interface(void)
{
    static FILE *fp;
    static bool initialized = false;
    static bool has_event = false;
    static bool current_value = false;
    static unsigned long long event_ms;
    static int value;

    if (!initialized) {
        fp = fopen("front_events.txt", "r");
        has_event = fscanf(fp, "%llu %d", &event_ms, &value) != EOF;
        initialized = true;
    }

    uint64_t elapsed = monotonic_ms() - start_ms;

    while (has_event && event_ms <= elapsed) {
        current_value = (value == 1);
        printf("\n[EVENT @ %llums] FRONT=%d\n", event_ms, value);

        has_event = fscanf(fp, "%llu %d", &event_ms, &value) != EOF;
    }

    if (!has_event && fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    return current_value;
}

bool Left_Sensor_Interface(bool tick)
{
    static FILE *fp;
    static bool current_value = false;
    int front, left, right, dust;

    if (!tick)
        return current_value;

    if (fp == NULL)
        fp = fopen("sensors.txt", "r");

    if (fscanf(fp, "%d %d %d %d", &front, &left, &right, &dust) == EOF) {
        fclose(fp);
        puts("End of sensor data.");
        exit(EXIT_SUCCESS);
    }

    current_value = (left == 1);
    return current_value;
}

bool Right_Sensor_Interface(bool tick)
{
    static FILE *fp;
    static bool current_value = false;
    int front, left, right, dust;

    if (!tick)
        return current_value;

    if (fp == NULL)
        fp = fopen("sensors.txt", "r");

    if (fscanf(fp, "%d %d %d %d", &front, &left, &right, &dust) == EOF) {
        fclose(fp);
        puts("End of sensor data.");
        exit(EXIT_SUCCESS);
    }

    current_value = (right == 1);
    return current_value;
}

bool Dust_Sensor_Interface(bool tick)
{
    static FILE *fp;
    static bool current_value = false;
    int front, left, right, dust;

    if (!tick)
        return current_value;

    if (fp == NULL)
        fp = fopen("sensors.txt", "r");

    if (fscanf(fp, "%d %d %d %d", &front, &left, &right, &dust) == EOF) {
        fclose(fp);
        puts("End of sensor data.");
        exit(EXIT_SUCCESS);
    }

    current_value = (dust == 1);
    return current_value;
}

void Motor_Interface(MotorCommand cmd)
{
    switch (cmd) {
    case MOTOR_FORWARD:  puts("  ACTION  Motor=FORWARD");  break;
    case MOTOR_BACKWARD: puts("  ACTION  Motor=BACKWARD"); break;
    case MOTOR_LEFT:     puts("  ACTION  Motor=LEFT");     break;
    case MOTOR_RIGHT:    puts("  ACTION  Motor=RIGHT");    break;
    }
}

void Cleaner_Interface(CleanerCommand cmd)
{
    switch (cmd) {
    case CLEANER_OFF:      puts("  ACTION  Cleaner=OFF");      break;
    case CLEANER_ON:       puts("  ACTION  Cleaner=ON");       break;
    case CLEANER_POWER_UP: puts("  ACTION  Cleaner=POWER_UP"); break;
    }
}

/* ===== Afferent ===== */

ObstacleLocation Determine_Obstacle_Location(bool tick)
{
    /* 1.5: 전방은 매 호출, 좌우는 Tick일 때 새 값을 읽는다. */
    ObstacleLocation loc;
    loc.front = Front_Sensor_Interface();
    loc.left = Left_Sensor_Interface(tick);
    loc.right = Right_Sensor_Interface(tick);
    return loc;
}

bool Determine_Dust_Existence(bool tick)
{
    /* 1.6: Tick 여부를 전달하고 먼지 센서 값을 반환한다. */
    return Dust_Sensor_Interface(tick);
}

/* ===== Transform Center ===== */

static RvcState state;      /* 현재 FSM 상태 (Tick 사이에도 유지) */
static int tick_count;      /* Turn Left/Right, Power Up의 Tick * 5 카운터 (상태 진입 시 0으로) */
static uint64_t last_tick_ms; /* 마지막 Tick 시각 (실제 경과 시간 기준) */
static bool front_stop_pending; /* Tick 사이에 감지한 전방 장애물을 다음 판단까지 기억 */

/* Console-only FSM trace helpers: do not change transition logic. */
static unsigned long trace_tick_number = 0;

static const char *state_name(RvcState value)
{
    switch (value) {
    case STATE_MOVE_FORWARD:  return "MOVE_FORWARD";
    case STATE_TURN_LEFT:     return "TURN_LEFT";
    case STATE_TURN_RIGHT:    return "TURN_RIGHT";
    case STATE_STOP:          return "STOP";
    case STATE_MOVE_BACKWARD: return "MOVE_BACKWARD";
    case STATE_POWER_UP:      return "POWER_UP";
    }
    return "UNKNOWN";
}

/* Transition IDs match the team's FSM transition table. */
static int transition_id(RvcState before, RvcState after)
{
    switch (before) {
    case STATE_MOVE_FORWARD:
    case STATE_POWER_UP:
        if (after == STATE_TURN_LEFT)  return 1;
        if (after == STATE_TURN_RIGHT) return 2;
        if (after == STATE_STOP)       return 3;
        if (before == STATE_MOVE_FORWARD && after == STATE_POWER_UP) return 9;
        if (before == STATE_POWER_UP && after == STATE_MOVE_FORWARD) return 10;
        break;
    case STATE_STOP:
        if (after == STATE_MOVE_BACKWARD) return 4;
        break;
    case STATE_MOVE_BACKWARD:
        if (after == STATE_TURN_LEFT)  return 5;
        if (after == STATE_TURN_RIGHT) return 6;
        break;
    case STATE_TURN_LEFT:
        if (after == STATE_MOVE_FORWARD) return 7;
        break;
    case STATE_TURN_RIGHT:
        if (after == STATE_MOVE_FORWARD) return 8;
        break;
    }
    return 0; /* no state transition */
}


void Controller_Init(void)
{
    /* 전이 0: 시간 기준을 설정하고 전진 및 일반 청소로 시작한다. */
    state = STATE_MOVE_FORWARD;
    tick_count = 0;
    front_stop_pending = false;
    start_ms = monotonic_ms();
    last_tick_ms = start_ms;
    trace_tick_number = 0;
    puts("[INIT] State=MOVE_FORWARD | Transition=T0");
    Move_Forward(SIG_ENABLE);
    Cleaner_Controller(CLEANER_ON);
}

void Controller(void)
{
    /* Tick 계산: 마지막 Tick 이후 TICK_MS가 지났으면 이번 호출이 Tick */
    uint64_t now = monotonic_ms();
    bool is_tick = now - last_tick_ms >= TICK_MS;
    if (is_tick)
        last_tick_ms += TICK_MS;

    /* 센서 입력: Front는 매 호출, Left/Right/Dust는 Tick일 때만 새로 읽음 */
    ObstacleLocation loc = Determine_Obstacle_Location(is_tick);
    bool dust = Determine_Dust_Existence(is_tick);

    if (!is_tick) {
        /* 전진 중 전방 장애물은 다음 Tick을 기다리지 않고 중지한다. */
        if ((state == STATE_MOVE_FORWARD || state == STATE_POWER_UP) &&
            loc.front && !front_stop_pending) {
            Move_Forward(SIG_DISABLE);
            Cleaner_Controller(CLEANER_OFF);
            front_stop_pending = true;
            printf("[ASYNC STOP @ %llums] Front obstacle detected; pending for next Tick\n",
                   (unsigned long long)(now - start_ms));
        }
        return;
    }

    /* 좌우 센서가 갱신된 Tick에서 회피 방향을 결정한다.
       Tick 사이에 잠깐 감지된 장애물도 누락하지 않는다. */
    bool latched_front = front_stop_pending;
    bool front_at_tick = loc.front;
    if (front_stop_pending) {
        loc.front = true;
        front_stop_pending = false;
    }

    RvcState before = state;
    ++trace_tick_number;
    printf("\n========== TICK %03lu | %llums ==========\n",
           trace_tick_number, (unsigned long long)(last_tick_ms - start_ms));
    printf("  INPUT   F=%d L=%d R=%d D=%d%s\n",
           (int)loc.front, (int)loc.left, (int)loc.right, (int)dust,
           latched_front && !front_at_tick ? "  (F held from earlier event)" : "");
    printf("  BEFORE  %s\n", state_name(before));

    switch (state) {
    case STATE_MOVE_FORWARD:  state = Handle_Move_Forward(loc, dust);  break;
    case STATE_TURN_LEFT:     state = Handle_Turn_Left(loc, dust);     break;
    case STATE_TURN_RIGHT:    state = Handle_Turn_Right(loc, dust);    break;
    case STATE_STOP:          state = Handle_Stop(loc, dust);          break;
    case STATE_MOVE_BACKWARD: state = Handle_Move_Backward(loc, dust); break;
    case STATE_POWER_UP:      state = Handle_Power_Up(loc, dust);      break;
    }

    int transition = transition_id(before, state);
    if (transition != 0)
        printf("  RESULT  [T%d] %s -> %s\n", transition,
               state_name(before), state_name(state));
    else
        printf("  RESULT  [HOLD] %s\n", state_name(state));
}

/* ----- Controller 내부: 상태별 처리 ----- */

RvcState Handle_Move_Forward(ObstacleLocation loc, bool dust)
{
    /* 전이 1~3: 장애물 회피가 먼지 처리보다 우선, 좌회전이 우선이다. */
    if (loc.front) {
        Move_Forward(SIG_DISABLE);
        Cleaner_Controller(CLEANER_OFF);
        tick_count = 0;
        if (!loc.left) {
            Turn_Left(true);
            return STATE_TURN_LEFT;
        }
        if (!loc.right) {
            Turn_Right(true);
            return STATE_TURN_RIGHT;
        }
        return STATE_STOP;
    }

    Move_Forward(SIG_ENABLE);
    if (dust) {                       /* 전이 9: 전진하면서 흡입 강화 */
        tick_count = 0;
        Cleaner_Controller(CLEANER_POWER_UP);
        return STATE_POWER_UP;
    }
    return STATE_MOVE_FORWARD;
}

RvcState Handle_Turn_Left(ObstacleLocation loc, bool dust)
{
    (void)loc; (void)dust;
    ++tick_count;
    /* Trigger Tick에 이미 첫 회전 명령이 나갔다.
       이후 4 Tick은 회전하고, 5번째 경과 Tick에는 전진한다. */
    if (tick_count >= TURN_TICKS) {     /* 전이 7 */
        tick_count = 0;
        Move_Forward(SIG_ENABLE);
        Cleaner_Controller(CLEANER_ON);
        return STATE_MOVE_FORWARD;
    }
    Turn_Left(false);
    return STATE_TURN_LEFT;
}

RvcState Handle_Turn_Right(ObstacleLocation loc, bool dust)
{
    (void)loc; (void)dust;
    ++tick_count;
    if (tick_count >= TURN_TICKS) {     /* 전이 8 */
        tick_count = 0;
        Move_Forward(SIG_ENABLE);
        Cleaner_Controller(CLEANER_ON);
        return STATE_MOVE_FORWARD;
    }
    Turn_Right(false);
    return STATE_TURN_RIGHT;
}

RvcState Handle_Stop(ObstacleLocation loc, bool dust)
{
    /* 전이 4: 정지 상태에 진입한 다음 Tick부터 후진한다. */
    (void)loc; (void)dust;
    Move_Backward(SIG_ENABLE);
    return STATE_MOVE_BACKWARD;
}

RvcState Handle_Move_Backward(ObstacleLocation loc, bool dust)
{
    (void)dust;
    if (!loc.left) {                  /* 전이 5: 왼쪽이 비면 좌회전 */
        Move_Backward(SIG_DISABLE);
        tick_count = 0;
        Turn_Left(true);
        return STATE_TURN_LEFT;
    }
    if (!loc.right) {                 /* 전이 6: 왼쪽이 막히면 오른쪽 확인 */
        Move_Backward(SIG_DISABLE);
        tick_count = 0;
        Turn_Right(true);
        return STATE_TURN_RIGHT;
    }
    Move_Backward(SIG_ENABLE);
    return STATE_MOVE_BACKWARD;
}

RvcState Handle_Power_Up(ObstacleLocation loc, bool dust)
{
    (void)dust;                       /* 강화 중 먼지 재감지는 타이머를 리셋하지 않는다. */
    if (loc.front)                    /* 전이 1~3: 전진 상태와 같은 장애물 회피 */
        return Handle_Move_Forward(loc, false);

    Move_Forward(SIG_ENABLE);
    ++tick_count;
    if (tick_count >= TURN_TICKS) {    /* 전이 10: 5 Tick 후 일반 흡입 복귀 */
        tick_count = 0;
        Cleaner_Controller(CLEANER_ON);
        return STATE_MOVE_FORWARD;
    }
    return STATE_POWER_UP;
}

/* ===== Efferent ===== */
/*
 * 공통 규칙
 * [v2 구조] main은 Controller를 쉬지 않고 호출하고, Tick 판정은 Controller 안에서 한다.
 *           - Tick일 때: FSM 처리 함수(Handle_xxx)가 Efferent 모듈을 호출한다.
 *                        Turn Left/Right의 회전 Tick 수는 이 호출 횟수로 센다.
 *           - Tick이 아닐 때: 전진(Move Forward / Power Up) 중 전방 장애물이 감지되면
 *                        Controller가 다음 Tick을 기다리지 않고
 *                        Move_Forward(SIG_DISABLE), Cleaner_Controller(CLEANER_OFF)를 바로 호출한다.
 *                        Disable은 명령을 보내지 않으므로 이때 모터는 바로 멈춘다.
 *           - Turn_Left/Turn_Right는 Tick일 때만 호출되므로 호출 1번 = 회전 1 Tick 이다.
 *  - Move Forward / Move Backward : Enable/Disable 제어 신호를 받는다.
 *      Controller가 유지하고 싶은 Tick마다 SIG_ENABLE로 호출하고,
 *      SIG_DISABLE이면 명령을 보내지 않는다 -> 모터 정지.
 *  - Turn Left / Turn Right : Trigger로 시작해서 TURN_TICKS(5) Tick 동안 회전 명령을 보내고
 *      (1 Tick = TURN_DEG_PER_TICK(18)도, 총 TURN_ANGLE_DEG(90)도. 회전할 때마다 누적 각도를 출력한다)
 *      스스로 정지한다. 회전이 끝난 뒤의 호출(trigger=false)은 아무것도 하지 않는다.
 *      -> Trigger를 받은 Tick이 1번째 회전 Tick이다.
 *         Handle_Turn_Left/Right는 tick_count를 먼저 올리고 5 미만일 때만 Turn_xxx(false)를 부른다.
 *         -> Trigger Tick 1번 + 이후 4번 = 정확히 5 Tick 회전,
 *            tick_count == 5인 Tick에는 Turn을 부르지 않고 바로 Move_Forward(SIG_ENABLE)로 전진한다.
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
        /* 지금까지 돈 누적 각도 표시 (Tick당 TURN_DEG_PER_TICK도, 총 TURN_ANGLE_DEG도) */
        printf("  TURN    LEFT %d / %d deg\n",
               (TURN_TICKS - turn_left_remaining) * TURN_DEG_PER_TICK, TURN_ANGLE_DEG);
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
        /* 지금까지 돈 누적 각도 표시 (Tick당 TURN_DEG_PER_TICK도, 총 TURN_ANGLE_DEG도) */
        printf("  TURN    RIGHT %d / %d deg\n",
               (TURN_TICKS - turn_right_remaining) * TURN_DEG_PER_TICK, TURN_ANGLE_DEG);
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
