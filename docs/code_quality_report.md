# TVController 코드 품질 분석 보고서

**대상:** `include/TVController.h`, `src/TVController.cpp`  
**관점:** SOLID, Code Smell, C++17 개선 가능성  
**작성일:** 2026-05-19

---

## 1. 분석 요약

`TVController`는 리모컨 숫자 입력 버퍼링, 채널 적용, 선호 채널 CRUD·순환을 **단일 클래스**에서 처리합니다. `Tuner` 인터페이스 의존(DIP)은 잘 지켜지나, 책임이 한곳에 모여 **SRP**가 약하고, 신규 리모컨 동작 추가 시 **OCP**를 만족하기 어렵습니다. 메서드 길이는 짧지만 **매직 넘버·센티널(-1)·문자열↔정수 변환 중복**이 반복되며, `PressNextFavorite()`에는 **정의되지 않은 동작(UB)** 가능 버그가 있습니다.

> **참고:** 질의에 언급된 `50`, `11`, `6`은 `TVController` 본문에 없습니다. `50`은 `test/TunerTest.cpp` Mock 반환값, `6`·`12` 등은 테스트 시나리오 채널 번호입니다. 본 클래스에 실재하는 리터럴은 **`-1`, `0`, `99`, `10`** 입니다.

---

## 2. 문제점 분석 표

| 문제점 | 위반 원칙/스멜 | 영향 | 개선 방향 | 우선순위 |
|--------|----------------|------|-----------|----------|
| `PressNumber()` 내부 잔존 문자 `햣` (L18) | **빌드 결함** | 컴파일 실패, CI/로컬 빌드 차단 | 즉시 제거 후 빌드·테스트 재실행 | **1** |
| `PressNextFavorite()`에서 `it == end()` 시 `*FavoriteChannels.end()` 역참조 (L50–51) | **버그 / UB** | 런타임 크래시·미정의 동작; wrap 의도(첫 선호 채널) 미달성 | `*FavoriteChannels.front()` 또는 `FavoriteChannels.at(0)`로 순환 | **1** |
| 한 클래스가 숫자 입력 FSM + 채널 검증/적용 + 선호 채널 저장·토글·순환 담당 | **SRP** | 변경 시 회귀 범위 확대, 단위 테스트 격리 어려움 | `FavoriteChannelStore`(또는 `FavoriteService`) 분리; `ChannelInputBuffer` 분리 | **2** |
| `PressFavorite` / `AddFavoriteChannel`의 push + `std::sort` 중복 | **Duplicated Code** | 정렬·중복 검사 정책 변경 시 N곳 수정 | `InsertSortedUnique(int ch)` private 헬퍼 또는 Store 단일 API | **2** |
| `std::stoi(tuner.getCurrentCH())`가 `PressFavorite`, `PressNextFavorite`에 반복 | **Duplicated Code** | 파싱 실패·형식 변경 시 다수 수정 | `GetCurrentChannelInt()` 헬퍼 또는 `Channel` 값 객체 | **3** |
| `ChannelBuffer == -1` 센티널로 Idle/DigitBuffered 구분 | **Magic Number**, **Primitive Obsession** | `-1`과 유효 채널 `0` 혼동 위험, 가독성 저하 | `constexpr int kNoBufferedDigit = -1` 또는 `std::optional<int>` / `enum class InputState` | **3** |
| 채널 범위 `0`, `99`가 헤더 인라인·`DevTuner`에도 중복 | **Magic Number**, **DRY** | 범위 정책 변경 시 불일치 | `namespace tv { constexpr int kMinChannel = 0; constexpr int kMaxChannel = 99; }` 공통 헤더 | **3** |
| 두 자리 합성 `ChannelBuffer * 10 + ch`의 `10` | **Magic Number** | 3자리·선행 0 정책 확장 시 하드코딩 | `constexpr int kDecimalBase = 10`; 자릿수별 합성 함수로 캡슐화 | **4** |
| README 요구(채널 업/다운·검색 등) 추가 시 `TVController` 메서드·분기 증가 예상 | **OCP**, **God Class** 경향 | 기존 메서드 수정·거대 switch/if 체인 | `remoteKey` → `std::function` 또는 `unordered_map<remoteKey, Handler>` 테이블; 필요 시 `std::variant` 기반 명시적 FSM | **4** |
| `protected` 멤버(`tuner`, `ChannelBuffer`, `FavoriteChannels`) | **캡슐화 약화** | 하위 클래스가 내부 상태에 직접 접근·결합 증가 | `private` + 필요 시 `protected` 접근자만; 상속 확장 계획 없으면 `final` | **5** |
| 헤더 `#include <iostream>` 미사용 | **불필요 의존** | 컴파일 시간·결합도 증가 | 제거; 로깅 필요 시 구현 파일로 이동 | **5** |
| `ApplyChannel`만 예외, 입력 숫자 `ch` 검증 없음 | **일관성 부족** | `PressNumber(10)` 등 비정상 입력 시 버퍼 오염 가능 | `PressNumber` 진입 시 digit `0–9` 검증 또는 상위(UI) 계약 문서화 | **4** |
| `IsFavorite`가 매 호출 O(n) 선형 탐색 | **성능 스멜**(규모 작을 때는 허용) | 선호 채널 수 증가 시 지연 | `std::unordered_set<int>` 또는 정렬 벡터 + `binary_search` | **5** |

