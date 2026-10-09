RVC FSM TEST CASES  (SA/SD team #12, no-wait version)
=====================================================
Test files are based on the FSM transition table 0..10 in your SA slides.
Each case is independent: reset/restart the RVC process for each case.

USAGE (Windows cmd, with gcc/MinGW)
-----------------------------------
1. Place this RVC_FSM_testcases folder under your project root.
2. Build your project's completed FSM version:
       gcc -std=c11 main.c rvc.c -o rvc.exe
3. Start each scenario using:
       RVC_FSM_testcases\run_case.bat TC02_turn_left_5ticks
   Or manually: cd RVC_FSM_testcases\TC02_turn_left_5ticks
                ..\..\rvc.exe
   (Programs read sensors.txt and front_events.txt from their current working directory.)
4. Open expected.txt or expected.csv and compare against actual state trace.

INPUT FILE FORMAT
-----------------
- sensors.txt: four integers per line, in order: front / left / right / dust.
  The first 'front' column is a placeholder in the no-wait version, which
  instead obtains front input from front_events.txt. Set placeholder to 0.
  The FIRST line is used at the first Tick, approximately 200ms after Init.
  Each subsequent line is used at another 200ms interval.
- front_events.txt: <elapsed milliseconds> <front state: 0 or 1>.
  Entries are processed when elapsed time reaches their timestamp, regardless
  of FSM Tick. Empty file means front remains false.
- Input is assumed valid (0/1; no validation requested). EOF handling is
  supplied by the Library implementation. sensors.txt EOF ends the program;
  front_events.txt EOF preserves the last front state.
- Tick timing is based on nominal times 200,400,600,...ms. The timestamps
  in these scenarios avoid being exactly on Tick boundaries.

IMPORTANT VERIFICATION NOTES
----------------------------
* The rvc.c last uploaded by the user has TODO placeholders for Afferent,
  FSM, Efferent. These fixtures do not make that placeholder code pass;
  implement those sections before verifying the FSM.
* Expected-state convention: turning and Power-Up states are entered with
  count=0; five SUBSEQUENT Ticks complete a turn or Power-Up operation.
  Example: enter TURN_LEFT at T2; it completes at T7. If your team decided
  to count the trigger Tick as tick #1, clarify the spec and adjust expected
  timing accordingly. The SA slides do not unambiguously define that detail.
* In STOP state, transition 4 occurs on the next Tick.
* Obstacle transitions (1/2/3) are evaluated before dust transition 9.
* No wait_ms() call is needed: the busy loop itself continuously polls time.
* To compare FSM internal states, log the state AFTER each Tick in Controller
  (or use a debugger). Motor/Cleaner prints alone may not prove correct state.
* This is testing DATA, not executable FSM logic; cases were checked with an
  independent reference-state model, NOT against your (still TODO) FSM code.

CASES
-----
TC01_forward_keep : 장애물과 먼지가 없을 때 계속 전진 (전이 없음) (transitions no transition)
TC02_turn_left_5ticks : 전방 장애물 + 좌측 비어 있음 -> 좌회전 (5 Tick 후 전진) (transitions 1,7)
TC03_turn_right_priority : F=1,L=1,R=0,D=1 -> 먼지보다 장애물 우선, 우회전 5 Tick (transitions 2,8)
TC04_stop_back_right : 전방/좌/우 막힘 -> STOP -> BACKWARD 유지 -> 우회전 -> 전진 (transitions 3,4,6,8)
TC05_stop_back_left : 전방/좌/우 막힘 -> STOP -> BACKWARD -> 좌측 개방 -> 좌회전 (transitions 3,4,5,7)
TC06_powerup_5ticks : 먼지 감지 -> POWER_UP, 먼지가 없어져도 5 Tick 유지 -> 전진 (transitions 9,10)
TC07_powerup_interrupted_left : POWER_UP 도중 전방 장애물 -> 5 Tick 종료 전에 좌회전 (장애물 우선) (transitions 1,7,9)
TC08_powerup_stop_back : POWER_UP 도중 사방 막힘 -> STOP -> BACKWARD -> 좌회전 -> 전진 (transitions 3,4,5,7,9)
TC09_front_pulse_between_ticks : 전방 센서가 두 Tick 사이에서 1->0 변화: FSM은 Tick 순간 0으로 보며 전진 유지 (transitions no transition)

TRANSITION COVERAGE
-------------------
 1: TC02_turn_left_5ticks, TC07_powerup_interrupted_left
 2: TC03_turn_right_priority
 3: TC04_stop_back_right, TC05_stop_back_left, TC08_powerup_stop_back
 4: TC04_stop_back_right, TC05_stop_back_left, TC08_powerup_stop_back
 5: TC05_stop_back_left, TC08_powerup_stop_back
 6: TC04_stop_back_right
 7: TC02_turn_left_5ticks, TC05_stop_back_left, TC07_powerup_interrupted_left, TC08_powerup_stop_back
 8: TC03_turn_right_priority, TC04_stop_back_right
 9: TC06_powerup_5ticks, TC07_powerup_interrupted_left, TC08_powerup_stop_back
10: TC06_powerup_5ticks
0: Controller_Init initial transition (MOVE_FORWARD, Cleaner ON)

Reference state sequences are in expected.txt and expected.csv for each case.
