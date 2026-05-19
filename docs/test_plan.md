# TV Controller 테스트 계획서

| 항목 | 내용 |
|---|---|
| 문서 버전 | 1.0 |
| 작성일 | 2026-05-19 |
| 대상 코드 | `include/TVController.h`, `src/TVController.cpp` |
| 요구사항 근거 | `docs/requirements_analysis.md`, `README.md` |
| 테스트 프레임워크 | Google Test / Google Mock (C++17), CMake |
| 커버리지 도구 | gcov + lcov (권장) |

---

## 1. 목적 및 범위

본 문서는 `TVController` 모듈에 대한 **TEST_F 기반 단위·통합 테스트**의 범위, 우선순위, 경계값·예외 시나리오, 커버리지 목표 및 측정 전략을 정의한다.

### 1.1 In Scope

- `TVController` 공개 API 및 `ApplyChannel` 경유 채널 검증 로직
- `Tuner` 인터페이스와의 협력 계약 (`setCH`, `getCurrentCH`, `seekCH`)
- 숫자 입력 버퍼 상태 머신 (`ChannelBuffer == -1` 센티널)
- 선호 채널 추가/삭제/다음 선호 채널 이동
- README·요구사항 분석에 정의된 **미구현 기능**(채널 검색, 업/다운)에 대한 **선행 테스트(TDD Red)**

### 1.2 Out of Scope

- 외부 업체 `Tuner` 실구현체의 하드웨어·펌웨어 검증
- `remoteKey` → Controller 매핑 계층(리모컨 센서 미정)
- UI·네트워크·멀티스레드 동시성

### 1.3 현재 구현 스냅샷 (기준일)

| API | 구현 | 비고 |
|---|---|---|
| `PressNumber` | ○ | `src/TVController.cpp` 18행 부근 컴파일 방해 문자(`햣`) 제거 필요 |
| `PressConfirm` | ○ | |
| `PressOthers` | ○ | 버퍼 초기화 |
| `PressFavorite` | ○ | `getCurrentCH()` → `stoi` |
| `PressNextFavorite` | △ | wrap 시 `*FavoriteChannels.end()` 역참조 결함 가능 |
| `AddFavoriteChannel` | ○ | 테스트·셋업 보조 |
| 채널 검색 / 업 / 다운 | × | 요구사항·분석 문서에만 존재 |

---

## 2. 테스트 아키텍처

### 2.1 테스트 더블 전략

| 더블 | 클래스/파일 | 용도 |
|---|---|---|
| **Fake** | `DevTuner` (`include/DevTuner.h`) | `TVControllerTest` — 상태(`getCurrentCH`) 기반 행위 검증 |
| **Mock** | `MockTunerForController` (`test/TVControllerMockTest.cpp`) | `TVControllerMockTest` — 호출 인자·횟수·순서 검증 |
| **Mock** | `MockTuner` (`test/TunerTest.cpp`) | `Tuner` 인터페이스 계약·파라미터화 테스트 |
| **Golden Master** | `ApprovalTest` (`test/ApprovalTest.cpp`) | 복합 시나리오 출력 회귀 방지 (단위 테스트 보조) |

**원칙:** `Tuner`는 “정확히 동작한다”고 가정하되, Controller 테스트에서는 **Fake로 최종 채널**, **Mock으로 협력 계약**을 분리 검증한다.

### 2.2 TEST_F 픽스처 구조

```cpp
// 통합(상태) — DevTuner
class ControllerTest : public ::testing::Test {
  std::unique_ptr<DevTuner> ptrDevTuner;
  std::unique_ptr<TVController> ptrTVController;
  void SetUp() override { /* DevTuner{1,4,12,56}, TVController */ }
};

// 협력(호출) — Google Mock
class TVControllerMockTest : public ::testing::Test {
  MockTunerForController dtuner;
  std::unique_ptr<TVController> ptrTVController;
  void SetUp() override { /* TVController(dtuner) */ }
};
```

파라미터화가 필요한 경계값(채널 `0`/`99`, 잘못된 채널 문자열)은 `TunerTest`의 `TEST_P` / `INSTANTIATE_TEST_SUITE_P` 패턴을 `ControllerTest`에도 확장한다.

### 2.3 실행 타깃 (CMake)

| 실행 파일 | 소스 | 링크 |
|---|---|---|
| `TunerTest` | `test/TunerTest.cpp` | gtest, gmock |
| `TVControllerTest` | `test/TVControllerTest.cpp` | gtest, gmock, **TVController**, `--coverage` |
| `TVControllerMockTest` | `test/TVControllerMockTest.cpp` | gtest, gmock, TVController |
| `all_tests` | `test/ApprovalTest.cpp` | ApprovalTests, TVController |