---

## 3. SOLID 상세

### 3.1 SRP (단일 책임 원칙) 위반

| 책임 | 현재 위치 | 근거 |
|------|-----------|------|
| 숫자 입력 상태 머신 | `PressNumber`, `PressConfirm`, `PressOthers` | `ChannelBuffer` 전이가 입력 전용 관심사 |
| 채널 유효성·튜너 적용 | `ApplyChannel`, `IsValidChannel` | 도메인 규칙 + `Tuner` 어댑터(`to_string`) |
| 선호 채널 영속(메모리) | `FavoriteChannels`, `PressFavorite`, `AddFavoriteChannel` | CRUD + 정렬 유지 |
| 선호 채널 네비게이션 | `PressNextFavorite` | 순환·upper_bound 로직 |

**한 클래스에 최소 4가지 변경 이유**가 있어, 선호 채널 정책만 바꿔도 숫자 입력 코드를 건드릴 위험이 있습니다.

### 3.2 OCP (개방-폐쇄 원칙) 위반

- 공개 API가 `PressNumber`, `PressConfirm`, `PressFavorite` 등 **고정 메서드 집합**입니다.
- `remoteKey` enum은 존재하나 루트 `TVController`는 사용하지 않습니다. 향후 `KEY_CH_UP`, `KEY_SEARCH` 등을 넣으려면 **클래스에 메서드 추가 + 호출부 수정**이 필요합니다.
- **개선:** 입력 디스패처(`void OnKey(remoteKey k)`) + 키별 핸들러 등록(테이블/람다)으로 확장 시 기존 핸들러 코드는 닫고 새 행만 추가.

### 3.3 잘 지켜진 부분

- **DIP:** `Tuner&` 추상화에 의존 → Mock/Fake 주입 가능.
- **LSP:** 인터페이스 계약 범위 내에서 구현체 교체 가능(테스트로 확인됨).

---

## 4. Code Smell 상세

### 4.1 Long Method

현재 구현 메서드는 대부분 **5~15줄**로 Long Method는 **해당 없음**. 다만 README 확장 시 `PressNumber`·검색 FSM이 비대해질 **잠재 위험**은 있음.

### 4.2 Duplicated Code

```cpp
// PressConfirm / PressNumber(2nd digit) — 버퍼 확정 패턴 유사
int SelectedChannel = ChannelBuffer;  // 또는 ChannelBuffer * 10 + ch
ChannelBuffer = -1;
ApplyChannel(SelectedChannel);

// PressFavorite / AddFavoriteChannel — 목록 갱신 후 sort
FavoriteChannels.push_back(...);
std::sort(FavoriteChannels.begin(), FavoriteChannels.end());
```

### 4.3 조건문 복잡도

- `PressNumber`: 1단계 분기 (`ChannelBuffer == -1`) — **낮음**.
- `PressFavorite`: 토글 if/else + erase-remove — **중간** (부수 효과 많음).
- `PressNextFavorite`: 삼항 + **잘못된 end() 분기** — **높음(버그)**.

---

## 5. Magic Number 정리

| 값 | 위치 | 의미 | 상수화 권장명 (예) |
|----|------|------|-------------------|
| `-1` | `ChannelBuffer` 초기값, 비교 다수 | 입력 버퍼 없음 | `kNoBufferedDigit` 또는 `std::nullopt` |
| `0`, `99` | `IsValidChannel` | 유효 채널 범위 | `kMinChannel`, `kMaxChannel` (공유) |
| `10` | `PressNumber` 2자리 합성 | 십진 자릿수 | `kDecimalBase` |
| `0` | (간접) 선호 목록 `empty()` early return | 빈 컬렉션 | 의미상 상수 불필요; `empty()` 유지 권장 |

