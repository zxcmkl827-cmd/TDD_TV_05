# TVController QA 최종 보고서

| 항목 | 내용 |
|---|---|
| 프로젝트 | TDD_TV_05 |
| 대상 | `TVController` C++17 모듈, `Tuner` 협력 계약, 테스트/빌드 인프라 |
| 작성일 | 2026-05-19 |
| 관점 | QA 리드 엔지니어 |
| 주요 근거 | `docs/requirements_analysis.md`, `docs/code_quality_report.md`, `docs/test_plan.md`, `docs/defect_report.md`, `docs/defect_list.md`, `include/TVController.h`, `src/TVController.cpp`, `test/TVControllerTest.cpp` |

---

## 1. Executive Summary

`TVController`의 핵심 구현 범위인 숫자 입력, 확인, 기타 입력, 선호 채널 토글, 다음 선호 채널 이동은 현재 테스트와 커버리지 관점에서 안정권에 진입했다. 최신 검증 기준으로 전체 CTest는 **56/56 Passed**, `TVController` 대상 gcov line coverage는 **97.8%**, function coverage는 **100.0%**로 목표를 충족한다.

다만 요구사항 전체 관점에서는 채널 검색 및 업/다운 API가 아직 구현되지 않아 기능 완료율은 테스트 통과율보다 낮다. 또한 `PressNumber(int)`의 0~9 외 입력 정책과 `getCurrentCH()` 비정상 문자열 정책은 결함 원장에 남아 있는 잠재 리스크다.

| 품질 항목 | 목표 | 현재 결과 | 판정 |
|---|---:|---:|---|
| 전체 테스트 통과율 | 100% | 56/56 Passed = **100.0%** | 달성 |
| TVController 관련 테스트 | 100% | `ControllerTest` 37건 + Mock 4건 = **41/41 Passed** | 달성 |
| 요구사항 시나리오 커버리지 | 100% | 30건 중 약 **18건 실질 커버** = **60.0%** | 부분 달성 |
| 구현 완료된 핵심 기능 커버리지 | 90% 이상 | 숫자/확인/기타/선호 채널 주요 흐름 대부분 커버 | 달성 |
| Line coverage | 90% 이상 | **97.8%** (45/46, `TVController.h/.cpp`) | 달성 |
| Function coverage | 100% | **100.0%** (11/11) | 달성 |
| Branch coverage | 85% 이상 | gcov branch executed **95.7%**, taken at least once **60.9%** | 추가 해석 필요 |
| Open Critical defects | 0건 | **0건** | 달성 |

> Branch coverage는 gcov 직접 출력 기준이다. `TVController.cpp`는 branches executed 95.0%, taken at least once 62.5%이며, `TVController.h`는 branches executed 100.0%, taken at least once 50.0%이다. STL 템플릿/인라인 분기 영향이 섞이므로 PR 게이트에는 lcov branch 집계 설정을 별도로 안정화하는 것이 필요하다.

---

## 2. 테스트 완료율 및 커버리지

### 2.1 테스트 실행 결과

검증 명령:

```powershell
cmake --build build
ctest --test-dir build --output-on-failure
```

결과:

| 스위트 | 테스트 수 | 결과 | 비고 |
|---|---:|---|---|
| `TunerTest` | 13 | Passed | 유효/무효 채널, `seekCH` 계약 |
| `ControllerTest` | 37 | Passed | `TVController` 상태 기반 검증 |
| `TVControllerMockTest` | 4 | Passed | `Tuner` 협력 호출 검증 |
| `GoldenMasterTest` | 1 | Passed | 파일 비교 기반 회귀 |
| `ApprovalTest` | 1 | Passed | ApprovalTests 기반 회귀 |
| **합계** | **56** | **56/56 Passed** | 전체 통과율 100.0% |

이전 결함 보고서에서 Open이었던 `all_tests_NOT_BUILT` 계열 인프라 리스크는 현재 `ApprovalTest`와 `GoldenMasterTest`가 모두 CTest에 정상 등록/실행되어 재현되지 않았다. 다만 결함 원장 상태는 별도 갱신이 필요하다.

### 2.2 요구사항 대비 테스트 완료율

`docs/requirements_analysis.md`의 30개 시나리오 기준으로 보면, 현재 테스트는 구현된 핵심 기능에는 강하지만 README 확장 범위에는 아직 도달하지 않았다.

