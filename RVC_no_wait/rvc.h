/*
 * rvc.h - RVC (Robot Vacuum Cleaner) 모듈 인터페이스
 * SA(DFD + P-spec)와 SD(Structured Chart) 기준
 */
#ifndef RVC_H
#define RVC_H

#include <stdbool.h>

#define TICK_MS          200   /* 주기 신호 Tick 간격 */
#define TURN_TICKS       5     /* Turn / Power Up 유지 Tick 수 (Tick * 5) */
#define TURN_ANGLE_DEG   90    /* Turn Left / Turn Right 1회 회전 각도 (도) */
#define TURN_DEG_PER_TICK (TURN_ANGLE_DEG / TURN_TICKS)  /* Tick당 회전 각도 = 18도 (0.2초에 18도, 1초에 90도) */

/* ===== Data Dictionary ===== */

/* Obstacle Location : {Front, Left, Right} */
typedef struct {
    bool front;
    bool left;
    bool right;
} ObstacleLocation;

/* Motor Command / Direction : Forward / Backward / Left / Right */
typedef enum {
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_LEFT,
    MOTOR_RIGHT
} MotorCommand;

/* Cleaner Command / Clean : On / Off / Power-Up */
typedef enum {
    CLEANER_OFF,
    CLEANER_ON,
    CLEANER_POWER_UP
} CleanerCommand;

/* Enable / Disable 제어 신호 (Control Flow) */
typedef enum {
    SIG_DISABLE,
    SIG_ENABLE
} EnableSignal;

/* Controller FSM 상태 (DFD level 4) */
typedef enum {
    STATE_MOVE_FORWARD,
    STATE_TURN_LEFT,
    STATE_TURN_RIGHT,
    STATE_STOP,
    STATE_MOVE_BACKWARD,
    STATE_POWER_UP
} RvcState;

/* ===== Library Modules (하드웨어 인터페이스, 에뮬레이션 대상) ===== */

bool Front_Sensor_Interface(void);              /* 1.1 -> Front Obstacle  (Tick 입력 없음: 호출될 때마다 읽음) */
bool Left_Sensor_Interface(bool tick);          /* 1.2 -> Left Obstacle   (Tick일 때만 읽음) */
bool Right_Sensor_Interface(bool tick);         /* 1.3 -> Right Obstacle  (Tick일 때만 읽음) */
bool Dust_Sensor_Interface(bool tick);          /* 1.4 -> Dust Existence  (Tick일 때만 읽음) */
void Motor_Interface(MotorCommand cmd);         /* 2.2 Motor Command -> Direction */
void Cleaner_Interface(CleanerCommand cmd);     /* 2.3 Cleaner Command -> Clean */

/* ===== Afferent (입력) ===== */

ObstacleLocation Determine_Obstacle_Location(bool tick);  /* 1.5  tick은 Left/Right로 전달 */
bool Determine_Dust_Existence(bool tick);                 /* 1.6  tick은 Dust로 전달 */

/* ===== Transform Center ===== */

void Controller_Init(void);                     /* 초기 전이 0 */
void Controller(void);                          /* 2.1.1  main 루프에서 매 바퀴 호출, 내부에서 Tick 계산 */

/* Controller 내부 함수: 상태별 처리. 현재 입력을 보고 다음 상태를 반환 */
RvcState Handle_Move_Forward(ObstacleLocation loc, bool dust);   /* 전이 1, 2, 3, 9 */
RvcState Handle_Turn_Left(ObstacleLocation loc, bool dust);      /* 전이 7 */
RvcState Handle_Turn_Right(ObstacleLocation loc, bool dust);     /* 전이 8 */
RvcState Handle_Stop(ObstacleLocation loc, bool dust);           /* 전이 4 */
RvcState Handle_Move_Backward(ObstacleLocation loc, bool dust);  /* 전이 5, 6 */
RvcState Handle_Power_Up(ObstacleLocation loc, bool dust);       /* 전이 1, 2, 3, 10 */

/* ===== Efferent (출력) ===== */

void Move_Forward(EnableSignal sig);            /* 2.1.2 */
void Turn_Left(bool trigger);                   /* 2.1.3  trigger=true로 시작, 매 Tick 호출, 5 Tick 후 스스로 정지 */
void Turn_Right(bool trigger);                  /* 2.1.4  trigger=true로 시작, 매 Tick 호출, 5 Tick 후 스스로 정지 */
void Move_Backward(EnableSignal sig);           /* 2.1.5 */
void Cleaner_Controller(CleanerCommand cmd);    /* 2.1.6 */

#endif /* RVC_H */