`gtest_discover_tests`로 CTest 등록됨 (`CMakeLists.txt`).

---

## 3. TEST_F 단위 테스트 범위 및 우선순위

우선순위: **P0**(릴리스 차단·안전) → **P1**(요구사항 핵심) → **P2**(회귀·품질·미구현 선행).

### 3.1 `ControllerTest` (`test/TVControllerTest.cpp`)

| ID | 우선순위 | TEST_F 이름 (권장) | 시나리오 | 검증 | 상태 |
|---|---|---|---|---|---|
| C-01 | P0 | `should_set_channel_1_when_press_1_then_confirm` | `1` → 확인 | `getCurrentCH() == "1"` | ○ 구현됨 (`PressNumber1Confirm`) |
| C-02 | P0 | `should_set_channel_12_when_press_1_then_2` | 두 자리 자동 확정 | `"12"` | ○ (`PressNumber2Confirm`) |
| C-03 | P1 | `should_apply_12_then_34_when_press_1_2_3_4` | 4연속 숫자 | `"12"` → `"34"` 순서 | × |
| C-04 | P1 | `should_apply_45_then_6_when_press_4_5_6_then_confirm` | 홀수 개 + 확인 | `"45"` 후 `"6"` | × |
| C-05 | P1 | `should_discard_6_when_press_4_5_6_then_others_then_confirm` | 버퍼 폐기 | 확인 후 `"6"` 미적용 | × |
| C-06 | P1 | `should_set_channel_7_when_press_0_then_7` | 선행 0 | `"7"` (`"07"` 아님) | × |
| C-07 | P0 | `should_keep_channel_when_confirm_without_buffer` | 빈 버퍼 + 확인 | 채널·`setCH` 무변경 | × |
| C-08 | P0 | `should_throw_when_two_digit_exceeds_99` | `9`,`9` → `99` OK / `9`,`10` 불가 | `invalid_argument`, 튜너 미변경 | × |
| C-09 | P1 | `should_add_favorite_when_not_in_list` | 선호 추가 | `GetFavoriteChannels()` 포함·정렬 | ○ (`FavoriteChannelAdd`) |
| C-10 | P1 | `should_remove_favorite_when_already_in_list` | 선호 토글 삭제 | 목록에서 제거 | ○ (`FavoriteChannelRemove`) |
| C-11 | P2 | `should_maintain_unique_sorted_favorites_after_toggle_sequence` | 복합 토글 | `{6,12,37}` | ○ (`FavoriteToggleScenario`) |
| C-12 | P1 | `should_move_to_12_when_current_6_and_next_favorite` | 다음 선호 | `"12"` | × (Mock에 일부) |
| C-13 | P1 | `should_wrap_to_1_when_current_56_and_next_favorite` | 순환 | `"1"` | × (구현 버그 시 Red) |
| C-14 | P0 | `should_noop_when_next_favorite_and_list_empty` | 빈 목록 | 채널 유지 | × |
| C-15~C-22 | P1 | 채널 검색·업/다운 (요구사항 §4~6) | README 시나리오 | §4 참조 | × (API 미구현) |

### 3.2 `TVControllerMockTest` (`test/TVControllerMockTest.cpp`)

| ID | 우선순위 | TEST_F 이름 (권장) | 검증 포인트 | 상태 |
|---|---|---|---|---|
| M-01 | P0 | `should_call_setCH_with_1_when_press_1_then_confirm` | `setCH("1")` ×1 | ○ |
| M-02 | P0 | `should_call_setCH_with_12_when_press_1_then_2` | `setCH("12")` ×1, 확인 미호출 | ○ |
| M-03 | P1 | `should_not_call_setCH_when_confirm_without_buffer` | `setCH` Never | × |
| M-04 | P1 | `should_call_getCurrentCH_when_press_favorite` | `getCurrentCH()` ×1 | ○ |
| M-05 | P1 | `should_call_setCH_with_next_favorite_channel` | `getCurrentCH` → `"6"`, `setCH("12")` | ○ |
| M-06 | P2 | `should_use_canonical_string_from_to_string` | `setCH("7")` not `"07"` | × |
| M-07 | P0 | `should_not_call_setCH_when_next_favorite_empty` | no-op | × |
| M-08 | P1 | `should_reset_buffer_on_press_others` | 두 번째 숫자 전 버퍼 `-1` | × |

### 3.3 `TunerTest` (Controller 간접·계약)

Controller 직접 대상은 아니나, **채널 문자열·범위·seekCH 반복** 계약을 고정한다.

