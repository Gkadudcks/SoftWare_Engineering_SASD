RVC 센서 시뮬레이션 (리팩토링 원본 기반 / NO WAIT)

수정 범위:
- main.c: 원본 그대로
- rvc.h: 원본 그대로
- rvc.c: Library Modules 구현 + Controller의 clock()을 단조 시계 기반 Tick으로 교체
- Afferent, FSM 처리 함수, Efferent TODO는 그대로

파일 포맷:
- sensors.txt: 각 Tick마다 front left right dust 0/1 4열. front 열은 이번 인터럽트 시뮬레이션에서 사용하지 않음.
- front_events.txt: 시작 후 경과시간(ms) front(0/1) 2열, 시간 오름차순
- front_events.txt EOF: 마지막 상태 유지
- sensors.txt EOF: 프로그램 정상 종료 (단, 각 주기 센서 인터페이스가 Tick마다 실제 호출되어야 함)
- 추가 입력 검사는 하지 않음. 파일과 데이터가 정상이라는 가정.

설계:
- Front_Sensor_Interface(): 호출될 때마다 경과시간 기준으로 도달한 이벤트만 반영
- Left/Right/Dust_Sensor_Interface(tick): tick=false면 저장값 반환; tick=true면 각자 파일을 한 행 읽고 값을 갱신
- 주기 센서 각 함수는 독립 FILE*을 가지고 같은 sensors.txt를 각각 열어 다음 행을 읽음
- Controller()가 시간과 Tick 계산을 유지하고, main.c는 그대로 무한 호출

주의:
- 원본의 Determine_Obstacle_Location / Determine_Dust_Existence가 아직 TODO이므로 main.c만으로 실행하면 센서 인터페이스를 호출하지 않음. 이 두 TODO를 팀원이 구현하면 입력 데이터 소비와 EOF 정상 종료가 발생함.
- 간단한 단독 확인은 test_sensors.c 사용

Linux/macOS GCC:
  gcc -std=c11 -Wall -Wextra main.c rvc.c -o rvc
  gcc -std=c11 -Wall -Wextra test_sensors.c rvc.c -o test_sensors
  ./test_sensors

Windows MinGW GCC:
  gcc -std=c11 main.c rvc.c -o rvc.exe
  gcc -std=c11 test_sensors.c rvc.c -o test_sensors.exe
  test_sensors.exe

(현재 while 문은 Sleep 없이 계속 돌므로 CPU를 사용할 수 있음.)