| 요구사항 그룹 | 시나리오 | 현재 커버 상태 |
|---|---:|---|
| Tuner 기본 계약 | 1~3 | 커버 |
| 숫자 입력/확인/버퍼 | 4~10 | 커버 |
| 잘못된 숫자 버튼 입력 | 11 | 미커버, DEF-TV-003 |
| 선호 채널 추가/삭제/정렬/순환 | 12~17 | 커버 |
| 채널 검색 | 18~19 | API 미구현 |
| 업/다운 일반 채널 이동 | 20~23 | API 미구현 |
| 업/다운 검색 결과 기반 이동 | 24~27 | API 미구현 |
| canonical string 변환 | 28 | 부분 커버 |
| 비정상 `getCurrentCH()` 파싱 | 29 | 미커버 |
| no-op 입력 | 30 | 부분 커버 |

정량적으로는 30개 중 약 18개가 실질 커버되어 **요구사항 시나리오 커버리지는 약 60.0%**로 본다. 단, 현재 구현된 `TVController` 공개 API 범위만 놓고 보면 `TVControllerTest.cpp`가 공개 메서드 6개 각각에 5건 이상 테스트를 제공하므로 단위 테스트 완료율은 높다.

### 2.3 gcov 수치

검증 명령:

```powershell
cmake -S . -B build-gcov -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage"
cmake --build build-gcov
ctest --test-dir build-gcov --output-on-failure
lcov --capture --directory build-gcov --output-file build-gcov/coverage.info
lcov --extract build-gcov/coverage.info '*src/TVController.cpp' '*include/TVController.h' --output-file build-gcov/coverage.tvcontroller.info
lcov --summary build-gcov/coverage.tvcontroller.info
gcov -b -o build-gcov/CMakeFiles/TVController.dir/src/TVController.cpp.obj src/TVController.cpp
```

| 대상 | Line | Function | Branch executed | Taken at least once |
|---|---:|---:|---:|---:|
| `src/TVController.cpp` | 97.44% (38/39) | 포함 | 95.00% (38/40) | 62.50% (25/40) |
| `include/TVController.h` | 100.00% (4/4) | 포함 | 100.00% (6/6) | 50.00% (3/6) |
| **합산** | **97.8% (45/46)** | **100.0% (11/11)** | **95.7% (44/46)** | **60.9% (28/46)** |

해석:

- Line/function 기준은 목표를 명확히 초과한다.
- Branch executed 기준도 85% 목표를 넘지만, taken-at-least-once 기준은 목표 미달이다.
- 미달 원인은 `ApplyChannel` 예외 경로, `PressNumber` invalid digit 정책, `stoi` 예외 정책, STL 알고리즘 내부 분기까지 함께 잡히는 gcov 특성 때문이다.
- 다음 게이트에서는 `--rc branch_coverage=1` 설정과 필터링 기준을 CI에 고정하고, "branches executed"와 "taken at least once" 중 어떤 값을 공식 지표로 삼을지 합의해야 한다.

---

## 3. 결함 패턴 분석

### 3.1 Severity별 현황

결함 원장(`docs/defect_list.md`) 기준 결함은 총 4건이다.

| Severity | Fixed | Open | 합계 | 비율 |
|---|---:|---:|---:|---:|
| Critical | 2 | 0 | 2 | 50% |
| Major | 0 | 1 | 1 | 25% |
| Minor | 0 | 1 | 1 | 25% |
| Info | 0 | 0 | 0 | 0% |
| **합계** | **2** | **2** | **4** | **100%** |

주요 관찰:

- Critical 2건은 모두 조기에 발견되어 수정되었다. `PressNextFavorite`의 UB와 `PressNumber` 내 비정상 토큰은 릴리스 차단급이었으나 단위 테스트/빌드 검증으로 차단됐다.
- Open Critical은 0건이므로 릴리스 안전성의 최우선 게이트는 통과했다.
- 남은 Open은 인프라 상태 갱신 필요 1건과 정책 미정 잠재 결함 1건이다.

### 3.2 ItemType별 현황

| ItemType | 결함 ID | 건수 | 패턴 |
|---|---|---:|---|
| Defect (Runtime / UB) | DEF-TV-001 | 1 | 컨테이너 `end()` 역참조, wrap 분기 테스트 부족 |
| Build Defect | DEF-TV-002 | 1 | 편집 잔여 토큰, 빌드 게이트로 탐지 |
| Test Infrastructure / Build Environment | DEF-BLD-001 | 1 | CTest 등록/실행 파일 생성 불일치 |
| Latent Defect / Specification Gap | DEF-TV-003 | 1 | API 입력 정책 미정, Red 테스트 부재 |

결함 패턴은 "로직 자체의 복잡성"보다 "경계 조건과 검증 게이트의 공백"에서 발생했다. 특히 wrap, 빈 목록, 비정상 입력처럼 정상 흐름에서 벗어나는 지점이 주요 결함 발생 지점이었다.

### 3.3 근본 원인 요약