| ID | 우선순위 | 유형 | 내용 | 상태 |
|---|---|---|---|---|
| T-01 | P2 | TEST_F | 초기 채널 0~99 | ○ |
| T-02 | P1 | TEST_P | 유효 채널 `"0"`,`"99"` 등 | ○ |
| T-03 | P1 | TEST_P | 무효 채널 `"-1"`,`"100"` → 예외 | ○ (Mock 기대) |
| T-04 | P2 | TEST_F | `seekCH` 10회 유효 범위 | ○ |

### 3.4 `ApprovalTest` (보조)

| ID | 우선순위 | 내용 |
|---|---|---|
| A-01 | P2 | 숫자+확인, 두 자리, 선호 토글 출력 스냅샷 유지 |

단위 테스트 실패 시 원인 분리가 어려우므로, **신규 요구사항은 TEST_F로 먼저 추가**하고 Approval은 대표 시나리오만 갱신한다.

---

## 4. 경계값 케이스 목록

### 4.1 채널 번호 (정수 / `ApplyChannel`)

| 구분 | 입력값 | 기대 동작 | TEST_F 담당 |
|---|---|---|---|
| 하한 유효 | `0` | `setCH("0")` 성공 | C, M |
| 상한 유효 | `99` | `setCH("99")` 성공 | C, M |
| 하한 무효 | `-1` | `std::invalid_argument`, `setCH` 미호출 | C, M |
| 상한 무효 | `100`, `9999` | 동일 | C, M |
| 두 자리 합성 하한 | `0`,`0` → `0` | `"0"` | C |
| 두 자리 합성 상한 | `9`,`9` → `99` | `"99"` | C |
| 두 자리 초과 | `1`,`0` → `10` OK; `9`,`9` OK; `10` 불가(단일 키) | 정책: 키는 0~9만 | C |
| 합성 초과 | `9`,`10` 불가; 연속 `10` 불가 | — | — |
| 잘못된 합성 | `1`,`0`,`0` → `10` 후 `0` | `"10"` | C |

### 4.2 숫자 버튼 (`PressNumber(int ch)`)

| 구분 | `ch` | 기대 (정책 확정 필요) | 비고 |
|---|---|---|---|
| 최소 유효 | `0` | 버퍼·채널 정상 | `0`,`7` → `7` |
| 최대 유효 | `9` | 정상 | |
| 하한 밖 | `-1` | 예외 또는 무시 (문서화) | API가 `int` 직접 수신 |
| 상한 밖 | `10` | 동일 | 요구사항 분석 §3 |

### 4.3 입력 버퍼 (`ChannelBuffer`)

| 상태 | 조건 | 기대 |
|---|---|---|
| Empty | `ChannelBuffer == -1` | 첫 숫자만 저장, `setCH` 없음 |
| Pending | 한 자리 대기 | 확인 시 확정; 두 번째 숫자 시 즉시 확정 후 Empty |
| Clear | `PressOthers()` | `-1`, 이후 확인 무효 |

### 4.4 선호 채널

| 구분 | 조건 | 기대 |
|---|---|---|
| 빈 목록 | `FavoriteChannels.empty()` | `PressNextFavorite` no-op |
| 1개 | `{12}`, 현재 `12` | 다음 선호 = `12` (자기 순환) 정책 고정 |
| 복수 정렬 | `{56,1,12,4}` 입력 순 | 저장 `{1,4,12,56}` |
| 중복 추가 | `AddFavoriteChannel(12)` ×2 | 1개만 유지 |
| wrap | 현재 `56`, 목록 `{1,4,12,56}` | `1` (not `end()`) |

### 4.5 채널 검색·업/다운 (미구현 — 경계값 선행 정의)

| 구분 | 조건 | 기대 |
|---|---|---|
| 검색 0회 | `seekCH` 즉시 종료 신호 | 빈 검색 목록 |
| 검색 100회 | 최대 반복 | 무한 루프 없음 |
| 검색 중복 | 동일 채널 반복 반환 | 목록 유일 |
| 업/다운 일반 | 현재 `99` / `0` | `0` / `99` 순환 |
| 업/다운 검색 | `{4,6,14}`, 현재 `15` | up→`4`, down→`14` |

---

## 5. 예외·특이 케이스 목록

### 5.1 예외 (`std::exception`)

