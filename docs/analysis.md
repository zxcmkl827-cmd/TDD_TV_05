# C++ TV Controller 프로젝트 구조 분석

## 1. 프로젝트 개요

이 프로젝트는 셋탑박스/TV 컨트롤러의 채널 제어 로직을 C++17로 구현한 TDD 연습용 코드베이스입니다.

핵심 도메인은 리모컨 입력을 받아 채널을 변경하고, 선호 채널을 관리하며, 외부 튜너 인터페이스를 통해 실제 채널 변경을 수행하는 것입니다.

기술 스택은 다음과 같습니다.

- C++17
- CMake
- Google Test
- Google Mock
- ApprovalTests.cpp
- gcov/lcov 기반 커버리지 확인

## 2. 디렉터리 구조

```text
.
├── CMakeLists.txt
├── README.md
├── lcov.info
├── include/
│   ├── Tuner.h
│   ├── DevTuner.h
│   ├── TVController.h
│   └── remoteKey.h
├── src/
│   └── TVController.cpp
├── test/
│   ├── TunerTest.cpp
│   ├── TVControllerTest.cpp
│   ├── TVControllerMockTest.cpp
│   ├── ApprovalTest.cpp
│   └── ApprovalTest.PrintTextFixture.approved.txt
├── docs/
│   └── analysis.md
├── Prompt/
├── Report/
└── TDD_TV_05/
    ├── CMakeLists.txt
    ├── include/
    └── test/
```

루트의 `include`, `src`, `test`가 현재 메인 코드베이스입니다.

중첩된 `TDD_TV_05` 폴더는 초기 실습용 스냅샷에 가깝습니다. 해당 폴더의 `TVController`는 `remoteKey` 기반의 단순 입력 처리만 갖고 있고, 루트 프로젝트의 현재 구현과는 분리되어 있습니다.

## 3. 주요 클래스 역할과 책임

### Tuner

`Tuner`는 외부 튜너 업체가 제공하는 기능을 추상화한 인터페이스입니다.

책임:

- 현재 채널에서 다음 시청 가능 채널을 검색합니다.
- 지정된 채널로 튜너를 변경합니다.
- 현재 채널 값을 반환합니다.

특징:

- 순수 가상 함수만 가진 인터페이스입니다.
- `TVController`는 구체 튜너가 아니라 이 인터페이스에 의존합니다.
- 레거시 호환성 경계이므로 구조 변경 금지 대상입니다.

### DevTuner

`DevTuner`는 개발 및 테스트를 위한 `Tuner` 구현체입니다.

책임:

- 현재 채널 값을 메모리에 보관합니다.
- `setCH()`로 채널을 변경합니다.
- `seekCH()` 호출 시 등록된 채널 목록에서 현재 채널보다 큰 다음 채널을 찾고, 없으면 첫 채널로 순환합니다.

특징:

- 실제 장비 튜너가 아닌 Fake 성격의 테스트/개발용 구현체입니다.
- `TVControllerTest`와 `ApprovalTest`에서 실제 동작 검증용으로 사용됩니다.
- 채널 유효 범위는 `0 ~ 99`로 제한합니다.

### TVController

`TVController`는 리모컨 입력에 따른 도메인 로직을 담당하는 중심 클래스입니다.

책임:

- 숫자 입력을 버퍼링합니다.
- 한 자리 입력 후 확인 시 해당 채널로 이동합니다.
- 두 자리 숫자 입력 시 자동으로 채널을 적용합니다.
- 기타 버튼 입력 시 숫자 입력 버퍼를 취소합니다.
- 현재 채널을 선호 채널에 추가하거나 제거합니다.
- 선호 채널 목록에서 현재 채널보다 큰 다음 채널로 이동합니다.
- 튜너에 최종 채널 변경 명령을 전달합니다.

주요 내부 상태:

- `Tuner& tuner`: 채널 변경을 위임할 튜너 인터페이스 참조
- `int ChannelBuffer`: 숫자 입력 중간 상태를 표현하는 버퍼
- `std::vector<int> FavoriteChannels`: 선호 채널 목록

특징:

- `ChannelBuffer == -1`이면 숫자 입력 대기 상태입니다.
- `ChannelBuffer != -1`이면 한 자리 숫자가 입력되어 확인 또는 두 번째 숫자를 기다리는 상태입니다.
- 선호 채널 목록은 정렬된 상태로 유지됩니다.

### remoteKey

`remoteKey`는 리모컨 키를 표현하는 enum입니다.

현재 루트 프로젝트의 메인 `TVController` 구현에서는 직접 사용되지 않습니다. 중첩된 `TDD_TV_05` 초기 스냅샷의 `TVController`에서 `KEY_1`, `KEY_OK` 처리에 사용됩니다.

특징:

- 레거시 호환성 경계이므로 구조 변경 금지 대상입니다.
- 현재 enum 값은 `KEY_1`, `KEY_OK`만 정의되어 있습니다.

## 4. 클래스 간 의존성 관계

