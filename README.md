# TDD_TV

1. 셋탑박스에서 리모컨으로 부터 입력을 받아 채널을 관리하고, 튜너를 Set 하는 모듈을 만들고자 한다.

2. 리모컨에서의 입력은 숫자 0,1,2,3,4,5,6,7,8,9, 채널 업/다운, 확인, 채널검색, 선호채널추가, 다음선호채널 버튼을 통해 이루어진다.

   채널은 일반 채널 0~99 까지 있다.

3. 리모컨 센서로 부터 들어오는 입력은 아직 정해지지 않았지만, 키 값이 입력으로 들어올 것으로 여겨진다.

4. Tuner 업체에서 Channel tuner를 제공한다. 따라서 구체적인 구현이나 테스트 코드를 우리가 작성하는 상황은 아닙니다. 

    * seekCH() :  현재 채널에서 숫자가 증가하는 방향으로 시청 가능한 채널을 검색하여,  
                  채널을 검색된 채널로 변경하고 검색된 채널값을 반환한다.  
    * setCH(ch) : 지정된 채널값으로 현재 채널을 변경한다.  
    * getCurrentCH() : 현재 설정된 채널 값을 반환한다.
   
   본 실습에서 Tuner는입력에 대해 정확하게 동작한다고 가정하고 기타 개발 코드를 테스트 해야 하는 상황입니다. 
   Fake 또는 Mock을 사용하여 테스트 진행에 문제없도록 해야 합니다.     
   제공된 TunerTest.cpp를 참조한다면 Mock을 사용했을 때 필요한 기능을 좀 더 이해할 수 있습니다. Fake를 사용한 경우에도 필요한 기능을 참고할 수 있습니다.  
  
  
## TDD practice 를 적용하여, 잘 구조화된 Controller 모듈을 만들어라.
  
1. 숫자 버튼으로 채널 변경 동작
   * 리모컨 ‘1’ ‘확인’ 을 누르면 1번 채널로 변경된다.  
   * 리모컨 ‘1’을 누르고 ’2’ 를 누르면 12번 채널로 변경된다.  
   * ‘1’,’2’,’3’,’4’ 를 연속적으로 누를 경우,   
      12번 채널로 변경되었다가 34번 채널로 변경된다.  
   * ‘4’,’5’,’6’ 을 연속적으로 누를 경우: 45번 채널로 변경  
      이후 숫자와 확인 버튼을 누르면 해당 숫자의 번호로 이동(6번 또는 6_번)  
      그 외의 버튼을 누르면 6 은 무효화  
   * ‘0’,’7’ 를 누를 경우 7번 채널로 변경된다.  


2. 선호 채널 추가 버튼 동작
   * 채널 시청중에 선호채널추가를 누르는 경우  
   *    선호채널이 아닌 채널은 선호채널에 저장된다.  
   *    이미 선호채널인 채널은 선호채널에서 삭제된다.  
    
3. 다음 선호 채널 버튼 동작
   * 다음선호채널을 누르면 선호 채널 목록 내에서 현재 채널값보다 큰 값 중에서 가장 작은 값의 채널로 변경된다.  
       * 1, 4, 12, 56 이 선호채널에 저장되어 있는경우,  
       * 6번 채널 시청중 다음선호채널을 누르면 위 방향으로 가장 가까운 12로 변경된다,  
       * 56번 채널 시청중 다음선호채널을 누르면 위 방향으로 로테이션 하여 1로 변경된다.  
    
4. 채널 검색 버튼 동작
   * 리모컨 채널검색을 누르면, 모든 채널을 검색하여 저장한다.  
    
5. 업/다운 버튼 동작 : 저장된 채널 검색 결과가 없는 경우
      * 6번 채널 시청중 채널 업을 누르는 경우 7 로 변경되며,  
                     채널 다운을 누르는 경우 5 로 변경된다.  
      * 99번 채널 시청중 채널 업을 누르는 경우 0으로 변경된다.  
      * 0번 채널 시청중 채널 다운을 누르는 경우 99로 변경된다.  
  