| ID | 트리거 | 예외 타입 | 검증 방법 | 우선순위 |
|---|---|---|---|---|
| E-01 | `ApplyChannel(-1)`, `ApplyChannel(100)` | `std::invalid_argument` | `EXPECT_THROW`, `setCH` 미호출(Mock) | P0 |
| E-02 | `DevTuner::setCH("100")` | `invalid_argument` | `TunerTest` TEST_P | P1 |
| E-03 | `getCurrentCH()` → `""` 후 `PressFavorite` | `stoi` → `invalid_argument` | Mock Return `""` | P1 |
| E-04 | `getCurrentCH()` → `"abc"` | 동일 | Mock | P1 |
| E-05 | `getCurrentCH()` → `"100"` (튜너 계약 위반) | `stoi` 또는 범위 정책 | 문서화 후 테스트 | P2 |

**검증 원칙:** 예외 **메시지 문자열은 고정하지 않음**. 타입·튜너 미호출·내부 목록 불변을 우선한다 (`requirements_analysis.md` §2).

### 5.2 No-op / 특이 동작

| ID | 시나리오 | 기대 | 우선순위 |
|---|---|---|---|
| S-01 | 버퍼 없음 + `PressConfirm` | 채널 유지, `setCH` 0회 | P0 |
| S-02 | `PressNextFavorite`, 선호 없음 | `setCH` 0회 | P0 |
| S-03 | `4`,`5`,`6` + `PressOthers` + `PressConfirm` | `6` 미적용 | P1 |
| S-04 | `PressFavorite` on invalid `getCurrentCH` | 예외 전파 vs catch 정책 명시 | P1 |
| S-05 | `PressNextFavorite` wrap 버그 | `end()` 역참조 없음 | P0 (결함 수정 전 Red) |
| S-06 | 컴파일 방해 | `TVController.cpp` 비ASCII/오타 제거 후 전체 테스트 Green | P0 |

### 5.3 문자열 직렬화 특이

| ID | 설명 | 기대 |
|---|---|---|
| STR-01 | `std::to_string(7)` | `setCH("7")` |
| STR-02 | `0`,`7` 입력 | `"7"` not `"07"` |
| STR-03 | Mock strict equality | `setCH("7")` ≠ `setCH("07")` |

---

## 6. 커버리지 목표 및 gcov/lcov 전략

### 6.1 목표치

| 대상 | Line | Branch | 함수 | 비고 |
|---|---|---|---|---|
| `src/TVController.cpp` | **≥ 90%** | **≥ 85%** | **100%** | 릴리스 기준 |
| `include/TVController.h` (inline) | ≥ 90% | ≥ 85% | — | `IsValidChannel`, `IsFavorite` |
| 테스트 코드 | 측정 제외 | — | — | lcov `--remove` |
| `DevTuner` | ≥ 80% (권장) | — | Fake 품질 | 별도 `DevTunerTest` 없으면 간접 |

미달 시 **PR 병합 전** 갭 리포트 첨부 및 P0/P1 테스트 추가를 원칙으로 한다.

### 6.2 CMake / 빌드 설정 (현황 및 개선)

**현황:** `TVControllerTest`에만 `--coverage` 적용 (`CMakeLists.txt` 54–55행).

**권장 개선:**

```cmake
# TVController 라이브러리 자체에 커버리지 플래그 (Mock 테스트도 동일 .o 반영)
target_compile_options(TVController PRIVATE --coverage)
target_link_options(TVController PRIVATE --coverage)
# 또는 모든 테스트 실행 파일 + TVController에 동일 적용
```

GCC/MinGW: `--coverage` (= `-fprofile-arcs -ftest-coverage -lgcov`).  
MSVC: gcov 미지원 → **WSL/Linux CI** 또는 MinGW 빌드로 커버리지 측정.

### 6.3 측정 절차 (Linux / WSL / MSYS2)

```bash
# 1. 커버리지 빌드
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage"
cmake --build build

# 2. 테스트 실행 (.gcda 생성)
ctest --test-dir build --output-on-failure

# 3. lcov 수집·필터
lcov --capture --directory build --output-file build/coverage.info
lcov --remove build/coverage.info \
  '*/test/*' '*/googletest/*' '*/gmock/*' '*/ApprovalTests/*' \
  --output-file build/coverage.filtered.info

# 4. HTML 리포트
genhtml build/coverage.filtered.info --output-directory build/coverage_html

# 5. 요약 (90% 게이트 예시)
lcov --summary build/coverage.filtered.info
```

Windows 네이티브: `lcov` 대신 **OpenCppCoverage** 또는 WSL 위 절차 사용.

### 6.4 커버리지 개선 전략

