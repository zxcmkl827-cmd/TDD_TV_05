# Requirements Analysis

## 1. C++ 구현 관점 요구사항 요약

| 구분 | 요구사항 | C++ 구현 관점 정리 | 검증 포인트 |
|---|---|---|---|
| 채널 범위 | 일반 채널은 `0`부터 `99`까지이다. | 내부 계산은 `int`로 처리하되, `Tuner` 인터페이스 호출 시 `std::string`으로 변환한다. | `0`, `99`, `-1`, `100` 경계값을 분리 검증한다. |
| 숫자 입력 | 숫자 버튼은 `0`부터 `9`까지 입력된다. | 숫자 입력 버퍼를 별도로 유지한다. 버퍼가 비어 있으면 첫 자리 저장, 이미 있으면 두 자리 채널을 즉시 확정한다. | 한 자리+확인, 두 자리 자동 확정, 세 자리 이상 연속 입력을 검증한다. |
| 확인 입력 | 숫자 한 자리 입력 후 확인을 누르면 해당 채널로 변경된다. | `PressConfirm()`은 숫자 버퍼가 있을 때만 채널 변경을 수행하고, 이후 버퍼를 초기화한다. | 버퍼가 비어 있을 때 확인 입력은 채널을 바꾸지 않아야 한다. |
| 두 자리 입력 | `1`, `2`를 누르면 `12`번 채널로 변경된다. | 두 번째 숫자가 들어오면 `first * 10 + second`로 채널을 만든 뒤 즉시 `setCH()`를 호출한다. | `12`, `45`, `99`, `00` 처리 방식을 명확히 한다. |
| 연속 숫자 입력 | `1`, `2`, `3`, `4`는 `12` 변경 후 `34` 변경이다. | 두 자리마다 입력 버퍼를 비우고 다음 숫자를 새 첫 자리로 처리한다. | `setCH("12")`, `setCH("34")` 호출 순서를 검증한다. |
| 홀수 개 숫자 입력 | `4`, `5`, `6`은 `45` 변경 후 `6`이 대기 상태가 된다. | 남은 한 자리 버퍼는 확인을 누르면 확정하고, 그 외 버튼을 누르면 폐기한다. | `6`+확인은 `6`번, 다른 버튼은 버퍼 삭제를 검증한다. |
| 선행 0 | `0`, `7`은 `7`번 채널로 변경된다. | 숫자 계산 결과는 `7`이며, `Tuner`에는 `"7"`을 전달하는 것이 자연스럽다. | `"07"`이 아니라 `"7"`을 기대값으로 둔다. |
| 선호 채널 추가 | 현재 채널이 선호 채널이 아니면 추가한다. | `getCurrentCH()` 값을 숫자로 파싱하고 중복 없이 저장한다. 다음 선호 채널 탐색을 위해 정렬된 컨테이너 사용을 권장한다. | 추가 후 목록 포함 여부와 정렬 상태를 검증한다. |
| 선호 채널 삭제 | 현재 채널이 이미 선호 채널이면 삭제한다. | 추가/삭제는 토글 동작이다. `erase-remove` 또는 `std::set` 기반으로 중복을 방지한다. | 같은 채널에서 두 번 누르면 목록에서 사라져야 한다. |
| 다음 선호 채널 | 현재 채널보다 큰 선호 채널 중 가장 작은 값으로 이동한다. | `std::upper_bound`를 사용할 수 있다. 현재보다 큰 값이 없으면 가장 작은 선호 채널로 순환한다. | `6 -> 12`, `56 -> 1` 순환을 검증한다. |
| 채널 검색 | 채널 검색 버튼은 모든 시청 가능 채널을 검색하여 저장한다. | `Tuner::seekCH()`를 반복 호출해 검색 결과를 저장한다. 시작점 복귀, 중복 발견, 최대 100회 제한 등 종료 조건이 필요하다. | 무한 루프 방지와 중복 제거를 검증한다. |
| 업/다운: 검색 결과 없음 | 검색 결과가 없으면 일반 채널 범위에서 1씩 이동한다. | 업은 `(current + 1) % 100`, 다운은 `(current + 99) % 100`으로 처리할 수 있다. | `99 -> 0`, `0 -> 99` 순환을 검증한다. |
| 업/다운: 검색 결과 있음 | 검색 결과가 있으면 저장된 채널 중 다음/이전 채널로 이동한다. | 검색 결과 목록을 정렬된 고유 목록으로 관리한다. 현재 채널이 목록에 없어도 다음 큰 값 또는 이전 작은 값을 찾아야 한다. | `{4, 6, 14}`에서 `6 -> 14/4`, `15 -> 4/14`를 검증한다. |
| Tuner 의존성 | `Tuner` 구현은 외부 제공이며 정확히 동작한다고 가정한다. | Controller 테스트는 Fake 또는 Mock `Tuner`를 주입한다. `setCH`, `getCurrentCH`, `seekCH` 호출 계약을 검증한다. | Google Mock으로 호출 인자와 호출 횟수를 검증한다. |

