RVC 테스트 케이스 (RVC_refactored_no_wait 기준, 15개)

[실행 방법]
  Windows (MinGW gcc):  tests 폴더에서  run_tests.bat
  Linux / macOS:        sh tests/run_tests.sh

  상위 폴더의 main.c, rvc.c를 rvc_test(.exe)로 빌드한 뒤,
  각 caseXX 폴더 안에서 실행해 output.txt를 만들고 expected.txt와 비교한다.
  (rvc.c가 sensors.txt / front_events.txt를 현재 폴더에서 읽기 때문에 케이스마다 폴더를 나눴다)

[케이스 폴더 구성]
  sensors.txt       각 줄 = 1 Tick, "front left right dust" (front 열은 이 버전에서 안 씀 -> 항상 0)
                    r번째 줄(0부터)은 (r+1)번째 Tick = (r+1)*200ms에 읽힌다.
                    줄이 다 떨어진 다음 Tick에 "End of sensor data." 출력 후 정상 종료.
  front_events.txt  "경과시간(ms) 값" -- 전방 센서 인터럽트 흉내. 빈 파일이면 전방은 계속 0.
  expected.txt      기대 출력 (손으로 FSM을 따라가 확인한 값)

  전방 이벤트 시각은 Tick 시각(200, 400, ...)과 100ms 떨어진 300, 500, 700 ... 에 두었다.
  Windows 타이머 해상도(약 16ms) 때문에 Tick 경계 근처에 두면 결과가 흔들릴 수 있어서다.

[케이스 목록]
  01 no_obstacle            장애물 없음 -> 매 Tick FORWARD, 입력 끝에서 정상 종료        (전이 0)
  02 front_turn_left        전방만 막힘 -> LEFT 5번 -> FORWARD + Cleaner ON              (전이 1, 7)
  03 front_left_turn_right  전방+왼쪽 막힘 -> RIGHT 5번 -> FORWARD + Cleaner ON          (전이 2, 8)
  04 stop_back_left         세 방향 막힘 -> 정지 -> BACKWARD -> 왼쪽 열림 -> LEFT 5번   (전이 3, 4, 5)
  05 stop_back_right        세 방향 막힘 -> 정지 -> BACKWARD -> 오른쪽만 열림 -> RIGHT 5번 (전이 3, 4, 6)
  06 back_keep              좌우가 계속 막힘 -> BACKWARD 5 Tick 유지 후 LEFT           (후진 유지)
  07 back_both_open         후진 중 좌우가 동시에 열림 -> 좌회전 우선                  (전이 5 우선순위)
  08 dust_power_up          먼지 -> POWER UP으로 5 Tick 전진 -> Cleaner ON               (전이 9, 10)
  09 dust_again_no_reset    Power-Up 중 먼지 재감지 -> 타이머 리셋 없이 08과 같은 시점에 복귀
  10 front_and_dust         전방 장애물과 먼지가 같은 Tick -> 회피 우선, POWER UP 없음
  11 power_up_then_front    Power-Up 중 전방 장애물 -> 즉시 Cleaner OFF -> LEFT 5번      (전이 9 -> 1, 7)
  12 ignore_while_turning   회전 중 전방/좌/우/먼지 입력 -> 무시하고 5 Tick 회전 완료
  13 short_front_pulse      Tick 사이 60ms(300~360ms)만 켜진 전방 신호 -> 놓치지 않고 다음 Tick에 LEFT
  14 two_obstacles          좌회전 후 전진하다 다시 막힘(왼쪽도 막힘) -> RIGHT 5번
  15 front_events_eof       front_events.txt가 "막힘"에서 끝남 -> 마지막 값 유지 -> 회피 반복

[참고]
  - 회전할 때마다 Efferent(Turn Left/Right)가 누적 각도를 "[Turn Left] 18 / 90 deg" 형식으로 출력한다.
    회전 1번 = 5 Tick = 90도, Tick당 18도 (rvc.h의 TURN_ANGLE_DEG, TURN_DEG_PER_TICK).
  - 전방 장애물을 Tick 사이에 감지하면 그 자리에서 Cleaner OFF, 다음 Tick에 FSM이 다시 OFF를 보내서
    "[Cleaner] OFF"가 두 번 찍히는 것이 현재 Controller 동작이다. expected.txt도 그대로 반영했다.
  - output.txt, rvc_test.exe는 실행할 때 생기는 파일이므로 git에 올리지 않아도 된다.
