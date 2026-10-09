RVC 테스트 케이스 TC01~TC25 (RVC_no_wait 기준)

[실행 방법]
  Windows (MinGW gcc):  TC25_tests 폴더에서  run_tests.bat
  Linux / macOS:        sh TC25_tests/run_tests.sh

  상위 폴더(RVC_no_wait)의 main.c, rvc.c를 rvc_test(.exe)로 빌드한 뒤 각 TC 폴더 안에서 실행하고,
  출력을 expected.txt와 비교해 PASS/FAIL을 보여준다.
  Tick 사이 정지 시각([ASYNC STOP @ ..ms])은 실행 환경마다 몇 ms씩 달라질 수 있어서 Xms로 바꾼 뒤 비교한다.

[입력 파일]
  sensors.txt       한 줄 = 1 Tick, "front left right dust" (front 열은 이 버전에서 안 씀 -> 0)
                    r번째 줄(0부터)은 (r+1)번째 Tick = (r+1)*200ms에 읽힌다. 줄이 끝나면 다음 Tick에 정상 종료.
  front_events.txt  "경과시간(ms) 값". 전방 센서 인터럽트 흉내. 빈 파일이면 전방은 계속 0.
                    Windows 타이머 해상도 때문에 Tick 시각(200, 400 ...)과 100ms 떨어진 300, 500 ... 에 둔다.
  expected.txt      기대 출력. Tick별 상태 전이를 표와 대조해 확인한 값이다.

