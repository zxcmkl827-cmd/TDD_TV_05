# Defect List — TVController TDD

| 문서 정보 | 내용 |
|-----------|------|
| 프로젝트 | TDD_TV_05 |
| 모듈 | `TVController` (`src/TVController.cpp`) |
| 작성일 | 2026-05-19 |
| 작성 기준 | `test/TVControllerTest.cpp` 실행, `Report/03`·`Report/05`, ctest 결과 |
| 관련 보고서 | `Report/03.code-quality-report.md`, `Report/05.tvcontroller-unit-test-report.md` |

## 요약

| 상태 | Critical | Major | Minor | Info | 합계 |
|------|----------|-------|-------|------|------|
| **Fixed** | 2 | 0 | 0 | 0 | 2 |
| **Open** | 0 | 1 | 1 | 0 | 2 |
| **합계** | 2 | 1 | 1 | 0 | 4 |

---

## 결함 목록

### DEF-TV-001

| 필드 | 내용 |
|------|------|
| **ID** | DEF-TV-001 |
| **Severity** | Critical |
| **ItemType** | Defect (Runtime / UB) |
| **Status** | **Fixed** (commit `b8729bf`) |
| **Steps** | 1. `AddFavorites({1, 4, 12, 56})` 또는 동등 설정<br>2. `TuneTo(56)`<br>3. `PressNextFavorite()` 호출<br><br>**또는**<br>1. `AddFavorites({12})`<br>2. `TuneTo(12)`<br>3. `PressNextFavorite()` 호출 |
| **Expected** | 현재 채널보다 큰 선호 채널이 없을 때 **가장 작은 선호 채널**로 순환 (C-11).<br>• `PressNextFavorite_WrapsToSmallest`: `getCurrentCH() == "1"`<br>• `PressNextFavorite_SingleFavoriteWrapsToSelf`: `getCurrentCH() == "12"` |
| **Actual** | `EXPECT_EQ` 도달 전 프로세스 종료.<br>```text<br>Assertion failed: can't dereference out of range vector iterator<br>SEH exception 0xc0000005<br>```<br>`ControllerTest` 37건 중 **2건 Failed**, 35건 Passed. |
| **Root Cause** | `src/TVController.cpp` `PressNextFavorite()` 49–50행. `std::upper_bound`가 `end()`를 반환할 때 wrap 분기에서 `*FavoriteChannels.end()`를 역참조함. `end()`는 유효 원소가 아님. |
| **Fix Summary** | wrap 분기를 `*FavoriteChannels.begin()`(또는 `front()`)로 변경. `empty()` 가드(44–45행) 하에서 안전. |

**수정 코드 (현재):**

```cpp
int NextChannel =
    it != FavoriteChannels.end() ? *it : *FavoriteChannels.begin();
```

**재현 테스트:** `ControllerTest.PressNextFavorite_WrapsToSmallest`, `ControllerTest.PressNextFavorite_SingleFavoriteWrapsToSelf`

---

### DEF-TV-002

| 필드 | 내용 |
|------|------|
| **ID** | DEF-TV-002 |
| **Severity** | Critical |
| **ItemType** | Build Defect |
| **Status** | **Fixed** (commit `b8729bf`) |
| **Steps** | 1. 수정 전 `src/TVController.cpp`로 프로젝트 빌드<br>2. `cmake --build build --target TVControllerTest` |
| **Expected** | 컴파일·링크 성공 후 `TVControllerTest` 실행 가능. |
| **Actual** | `PressNumber()` 본문 18행 부근 비 C++ 토큰 `햣`으로 **컴파일 오류**. `TVControllerTest` 타겟 및 의존 테스트 전체 빌드 불가. |
| **Root Cause** | `src/TVController.cpp` `PressNumber()` 18행에 편집 잔여 한글 문자 `햣`이 `ApplyChannel` 호출 뒤에 삽입됨. |
| **Fix Summary** | 해당 문자 1줄 삭제. 로직 변경 없음. |