## 2. 문자열 변경 시 주의사항

| 항목 | 주의사항 | C++17 구현 권장 |
|---|---|---|
| `std::to_string(int)` | 숫자 채널을 문자열로 바꿀 때 선행 0이 제거된다. `0`, `7` 입력 결과는 `"07"`이 아니라 `"7"`이다. | 채널 값은 정수 의미로 관리하고, `Tuner::setCH()` 호출 직전에만 `std::to_string()`을 사용한다. |
| `std::stoi(std::string)` | 빈 문자열, 숫자가 아닌 문자열, 범위를 벗어난 숫자 문자열에서 `std::invalid_argument` 또는 `std::out_of_range`가 발생할 수 있다. | `getCurrentCH()` 결과를 파싱하는 경계에서 예외 정책을 명확히 한다. Controller는 잘못된 Tuner 응답을 테스트에서 별도로 검증한다. |
| `remoteKey`의 `to_string()` | 프로젝트에 `remoteKey`용 `to_string(remoteKey)`가 있으므로 `std::to_string`과 이름 충돌 또는 오해가 생길 수 있다. | 숫자 변환은 반드시 `std::to_string(ch)`처럼 네임스페이스를 명시한다. |
| 채널 문자열 형식 | `Tuner::setCH()`는 문자열을 받지만 요구사항의 채널은 숫자 범위이다. `" 7"`, `"7abc"`, `"+7"`, `"-0"` 같은 문자열 허용 여부를 구현마다 다르게 해석할 수 있다. | Controller는 직접 만든 정상 문자열만 `setCH()`에 전달한다. 외부 입력 문자열을 받는 계층이 생기면 형식 검증을 별도 함수로 분리한다. |
| 기대값 비교 | Mock 테스트에서 `setCH("7")`과 `setCH("07")`은 다른 호출이다. | 테스트 기대값은 요구사항의 숫자 의미와 구현의 문자열 직렬화 정책을 일치시킨다. |
| 예외 메시지 | 예외 메시지는 구현마다 달라질 수 있다. | Google Test에서는 메시지 문자열보다 예외 타입과 상태 불변성을 우선 검증한다. |

## 3. 예외/경계값 조건