| 단계 | 액션 |
|---|---|
| 1. Baseline | 현재 `TVControllerTest` + `TVControllerMockTest` 전부 실행 후 line 리포트 저장 |
| 2. Gap 분석 | 미커버: `PressOthers`, `ApplyChannel` 예외 분기, `PressNextFavorite` wrap/empty, `AddFavoriteChannel` 중복 방지 |
| 3. Red → Green | P0 시나리오(C-07, C-08, M-03, M-07, E-01) TEST_F 추가 |
| 4. Branch | `PressNumber`의 `ChannelBuffer == -1` 양분기, `PressFavorite` add/remove, `PressNextFavorite`의 `upper_bound` vs wrap |
| 5. 회귀 | CI에서 `lcov --summary` 파싱, line < 90% 시 실패 (선택) |
| 6. 제외 | `ApprovalTest`, third-party, `main` — 필터 유지 |

### 6.5 커버리지와 테스트 레벨 매핑

| 미커버 가능 구간 | 대응 TEST_F |
|---|---|
| `ApplyChannel` invalid | C-08, M + `EXPECT_THROW` |
| `PressOthers` | C-05, M-08 |
| `PressConfirm` no buffer | C-07, M-03 |
| `PressNextFavorite` empty / wrap | C-14, C-13, M-07 |
| `AddFavoriteChannel` duplicate | 신규 `should_not_duplicate_when_add_twice` |

---

## 7. 요구사항 추적 매트릭스 (요약)

| 요구사항 (README) | 우선 TEST_F | 픽스처 |
|---|---|---|
| 한 자리 + 확인 | C-01, M-01 | Controller, Mock |
| 두 자리 자동 | C-02, M-02 | |
| 4연속 숫자 | C-03 | Controller |
| 456 + 확인/기타 | C-04, C-05 | Controller |
| 0,7 → 7 | C-06, M-06 | |
| 선호 추가/삭제 | C-09~C-11, M-04 | |
| 다음 선호 / wrap | C-12~C-14, M-05 | |
| 채널 검색 | C-15 (신규 API 후) | Mock `seekCH` |
| 업/다운 (일반/검색) | C-16~C-22 | Controller + Mock |

상세 시나리오 30건은 `docs/requirements_analysis.md` §4와 1:1 대응한다.

---

## 8. 실행·품질 게이트

### 8.1 로컬 최소 게이트

```bash
cmake --build build
ctest --test-dir build
# 커버리지 (gcov 환경)
lcov --summary build/coverage.filtered.info  # line ≥ 90%
```

### 8.2 완료 정의 (Definition of Done)

- [ ] P0 TEST_F 전부 Green
- [ ] `TVController.cpp` 컴파일 오류·UB 없음
- [ ] `TVController` line coverage ≥ 90%
- [ ] 요구사항 §4 시나리오 1~17번 Controller/Mock에 매핑된 테스트 존재
- [ ] 미구현 API(검색·업/다운)는 테스트 먼저 추가 후 구현(TDD)

### 8.3 테스트 명명 규칙 (신규)

기존: `PressNumber1Confirm`, `FavoriteChannelAdd`  
신규 권장: `should_<기대결과>_when_<조건>` (가독성·요구사항 추적)

---

## 9. 리스크 및 의존성

| 리스크 | 영향 | 완화 |
|---|---|---|
| `PressNextFavorite` wrap UB | 크래시 / 잘못된 채널 | C-13 Red → `front()` 수정 → Green |
| `stoi` on bad `getCurrentCH` | 미처리 예외 | E-03, E-04 Mock 테스트 |
| Mock만 통과·Fake 실패 | 거짓 Green | 동일 시나리오를 C-*와 M-* 쌍으로 유지 |
| MSVC gcov 부재 | 커버리지 미측정 | CI Linux + lcov |
| 미구현 업/다운 | 요구사항 미충족 | §3.1 C-15~C-22 선행 TEST_F |

---

## 10. 부록 — 신규 TEST_F 스켈레톤

```cpp
// ControllerTest — 경계: 채널 99
TEST_F(ControllerTest, should_set_channel_99_when_press_9_then_9) {
  ptrTVController->PressNumber(9);
  ptrTVController->PressNumber(9);
  EXPECT_EQ("99", ptrDevTuner->getCurrentCH());
}

// TVControllerMockTest — no-op
TEST_F(TVControllerMockTest, should_not_call_setCH_when_confirm_without_buffer) {
  EXPECT_CALL(dtuner, setCH(::testing::_)).Times(0);
  ptrTVController->PressConfirm();
}
```

---

## 11. 변경 이력

| 버전 | 날짜 | 변경 |
|---|---|---|
| 1.0 | 2026-05-19 | 초안 작성 — TVController 범위·우선순위·경계값·커버리지 전략 |