6. 업/다운 버튼 동작 : 저장된 채널 검색 결과가 있는 경우
      * 리모컨 채널 업/다운을 누르면 저장된 채널중 다음/이전 채널로 가야 한다.  
      * 저장된 채널이 4 6 14 인 경우  
        * 채널 6을 시청중, 채널 업을 누르면 14로 변경, 다운을 누르면 4로 변경된다.  
        * 채널 15를 시청중, 채널 업을 누르면 4로 변경, 다운을 누르면 14로 변경된다.  
  

# To Do List
--------- DEV BRANCH ---------
[O] Tuner 메소드 정의
[O] TV Controller 메소드 정의
[O] Tuner 테스트 진행
[O] TV Controller 테스트 진행
[O] TV Controller Mock 테스트 진행
[O] Approval Test 진행
[O] Coverage 확인 (lcov)

--------- Refactoring BRANCH ---------
[ ] 코드 리팩토링 진행

# Tuner 테스트 결과
#3588│ [==========] Running 13 tests from 3 test suites.
#3588│ [----------] Global test environment set-up.
#3588│ [----------] 3 tests from TunerTest
#3588│ [ RUN      ] TunerTest.initChannel @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:20
#3588│ [       OK ] TunerTest.initChannel (0 ms)
#3588│ 
#3588│ [ RUN      ] TunerTest.testSeekCh10times @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:60
#3588│ [       OK ] TunerTest.testSeekCh10times (0 ms)
#3588│ 
#3588│ [ RUN      ] TunerTest.testSeekCh10timesAfterSetCH @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:76
#3588│ [       OK ] TunerTest.testSeekCh10timesAfterSetCH (0 ms)
#3588│ 
#3588│ [----------] 3 tests from TunerTest (0 ms total)
#3588│ 
#3588│ [----------] 5 tests from ValidChannels/TunerValidChannelTest
#3588│ [ RUN      ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/0 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:32
#3588│ [       OK ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/0 (0 ms)
#3588│ 
#3588│ [ RUN      ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/1 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:32
#3588│ [       OK ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/1 (0 ms)
#3588│ 
#3588│ [ RUN      ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/2 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:32
#3588│ [       OK ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/2 (0 ms)
#3588│ 
#3588│ [ RUN      ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/3 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:32
#3588│ [       OK ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/3 (0 ms)
#3588│ 
#3588│ [ RUN      ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/4 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:32
#3588│ [       OK ] ValidChannels/TunerValidChannelTest.testSetChForValidChannel/4 (0 ms)
#3588│ 
#3588│ [----------] 5 tests from ValidChannels/TunerValidChannelTest (0 ms total)
#3588│ 
#3588│ [----------] 5 tests from InvalidChannels/TunerInvalidChannelTest
#3588│ [ RUN      ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/0 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:49
#3588│ [       OK ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/0 (0 ms)
#3588│ 
#3588│ [ RUN      ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/1 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:49
#3588│ [       OK ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/1 (0 ms)
#3588│ 
#3588│ [ RUN      ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/2 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:49
#3588│ [       OK ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/2 (0 ms)
#3588│ 
#3588│ [ RUN      ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/3 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:49
#3588│ [       OK ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/3 (0 ms)
#3588│ 
#3588│ [ RUN      ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/4 @ /C:/DEV/TDD_TV_05/test/TunerTest.cpp:49
#3588│ [       OK ] InvalidChannels/TunerInvalidChannelTest.testSetChForInvalidChannel/4 (0 ms)
#3588│ 
#3588│ [----------] 5 tests from InvalidChannels/TunerInvalidChannelTest (0 ms total)
#3588│ 
#3588│ [==========] 13 tests from 3 test suites ran. (0 ms total)
#3588│ [  PASSED  ] 13 tests.
#3588│ Stopped PID#3588 - Exit(0) / OK - 'c:\DEV\TDD_TV_05\build\TunerTest.exe'