| 조건 | 기대 동작 | 테스트 관점 |
|---|---|---|
| 채널 `0` | 유효 채널이며 설정 가능하다. | `setCH("0")`, 채널 다운 순환의 도착값으로 검증한다. |
| 채널 `99` | 유효 채널이며 설정 가능하다. | `setCH("99")`, 채널 업 순환의 시작값으로 검증한다. |
| 채널 `-1` 이하 | 유효하지 않은 채널이다. | `std::invalid_argument` 발생 또는 `Tuner::setCH()` 미호출을 검증한다. |
| 채널 `100` 이상 | 유효하지 않은 채널이다. | `100`, `9999`를 대표값으로 검증한다. |
| 숫자 버튼 인자 `0~9` 밖 | 리모컨 숫자 입력으로 볼 수 없다. | Controller API가 `int`를 받는다면 `-1`, `10` 입력에 대한 예외 또는 무시 정책을 정한다. |
| 버퍼 없음 + 확인 | 확정할 숫자가 없다. | 현재 채널 유지, `setCH()` 미호출을 검증한다. |
| 버퍼 있음 + 숫자 외 버튼 | 대기 중인 한 자리 숫자는 무효화된다. | 이후 확인을 눌러도 이전 숫자가 적용되지 않아야 한다. |
| 선호 채널 목록 비어 있음 | 다음 선호 채널 이동 대상이 없다. | no-op, `setCH()` 미호출을 검증한다. |
| 선호 채널 목록 1개 | 다음 선호 채널은 자기 자신으로 순환할 수 있다. | 현재 채널과 같거나 다른 경우 모두 기대 동작을 고정한다. |
| 검색 결과 비어 있음 | 업/다운은 전체 일반 채널 기준으로 동작한다. | 검색 전 상태와 검색 결과 없음 상태를 구분해 검증한다. |
| 검색 결과 중복 | 저장 목록에는 중복이 없어야 한다. | `seekCH()`가 같은 채널을 반복 반환하는 Fake로 검증한다. |
| 검색 종료 조건 | `seekCH()` 반복이 끝나야 한다. | 최대 100회 제한 또는 시작 채널 재방문 조건을 검증한다. |
| `getCurrentCH()`가 비정상 문자열 반환 | `stoi` 예외가 발생할 수 있다. | Mock으로 `""`, `"abc"`, `"100"` 등을 반환시켜 Controller 정책을 검증한다. |

## 4. Google Test 기준 테스트 시나리오 목록