```text
TVController
  └── depends on Tuner interface

DevTuner
  └── implements Tuner

MockTuner / MockTunerForController
  └── implement Tuner through Google Mock

TVControllerTest
  ├── uses TVController
  └── uses DevTuner

TVControllerMockTest
  ├── uses TVController
  └── uses MockTunerForController

ApprovalTest
  ├── uses TVController
  └── uses DevTuner

TunerTest
  └── uses MockTuner
```

설계상 가장 중요한 의존성은 `TVController -> Tuner`입니다. `TVController`가 `Tuner` 인터페이스에만 의존하기 때문에 실제 튜너 대신 Fake나 Mock을 주입할 수 있습니다.

현재 구조는 생성자 주입을 사용합니다.

```cpp
explicit TVController(Tuner &t) : tuner(t) {}
```

이 방식 덕분에 테스트에서는 다음 두 가지 전략을 모두 사용할 수 있습니다.

- `DevTuner`를 넣어 상태 기반 검증을 수행합니다.
- `MockTunerForController`를 넣어 `setCH()`, `getCurrentCH()` 호출 여부를 검증합니다.

## 5. State 패턴 구현 방식

현재 코드에는 GoF State 패턴처럼 `State` 추상 클래스와 구체 상태 클래스가 분리되어 있지는 않습니다.

대신 `TVController` 내부의 `ChannelBuffer` 값으로 상태를 암묵적으로 표현하는 단순 상태 머신 구조입니다.

### 암묵적 상태

```text
Idle 상태
  - ChannelBuffer == -1
  - 아직 처리 중인 숫자 입력이 없음

DigitBuffered 상태
  - ChannelBuffer != -1
  - 한 자리 숫자가 입력되어 다음 입력을 기다림
```

### 상태 전이

```text
Idle
  └── PressNumber(ch)
      └── DigitBuffered

DigitBuffered
  ├── PressNumber(ch)
  │   ├── 두 자리 채널 계산
  │   ├── ChannelBuffer 초기화
  │   └── ApplyChannel()
  │       └── Idle
  ├── PressConfirm()
  │   ├── 한 자리 채널 적용
  │   ├── ChannelBuffer 초기화
  │   └── ApplyChannel()
  │       └── Idle
  └── PressOthers()
      └── Idle
```

### 평가

현재 구현은 상태 수가 적어서 별도 State 클래스 없이도 이해 가능한 수준입니다. 다만 리모컨 기능이 README의 전체 요구사항처럼 채널 업/다운, 채널 검색, 검색 결과 기반 이동까지 확장되면 `ChannelBuffer` 하나로 모든 상태를 표현하기 어려워질 수 있습니다.

확장 시 고려할 수 있는 방향:

- 입력 버퍼 상태를 명시적인 enum으로 분리합니다.
- 채널 검색 결과 보유 여부를 별도 상태로 분리합니다.
- 기능이 더 커지면 `IdleState`, `DigitInputState`, `ScannedChannelState` 같은 명시적 State 객체로 분리합니다.

## 6. Test Double 사용 패턴

이 프로젝트는 Fake, Mock, Approval Test를 함께 사용합니다.

### Fake: DevTuner

`DevTuner`는 실제 튜너 대신 동작하는 Fake입니다.

사용 위치:

- `TVControllerTest.cpp`
- `ApprovalTest.cpp`

용도:

- `TVController`의 동작 결과로 실제 현재 채널 값이 바뀌었는지 확인합니다.
- 선호 채널 추가/삭제 같은 상태 기반 동작을 자연스럽게 검증합니다.
- 테스트가 실제 튜너 장비나 외부 시스템에 의존하지 않게 합니다.

대표 패턴:

```text
DevTuner 생성
TVController에 DevTuner 참조 주입
리모컨 입력 메서드 호출
DevTuner의 현재 채널 또는 TVController의 선호 채널 목록 검증
```

### Mock: Google Mock 기반 Tuner Mock

`TunerTest.cpp`와 `TVControllerMockTest.cpp`는 Google Mock으로 `Tuner` 인터페이스를 대체합니다.

사용 위치:

- `MockTuner`
- `MockTunerForController`

용도:

- `setCH("12")`가 정확히 한 번 호출되는지 검증합니다.
- `getCurrentCH()`가 호출되고 특정 값을 반환하도록 제어합니다.
- 예외 발생 같은 상황을 튜너 구현 없이 시뮬레이션합니다.

대표 패턴:

```cpp
EXPECT_CALL(dtuner, setCH("12")).Times(1);
ptrTVController->PressNumber(1);
ptrTVController->PressNumber(2);
```

### Approval Test

`ApprovalTest.cpp`는 여러 입력 시나리오의 출력 문자열을 만들고 승인된 결과 파일과 비교합니다.

용도:

- 레거시 동작을 큰 흐름으로 고정하는 Golden Master 성격의 테스트입니다.
- 리팩토링 중 의도하지 않은 출력 변화가 발생했는지 빠르게 확인할 수 있습니다.

현재 승인 파일:

- `test/ApprovalTest.PrintTextFixture.approved.txt`