[케이스 목록]  (F/L/R/D = 전방/좌/우 장애물, 먼지)
  TC01_forward_clear
      전진 중 0 0 0 0: 초기 On, 전진 유지
      전방 이벤트: 없음
      전이: 없음 (전진 유지)
  TC02_right_blocked
      전진 중 0 0 1 0: 오른쪽이 막혀도 전진 유지
      전방 이벤트: 없음
      전이: 없음 (전진 유지)
  TC03_left_blocked
      전진 중 0 1 0 0: 왼쪽이 막혀도 전진 유지
      전방 이벤트: 없음
      전이: 없음 (전진 유지)
  TC04_both_sides_blocked
      전진 중 0 1 1 0: 좌우가 막혀도 앞이 비면 전진 유지
      전방 이벤트: 없음
      전이: 없음 (전진 유지)
  TC05_front_turn_left
      전진 중 1 0 0 0: 전진 중지·Off -> 좌회전
      전방 이벤트: 300ms F=1, 500ms F=0
      전이: T2:[T1]TURN_LEFT T7:[T7]MOVE_FORWARD
  TC06_front_right_blocked_left
      전진 중 1 0 1 0: 전진 중지·Off -> 좌회전
      전방 이벤트: 300ms F=1, 500ms F=0
      전이: T2:[T1]TURN_LEFT T7:[T7]MOVE_FORWARD
  TC07_front_left_blocked_right
      전진 중 1 1 0 0: 전진 중지·Off -> 우회전
      전방 이벤트: 300ms F=1, 500ms F=0
      전이: T2:[T2]TURN_RIGHT T7:[T8]MOVE_FORWARD
  TC08_all_blocked_back_keep
      전진 중 1 1 1 0 유지: 정지·Off -> 다음 Tick 후진 -> 후진 유지
      전방 이벤트: 300ms F=1, 1100ms F=0
      전이: T2:[T3]STOP T3:[T4]MOVE_BACKWARD
  TC09_back_both_open_left
      후진 진입 후 1 0 0 0: 후진 중지 -> 좌회전 우선
      전방 이벤트: 300ms F=1, 900ms F=0
      전이: T2:[T3]STOP T3:[T4]MOVE_BACKWARD T4:[T5]TURN_LEFT T9:[T7]MOVE_FORWARD
  TC10_back_left_open
      후진 진입 후 1 0 1 0: 후진 중지 -> 좌회전
      전방 이벤트: 300ms F=1, 900ms F=0
      전이: T2:[T3]STOP T3:[T4]MOVE_BACKWARD T4:[T5]TURN_LEFT T9:[T7]MOVE_FORWARD
  TC11_back_right_open
      후진 진입 후 1 1 0 0: 후진 중지 -> 우회전
      전방 이벤트: 300ms F=1, 900ms F=0
      전이: T2:[T3]STOP T3:[T4]MOVE_BACKWARD T4:[T6]TURN_RIGHT T9:[T8]MOVE_FORWARD
  TC12_dust_held_power_up
      전진 중 0 0 0 1 유지: Power-Up 진입, 전진 유지 -> 5 Tick 후 On
      전방 이벤트: 없음
      전이: T1:[T9]POWER_UP T6:[T10]MOVE_FORWARD
  TC13_dust_gone_keep_power_up
      Power-Up 진입 후 0 0 0 0: 먼지가 사라져도 5 Tick 유지 -> On
      전방 이벤트: 없음
      전이: T2:[T9]POWER_UP T7:[T10]MOVE_FORWARD
  TC14_front_and_dust
      전진 중 1 0 0 1: 먼지보다 장애물 우선 -> Off·좌회전
      전방 이벤트: 300ms F=1, 500ms F=0
      전이: T2:[T1]TURN_LEFT T7:[T7]MOVE_FORWARD
  TC15_power_up_front_left
      Power-Up 진입 후 1 0 0 0: 강화 중단·Off -> 좌회전
      전방 이벤트: 700ms F=1, 900ms F=0
      전이: T2:[T9]POWER_UP T4:[T1]TURN_LEFT T9:[T7]MOVE_FORWARD
  TC16_power_up_front_right
      Power-Up 진입 후 1 1 0 0: 강화 중단·Off -> 우회전
      전방 이벤트: 700ms F=1, 900ms F=0
      전이: T2:[T9]POWER_UP T4:[T2]TURN_RIGHT T9:[T8]MOVE_FORWARD
  TC17_power_up_all_blocked
      Power-Up 진입 후 1 1 1 0: 강화 중단·Off -> 정지 -> 다음 Tick 후진
      전방 이벤트: 700ms F=1, 900ms F=0
      전이: T2:[T9]POWER_UP T4:[T3]STOP T5:[T4]MOVE_BACKWARD T6:[T5]TURN_LEFT T11:[T7]MOVE_FORWARD
  TC18_left_then_right
      좌회전 완료 후 우회전 조건: 카운터 초기화, 다시 5 Tick 회전 -> 전진·On
      전방 이벤트: 300ms F=1, 500ms F=0, 1700ms F=1, 1900ms F=0
      전이: T2:[T1]TURN_LEFT T7:[T7]MOVE_FORWARD T9:[T2]TURN_RIGHT T14:[T8]MOVE_FORWARD
  TC19_async_stop_between_ticks
      전진 중 (Tick 사이) 1 0 0 0: 즉시 전진 중지·Off -> 다음 Tick 좌회전
      전방 이벤트: 500ms F=1, 700ms F=0
      전이: T3:[T1]TURN_LEFT T8:[T7]MOVE_FORWARD
  TC20_short_front_pulse
      전진 중 (Tick 사이 60ms 신호): 짧은 신호도 다음 Tick 좌회전
      전방 이벤트: 300ms F=1, 360ms F=0
      전이: T2:[T1]TURN_LEFT T7:[T7]MOVE_FORWARD
  TC21_ignore_while_turning
      좌회전 중 1 1 1 1: 회전 중 입력 무시, 5 Tick 회전 완료
      전방 이벤트: 300ms F=1, 500ms F=0, 700ms F=1, 1100ms F=0
      전이: T2:[T1]TURN_LEFT T7:[T7]MOVE_FORWARD
  TC22_turn_angle_output
      회전 Tick마다 누적 각도 18 -> 36 -> 54 -> 72 -> 90도
      전방 이벤트: 300ms F=1, 500ms F=0
      전이: T2:[T2]TURN_RIGHT T7:[T8]MOVE_FORWARD
  TC23_dust_again_no_reset
      Power-Up 진입 후 0 0 0 1 재감지: 카운트 초기화 없이 원래 시점에 On
      전방 이벤트: 없음
      전이: T2:[T9]POWER_UP T7:[T10]MOVE_FORWARD
  TC24_front_events_eof
      전진 중 전방 입력 끝(1 유지): 마지막 값 유지 -> 회전 -> 전진 -> 즉시 정지·회피 반복
      전방 이벤트: 300ms F=1
      전이: T2:[T1]TURN_LEFT T7:[T7]MOVE_FORWARD T8:[T1]TURN_LEFT
  TC25_sensor_eof
      전진 중 센서 입력 끝: End of sensor data. 출력 후 정상 종료
      전방 이벤트: 없음
      전이: 없음 (전진 유지)

[참고]
  - TC19, TC20, TC24는 Tick 사이 전방 처리(즉시 정지 + 다음 Tick까지 기억)가 있어야 통과한다.
    이 동작은 FSM 표에는 없는 확장 기능이므로, Controller에서 이 부분을 빼면 세 케이스의 기대값도 바뀐다.
  - output.txt, output_norm.txt, rvc_test(.exe)는 실행할 때 생기는 파일이라 git에 올리지 않아도 된다.