| 원인 | 사례 | QA 판단 |
|---|---|---|
| 경계값 테스트 부족 | `PressNextFavorite` wrap, 단일 선호 채널 자기 순환 | P0/P1 경계 테스트가 가장 큰 결함 예방 효과를 냄 |
| 빌드/정적 검증 미흡 | `햣` 토큰으로 컴파일 실패 | 로컬 빌드와 CI를 산출물 작성 전 항상 실행해야 함 |
| 명시적 입력 계약 부재 | `PressNumber(-1)`, `PressNumber(10)` | 공개 API가 `int`를 받는 이상 정책 테스트 필요 |
| 문자열 경계 리스크 | `std::stoi`, `std::to_string` | Tuner 문자열 계약은 별도 헬퍼/값 객체로 격리하는 것이 바람직 |
| 테스트 인프라와 기능 테스트 혼재 | Approval target 문제 | Golden/Approval은 보조 게이트로 두되 단위 테스트와 원인 분리 필요 |

---

## 4. 9단계 QA 활동 평가

| 단계 | 산출물 | 효과 | 개선 필요 |
|---|---|---|---|
| 1. 프로젝트 규칙/구조 분석 | `Report/01`, `.cursorrules`, `docs/analysis.md` | 레거시 경계(`Tuner.h`, `remoteKey.h`)와 테스트 원칙을 초기에 고정 | 규칙은 좋았지만 테스트 이름 규칙이 기존 코드와 완전히 일치하지는 않음 |
| 2. 요구사항 분석 | `Report/02`, `docs/requirements_analysis.md` | 30개 시나리오와 문자열/경계값 리스크를 체계화 | 검색/업다운처럼 미구현 범위는 구현 일정과 별도 표시 필요 |
| 3. 코드 품질 분석 | `Report/03`, `docs/code_quality_report.md` | Critical 후보 2건과 SRP/OCP 리스크를 조기 식별 | 분석 시점 결함 상태와 현재 수정 상태를 계속 동기화해야 함 |
| 4. 테스트 계획 | `Report/04`, `docs/test_plan.md` | P0/P1/P2 우선순위, Fake/Mock 분리, 커버리지 목표 수립 | branch coverage 측정 정의가 충분히 고정되지 않음 |
| 5. 단위 테스트 구현 | `Report/05`, `test/TVControllerTest.cpp` | 37개 `TEST_F`로 핵심 공개 API를 강하게 보호 | invalid digit, malformed `getCurrentCH()`는 아직 Red 테스트 필요 |
| 6. 결함 분석/원장화 | `Report/06`, `docs/defect_list.md` | 결함 ID, Severity, Root Cause, Fix Summary가 추적 가능 | 원장 상태가 최신 CTest 결과와 일부 불일치 가능 |
| 7. Golden Master/Approval | `Report/07` | 리팩토링 시 복합 시나리오 회귀 감지 기반 마련 | 스냅샷은 보조 수단이므로 단위 테스트 대체로 쓰면 안 됨 |
| 8. 리팩토링 계획 | `Report/08` | Green 상태 기반의 점진적 개선 순서를 제시 | 아직 계획 단계이며 실제 리팩토링 커밋으로 전환 필요 |
| 9. 결함 관리 체계 | `Report/09`, `docs/defect_report.md` | Severity, ItemType, 상태 전이, 메트릭 운영 기준 수립 | 실측 커버리지와 결함 원장 갱신을 정례화해야 함 |

가장 효과적이었던 단계는 **2. 요구사항 분석**, **4. 테스트 계획**, **5. 단위 테스트 구현**, **6. 결함 분석/원장화**다. 이 흐름이 `PressNextFavorite` UB를 재현 가능한 테스트로 바꾸고, 수정 후 회귀를 방지했다.

개선이 필요한 단계는 **3. 코드 품질 분석의 최신화**, **4. 커버리지 지표 정의**, **9. 결함 상태 운영**이다. 분석 보고서와 실제 코드가 빠르게 달라지는 만큼, 최종 보고서 작성 전 자동으로 테스트/커버리지/결함 원장 상태를 재검증하는 루틴이 필요하다.

---

## 5. 다음 레거시 프로젝트를 위한 Best Practice 5가지

1. **요구사항을 테스트 시나리오 ID로 먼저 쪼갠다.**  
   자연어 요구사항을 C-*, M-*, E-* 같은 추적 ID로 바꾸면 구현 완료율과 테스트 갭이 숫자로 보인다.

2. **Fake와 Mock의 역할을 분리한다.**  
   Fake는 최종 상태 검증, Mock은 협력 계약 검증에 사용한다. 이번 프로젝트에서는 `DevTuner`와 Google Mock 분리가 거짓 Green을 줄였다.

3. **경계값과 no-op을 P0로 올린다.**  
   `0`, `99`, 빈 버퍼, 빈 선호 목록, wrap은 작은 코드에서도 크래시를 만든다. 정상 흐름보다 먼저 잠가야 한다.