---

## 6. C++17 스타일 개선 방향

### 6.1 상태 표현: `optional` / `enum`

```cpp
enum class DigitBufferState { Empty, OneDigitPending };
std::optional<int> bufferedDigit_;  // -1 센티널 제거
```

3자리·선행 0(`07`→`7`) 요구가 오면 `enum class` + 전이 테이블이 `int` 센티널보다 안전합니다.

### 6.2 테이블 기반 디스패치 (OCP)

```cpp
using KeyHandler = std::function<void(TVController&)>;
const std::unordered_map<remoteKey, KeyHandler> handlers = {
    { remoteKey::KEY_1, /* ... */ },
    { remoteKey::KEY_OK, /* ... */ },
};
```

리모컨 키 종류가 늘어날 때 **맵에 행 추가**만으로 확장 가능합니다.

### 6.3 `std::variant` FSM (확장 시)

```cpp
using ControllerState = std::variant<
    struct Idle {},
    struct DigitBuffered { int first; },
    struct ScanResultPending { std::vector<int> hits; }
>;
```

채널 검색·검색 결과 탐색 등 README 미구현 기능을 넣을 때 if/switch 폭발을 줄입니다.

### 6.4 전략 패턴 (책임 분리)

| 전략 | 역할 |
|------|------|
| `IChannelApplicator` | `IsValid` + `tuner.setCH` |
| `IFavoriteStore` | sorted unique insert/remove/contains/next |
| `IDigitInputPolicy` | 1/2/3자리·선행 0 규칙 |

`TVController`는 전략을 조합하는 **Facade**로 축소합니다.

### 6.5 기타 C++17

- `[[nodiscard]]` on `GetFavoriteChannels()`
- `string_view`는 `Tuner` API가 `std::string` 고정이라 레거시 경계 내에서는 `stoi` 래퍼만 우선
- `FavoriteChannels` 반복 시 `const auto&` (이미 일부 적용)

---

## 7. 리팩토링 우선순위 (1~5) 및 이유

| 순위 | 항목 | 이유 |
|------|------|------|
| **1** | `햣` 제거, `PressNextFavorite` UB 수정 | 정확성·빌드 가능성 — 기능 코드보다 선행 필수 |
| **2** | 선호 채널 로직 캡슐화 + 중복 제거 (`InsertSortedUnique`, erase 헬퍼) | SRP·DRY; 버그 수정 후 회귀 테스트(`NextFav_CallsSetCH` 등)로 보호 용이 |
| **3** | `-1`/`0`/`99`/`10` 명명 상수 또는 `optional` 버퍼 | 가독성·오류 예방; 동작 변경 없는 안전 리팩토링 |
| **4** | `GetCurrentChannelInt()` 추출, digit 검증, OCP용 키 디스패치 스켈레톤 | 문자열 경계 일원화; README 확장 대비 |
| **5** | `protected`→`private`, 미사용 include, `unordered_set` 검토 | 유지보수성·미세 최적화; 동작 영향 적음 |

---

## 8. 개선 방향 요약

1. **즉시:** 컴파일 오류 문자 제거, `PressNextFavorite` wrap을 `front()`로 수정하고 wrap 시나리오 테스트 추가.
2. **단기:** 매직 넘버를 `constexpr` 또는 `optional`로 치환; 선호 채널·숫자 버퍼를 private 헬퍼/소형 클래스로 분리해 SRP 회복.
3. **중기:** `Tuner` 문자열 API를 감싸는 채널 값 타입 도입; README의 업/다운·검색은 `remoteKey` 테이블 + `variant` FSM으로 OCP 충족.
4. **원칙:** 리팩토링마다 기존 `TVControllerTest` / Mock / Approval 테스트 Green 유지(TDD).

현재 규모에서는 **전략 패턴 전면 도입은 과설계**일 수 있으나, 요구사항 확장이 확인된 상태이므로 **테이블 디스패치 + FavoriteStore 분리**를 먼저 적용하는 것이 비용 대비 효과가 큽니다.

---

## 9. 참고 코드 위치

| 이슈 | 파일:라인 |
|------|-----------|
| 컴파일 오류 문자 | `src/TVController.cpp:18` |
| UB 가능 순환 | `src/TVController.cpp:50-51` |
| 채널 범위 매직 넘버 | `include/TVController.h:40` |
| 센티널 `-1` | `include/TVController.h:24`, `src/TVController.cpp` 다수 |
| 선호 채널 sort 중복 | `src/TVController.cpp:39-40`, `57-58` |