# Approval Test 결과
#6140│ Note: Google Test filter = ApprovalTest.PrintTextFixture
#6140│ [==========] Running 1 test from 1 test suite.
#6140│ [----------] Global test environment set-up.
#6140│ [----------] 1 test from ApprovalTest
#6140│ [ RUN      ] ApprovalTest.PrintTextFixture @ /C:/DEV/TDD_TV_05/test/ApprovalTest.cpp:49
#6140│ [       OK ] ApprovalTest.PrintTextFixture (1 ms)
#6140│ 
#6140│ [----------] 1 test from ApprovalTest (2 ms total)
#6140│ 
#6140│ [==========] 1 test from 1 test suite ran. (2 ms total)
#6140│ [  PASSED  ] 1 test.
#6140│ Stopped PID#6140 - Exit(0) / OK - 'c:\DEV\TDD_TV_05\build\all_tests.exe'

# TV Controller Test 결과
#10472│ [==========] Running 5 tests from 1 test suite.
#10472│ [----------] Global test environment set-up.
#10472│ [----------] 5 tests from ControllerTest
#10472│ [ RUN      ] ControllerTest.PressNumber1Confirm @ /C:/DEV/TDD_TV_05/test/TVControllerTest.cpp:16
#10472│ [       OK ] ControllerTest.PressNumber1Confirm (0 ms)
#10472│ 
#10472│ [ RUN      ] ControllerTest.PressNumber2Confirm @ /C:/DEV/TDD_TV_05/test/TVControllerTest.cpp:23
#10472│ [       OK ] ControllerTest.PressNumber2Confirm (0 ms)
#10472│ 
#10472│ [ RUN      ] ControllerTest.FavoriteChannelAdd @ /C:/DEV/TDD_TV_05/test/TVControllerTest.cpp:30
#10472│ [       OK ] ControllerTest.FavoriteChannelAdd (0 ms)
#10472│ 
#10472│ [ RUN      ] ControllerTest.FavoriteChannelRemove @ /C:/DEV/TDD_TV_05/test/TVControllerTest.cpp:38
#10472│ [       OK ] ControllerTest.FavoriteChannelRemove (0 ms)
#10472│ 
#10472│ [ RUN      ] ControllerTest.FavoriteToggleScenario @ /C:/DEV/TDD_TV_05/test/TVControllerTest.cpp:47
#10472│ [       OK ] ControllerTest.FavoriteToggleScenario (0 ms)
#10472│ 
#10472│ [----------] 5 tests from ControllerTest (0 ms total)
#10472│ 
#10472│ [==========] 5 tests from 1 test suite ran. (0 ms total)
#10472│ [  PASSED  ] 5 tests.
#10472│ Stopped PID#10472 - Exit(0) / OK - 'c:\DEV\TDD_TV_05\build\TVControllerTest.exe'

# TVControllerMock Test 결과
#17572│ [==========] Running 4 tests from 1 test suite.
#17572│ [----------] Global test environment set-up.
#17572│ [----------] 4 tests from TVControllerMockTest
#17572│ [ RUN      ] TVControllerMockTest.PressNumber1Confirm @ /C:/DEV/TDD_TV_05/test/TVControllerMockTest.cpp:23
#17572│ [       OK ] TVControllerMockTest.PressNumber1Confirm (0 ms)
#17572│ 
#17572│ [ RUN      ] TVControllerMockTest.PressNumber2Confirm @ /C:/DEV/TDD_TV_05/test/TVControllerMockTest.cpp:30
#17572│ [       OK ] TVControllerMockTest.PressNumber2Confirm (0 ms)
#17572│ 
#17572│ [ RUN      ] TVControllerMockTest.PressFavorite_GetsCurrentCH @ /C:/DEV/TDD_TV_05/test/TVControllerMockTest.cpp:37
#17572│ [       OK ] TVControllerMockTest.PressFavorite_GetsCurrentCH (0 ms)
#17572│ 
#17572│ [ RUN      ] TVControllerMockTest.NextFav_CallsSetCH @ /C:/DEV/TDD_TV_05/test/TVControllerMockTest.cpp:43
#17572│ [       OK ] TVControllerMockTest.NextFav_CallsSetCH (0 ms)
#17572│ 
#17572│ [----------] 4 tests from TVControllerMockTest (0 ms total)
#17572│ 
#17572│ [==========] 4 tests from 1 test suite ran. (0 ms total)
#17572│ [  PASSED  ] 4 tests.
#17572│ Stopped PID#17572 - Exit(0) / OK - 'c:\DEV\TDD_TV_05\build\TVControllerMockTest.exe'