**재현:** git `01833ee` 기준 소스. 수정 후 `PressNumber` 관련 8건 `TEST_F` 전부 Passed.

---

### DEF-BLD-001

| 필드 | 내용 |
|------|------|
| **ID** | DEF-BLD-001 |
| **Severity** | Major |
| **ItemType** | Test Infrastructure / Build Environment |
| **Status** | **Open** |
| **Steps** | 1. `cmake -S . -B build` (MinGW Makefiles + Clang)<br>2. `cmake --build build`<br>3. `ctest --test-dir build` |
| **Expected** | `all_tests` 타겟 빌드 및 CTest 55/55 Passed. |
| **Actual** | `all_tests` 미빌드. CTest 항목 `all_tests_NOT_BUILT` **Not Run**.<br>```text<br>Could not find executable all_tests_NOT_BUILT<br>98% tests passed, 1 tests failed out of 55<br>```<br>TVController·Tuner·Mock 관련 **54건은 Passed**. |
| **Root Cause** | ApprovalTests.cpp 의존 타겟 `all_tests`가 Clang 환경에서 MSVC 전용 컴파일 플래그(`/W4`, `/WX`)와 충돌하여 빌드 실패. `CMakeLists.txt`에 `all_tests`가 CTest에 등록되나 실행 파일 미생성. |
| **Fix Summary** | 툴체인별 경고 플래그 분기(`MSVC` vs `GNU`/`Clang`), 또는 ApprovalTests 타겟을 선택적(`option`)으로 분리. MinGW/Clang에서 `-Wall -Wextra` 등으로 대체. |

**영향 범위:** `test/ApprovalTest.cpp`, ApprovalTests FetchContent. `TVController` 프로덕션 로직과 무관.

---

### DEF-TV-003

| 필드 | 내용 |
|------|------|
| **ID** | DEF-TV-003 |
| **Severity** | Minor |
| **ItemType** | Latent Defect / Specification Gap |
| **Status** | **Open** (테스트 미작성, Red 대기) |
| **Steps** | 1. `PressNumber(10)` 또는 `PressNumber(-1)` 호출<br>2. `PressConfirm()` 또는 두 번째 자리 입력 시도 |
| **Expected** | 요구사항 E-02(Report/04): 0–9 외 입력은 무시하거나 예외 없이 버퍼 오염 방지. |
| **Actual** | `PressNumber`가 `ch` 범위를 검증하지 않음. `ChannelBuffer = 10` 등 비정상 버퍼 상태 가능. 현재 `TVControllerTest`에는 해당 케이스 없음 → **실패 테스트 없음**. |
| **Root Cause** | `src/TVController.cpp` `PressNumber()` 11–18행에 digit 유효성 검사 부재. `ApplyChannel`만 `0–99` 채널 검증. |
| **Fix Summary** | `PressNumber` 진입 시 `if (ch < 0 \|\| ch > 9) return;` 또는 `std::invalid_argument` 정책 확정 후 테스트 추가 (`Report/04` E-02). `Tuner.h`·`remoteKey.h` 변경 불필요. |

---

## 검증 이력

| 일자 | 명령 | 결과 |
|------|------|------|
| 2026-05-19 | `ctest -R "ControllerTest\|TVControllerMockTest\|TunerTest"` | **54/54 Passed** (DEF-TV-001·002 수정 반영) |
| 2026-05-19 | `ctest` (전체) | **54 Passed**, 1 Not Run (`all_tests_NOT_BUILT`, DEF-BLD-001) |

### TVController 관련 Green 확인 (PowerShell)

```powershell
cmake --build build --target TVControllerTest TVControllerMockTest
ctest --test-dir build -R "ControllerTest|TVControllerMockTest" --output-on-failure
```

---

## 변경 이력

| 버전 | 일자 | 변경 내용 |
|------|------|-----------|
| 1.0 | 2026-05-19 | 최초 작성 — 테스트 중 발견 결함 4건 정리 |