## 7. 테스트 구조와 특징

### TunerTest

`Tuner` 인터페이스를 Google Mock으로 대체해 기본 계약을 검증합니다.

주요 검증:

- 초기 채널 값이 유효 범위인지 확인
- 유효 채널 설정
- 유효하지 않은 채널 설정 시 예외
- `seekCH()`가 유효 채널을 반환하는지 확인

주의점:

- 실제 `DevTuner` 구현을 검증하기보다 Mock에 기대 동작을 설정하고 호출하는 형태입니다.
- 따라서 `Tuner` 계약 테스트에 가깝고, 구체 구현의 결함을 잡는 테스트는 아닙니다.

### TVControllerTest

`DevTuner` Fake를 사용해 `TVController`의 상태 기반 동작을 검증합니다.

주요 검증:

- 한 자리 입력 후 확인
- 두 자리 입력 시 자동 채널 변경
- 선호 채널 추가
- 선호 채널 제거
- 선호 채널 토글 시나리오

### TVControllerMockTest

Google Mock을 사용해 `TVController`와 `Tuner` 사이의 협력 방식을 검증합니다.

주요 검증:

- 숫자 입력 결과로 `setCH()`가 호출되는지 확인
- 선호 채널 처리 시 `getCurrentCH()` 호출 여부 확인
- 다음 선호 채널 이동 시 기대 채널로 `setCH()`가 호출되는지 확인

### ApprovalTest

`TVController`와 `DevTuner`를 조합한 시나리오 결과를 문자열로 만든 뒤 승인 파일과 비교합니다.

리팩토링 보호망으로 유용하지만, 실패 시 어떤 세부 동작이 깨졌는지는 단위 테스트보다 덜 직접적입니다.

## 8. 현재 구현상 주의할 점

### 컴파일 오류 가능성

`src/TVController.cpp`의 `PressNumber()` 내부에 독립적인 한글 문자 `햣`이 남아 있습니다. 현재 상태 그대로라면 컴파일 오류가 발생할 가능성이 큽니다.

### 다음 선호 채널 순환 로직 결함 가능성

`PressNextFavorite()`에서 현재 채널보다 큰 선호 채널이 없을 때 `*FavoriteChannels.end()`를 사용합니다.

`end()`는 마지막 원소가 아니라 마지막 다음 위치를 가리키므로 역참조하면 정의되지 않은 동작입니다. 의도는 첫 번째 선호 채널로 순환하는 것으로 보이며, 이 경우 `FavoriteChannels.front()`를 사용해야 합니다.

### 테스트 이름 규칙 불일치

현재 테스트 이름은 `PressNumber1Confirm`, `FavoriteChannelAdd`, `testSetChForValidChannel` 등으로 작성되어 있습니다.

새 `.cursorrules` 기준인 `should_[결과]_when_[조건]` 형식과는 맞지 않습니다. 향후 테스트 추가 또는 리팩토링 시 이름 규칙을 맞추는 것이 좋습니다.

### README 요구사항 대비 미구현 기능

README에는 채널 검색, 채널 업/다운, 검색 결과 기반 업/다운 동작이 요구사항으로 존재합니다.

현재 루트 `TVController` 구현에는 다음 기능이 명시적으로 보이지 않습니다.

- 채널 검색 버튼 처리
- 채널 업
- 채널 다운
- 검색된 채널 목록 기반 이동

## 9. 리팩토링 관점 제안

우선순위가 높은 개선 방향은 다음과 같습니다.

- 컴파일 오류를 유발할 수 있는 불필요한 문자 제거
- `PressNextFavorite()`의 순환 로직을 테스트로 보호한 뒤 수정
- `ChannelBuffer == -1` 매직 넘버를 이름 있는 상수 또는 명시적 상태 enum으로 변경
- 테스트 이름을 `should_[결과]_when_[조건]` 규칙으로 점진적으로 변경
- README의 미구현 기능을 테스트로 먼저 정의한 뒤 구현
- 함수 길이 20줄 이하 원칙을 유지하면서 상태 전이 로직을 작은 헬퍼로 분리

## 10. 요약

현재 프로젝트는 `TVController`가 도메인 로직을 담당하고, `Tuner` 인터페이스를 통해 외부 튜너 의존성을 분리한 구조입니다.

State 패턴은 명시적 클래스 기반이 아니라 `ChannelBuffer` 센티널 값으로 구현된 암묵적 상태 머신입니다.

Test Double은 다음처럼 역할이 분리되어 있습니다.

- `DevTuner`: 상태 기반 검증을 위한 Fake
- `MockTuner`, `MockTunerForController`: 협력 호출 검증을 위한 Mock
- `ApprovalTest`: 레거시 시나리오 보호를 위한 Golden Master 테스트

전체적으로 테스트 가능한 구조의 기반은 마련되어 있지만, 현재 코드에는 컴파일 오류 가능성, 선호 채널 순환 버그, 테스트 이름 규칙 불일치, README 요구사항 대비 미구현 영역이 남아 있습니다.