4. **커버리지 목표는 측정 명령까지 함께 버전 관리한다.**  
   line/function/branch의 정의가 도구마다 다르므로 CMake 플래그, lcov 필터, gcov 버전을 CI에 고정해야 한다.

5. **결함 원장을 테스트 결과와 연결한다.**  
   결함은 ID, 재현 테스트, Root Cause, Fix Summary, Verification을 한 묶음으로 관리한다. Fixed는 테스트와 커버리지 확인 후 Verified로 전환한다.

---

## 6. Cursor AI 활용 효과

### 6.1 정량 요약

| 영역 | 효과 | 근거 |
|---|---:|---|
| 테스트 확장 | `ControllerTest` 5건 수준에서 37건으로 확장 | 공개 API 6개에 메서드별 5건 이상 |
| 전체 테스트 게이트 | 56/56 Passed | CTest 전체 실행 기준 |
| 결함 조기 발견 | Critical 2건 발견 및 Fixed | DEF-TV-001, DEF-TV-002 |
| 커버리지 향상 | line 97.8%, function 100.0% | gcov/lcov `build-gcov` 실측 |
| 요구사항 추적 | 30개 시나리오 도출 | `docs/requirements_analysis.md` |
| 결함 관리 표준화 | 4개 결함 ID와 Severity/ItemType 체계화 | `docs/defect_list.md`, `docs/defect_report.md` |

시간 단축은 저장소 내 실제 작업 로그만으로 정확한 man-hour를 산정할 수는 없지만, QA 산출물 9종과 테스트/결함/커버리지 검증을 같은 세션 흐름에서 생성·검토했다는 점에서 수작업 대비 문서화와 테스트 설계의 반복 시간이 크게 줄었다. 보수적으로는 요구사항 분석, 테스트 매트릭스 작성, 결함 템플릿 작성 영역에서 **30~50% 수준의 리드타임 단축 효과**가 있었다고 평가한다.

### 6.2 정성 요약

Cursor AI의 가장 큰 효과는 "코드 작성"보다 **QA 사고의 구조화**였다. 요구사항을 경계값과 예외 조건으로 풀어내고, 코드 품질 스멜을 결함 후보로 연결한 뒤, 테스트와 결함 원장으로 추적 가능하게 만들었다.

또한 결함 조기 발견 측면에서 `PressNextFavorite`의 `end()` 역참조처럼 리뷰만으로 지나칠 수 있는 런타임 결함을 테스트 시나리오로 재현해 수정까지 연결했다. 커버리지 측면에서는 단순 테스트 개수 증가가 아니라 `PressOthers`, no-op, wrap, boundary 같은 미커버 지점을 명시적으로 채우는 방향으로 개선됐다.

주의할 점도 있다. AI가 작성한 분석 문서는 시간이 지나면 실제 코드 상태와 어긋날 수 있다. 따라서 Cursor AI 활용은 "초안 자동화"가 아니라 **빌드, 테스트, gcov 실측으로 검증되는 QA 루프** 안에 넣어야 신뢰할 수 있다.

---

## 7. 최종 QA 판단 및 후속 과제

현재 `TVController`의 구현된 핵심 기능은 릴리스 후보 수준의 테스트 안정성을 확보했다. 전체 테스트 100%, line coverage 97.8%, function coverage 100%는 좋은 신호다. Critical 결함도 잔존하지 않는다.

다만 요구사항 전체 릴리스 관점에서는 다음 항목을 완료해야 한다.

| 우선순위 | 과제 | 기대 효과 |
|---|---|---|
| P0 | `docs/defect_list.md`의 DEF-BLD-001 상태를 최신 CTest 56/56 Passed 기준으로 재검증/갱신 | 결함 원장 신뢰도 회복 |
| P1 | `PressNumber(-1/10)` 정책 확정 및 Red 테스트 추가 | DEF-TV-003 해소 |
| P1 | `getCurrentCH()` 비정상 문자열 테스트 추가 | `stoi` 경계 리스크 제거 |
| P1 | 채널 검색/업/다운 API의 TDD Red 테스트 작성 후 구현 | 요구사항 시나리오 커버리지 100%에 접근 |
| P2 | CI에서 GCC gcov/lcov job 고정 | 커버리지 수치 재현성 확보 |
| P2 | `ChannelBuffer`를 `std::optional<int>` 또는 명명 상수로 리팩토링 | 센티널 기반 오류 가능성 축소 |

종합 판정은 **"구현된 핵심 범위는 QA Green, 전체 요구사항은 Partial Green"**이다. 다음 단계는 기능 확장보다 먼저 남은 정책성 결함과 커버리지 측정 기준을 고정하는 것이다.