1. `TunerTest.ValidChannel_0_99`: `setCH("0")`, `setCH("99")`가 정상 처리되는지 검증한다.
2. `TunerTest.InvalidChannel_OutOfRange`: `"-1"`, `"100"`, `"9999"` 입력 시 `std::invalid_argument`가 발생하는지 검증한다.
3. `TunerTest.SeekCH_ReturnsValidChannel`: `seekCH()` 반환값이 `0~99` 범위인지 검증한다.
4. `ControllerTest.PressNumber_OneDigitThenConfirm`: `1`, 확인 입력 시 `setCH("1")` 또는 현재 채널 `"1"`을 검증한다.
5. `ControllerTest.PressNumber_TwoDigitsAutoApply`: `1`, `2` 입력 시 확인 없이 `12`번으로 변경되는지 검증한다.
6. `ControllerTest.PressNumber_FourDigitsPairwiseApply`: `1`, `2`, `3`, `4` 입력 시 `12`, `34` 순서로 변경되는지 검증한다.
7. `ControllerTest.PressNumber_ThreeDigitsKeepsLastDigitBuffered`: `4`, `5`, `6`, 확인 입력 시 `45` 변경 후 `6`번으로 변경되는지 검증한다.
8. `ControllerTest.PressNumber_ThreeDigitsThenOtherClearsBuffer`: `4`, `5`, `6`, 기타 버튼, 확인 입력 시 `6`이 적용되지 않는지 검증한다.
9. `ControllerTest.PressNumber_LeadingZero`: `0`, `7` 입력 시 `setCH("7")`이 호출되는지 검증한다.
10. `ControllerTest.PressConfirm_WithoutBufferedDigitDoesNothing`: 버퍼가 비어 있을 때 확인 입력이 채널을 변경하지 않는지 검증한다.
11. `ControllerTest.PressNumber_InvalidDigitRejected`: 숫자 버튼 API에 `-1` 또는 `10`이 들어온 경우 예외 또는 무시 정책을 검증한다.
12. `ControllerTest.PressFavorite_AddsCurrentChannel`: 현재 채널이 선호 목록에 없으면 추가되는지 검증한다.
13. `ControllerTest.PressFavorite_RemovesExistingChannel`: 현재 채널이 이미 선호 목록에 있으면 삭제되는지 검증한다.
14. `ControllerTest.PressFavorite_KeepsFavoritesSortedAndUnique`: 여러 채널을 추가/삭제해 선호 목록이 정렬되고 중복이 없는지 검증한다.
15. `ControllerTest.PressNextFavorite_SelectsNearestGreaterChannel`: 선호 목록 `{1, 4, 12, 56}`, 현재 `6`에서 `12`로 이동하는지 검증한다.
16. `ControllerTest.PressNextFavorite_WrapsToSmallestChannel`: 선호 목록 `{1, 4, 12, 56}`, 현재 `56`에서 `1`로 순환하는지 검증한다.
17. `ControllerTest.PressNextFavorite_EmptyListDoesNothing`: 선호 목록이 비어 있으면 `setCH()`가 호출되지 않는지 검증한다.
18. `ControllerTest.PressChannelSearch_StoresAvailableChannels`: 채널 검색 버튼 입력 시 `seekCH()` 결과가 저장되는지 검증한다.
19. `ControllerTest.PressChannelSearch_DeduplicatesAndTerminates`: `seekCH()`가 중복을 반환해도 검색이 종료되고 목록이 중복 없이 유지되는지 검증한다.
20. `ControllerTest.PressUp_NoSearchResultMovesToNextGeneralChannel`: 검색 결과가 없고 현재 `6`이면 업 입력 시 `7`로 이동하는지 검증한다.
21. `ControllerTest.PressDown_NoSearchResultMovesToPreviousGeneralChannel`: 검색 결과가 없고 현재 `6`이면 다운 입력 시 `5`로 이동하는지 검증한다.
22. `ControllerTest.PressUp_NoSearchResultWraps99To0`: 검색 결과가 없고 현재 `99`이면 업 입력 시 `0`으로 이동하는지 검증한다.
23. `ControllerTest.PressDown_NoSearchResultWraps0To99`: 검색 결과가 없고 현재 `0`이면 다운 입력 시 `99`로 이동하는지 검증한다.
24. `ControllerTest.PressUp_WithSearchResultMovesToNextStoredChannel`: 검색 결과 `{4, 6, 14}`, 현재 `6`에서 업 입력 시 `14`로 이동하는지 검증한다.
25. `ControllerTest.PressDown_WithSearchResultMovesToPreviousStoredChannel`: 검색 결과 `{4, 6, 14}`, 현재 `6`에서 다운 입력 시 `4`로 이동하는지 검증한다.
26. `ControllerTest.PressUp_WithSearchResultWrapsWhenCurrentNotInList`: 검색 결과 `{4, 6, 14}`, 현재 `15`에서 업 입력 시 `4`로 이동하는지 검증한다.
27. `ControllerTest.PressDown_WithSearchResultWrapsWhenCurrentNotInList`: 검색 결과 `{4, 6, 14}`, 현재 `15`에서 다운 입력 시 `14`로 이동하는지 검증한다.
28. `ControllerMockTest.SetCH_UsesCanonicalString`: 숫자 입력 결과가 `std::to_string` 기준 문자열로 `Tuner::setCH()`에 전달되는지 검증한다.
29. `ControllerMockTest.GetCurrentCH_ParseErrorPolicy`: `getCurrentCH()`가 `""` 또는 `"abc"`를 반환할 때 Controller 예외 정책을 검증한다.
30. `ControllerMockTest.DoesNotCallTunerOnNoOpInputs`: 확인 단독, 선호 목록 없음, 버퍼 폐기 등 no-op 시나리오에서 `setCH()` 미호출을 검증한다.

