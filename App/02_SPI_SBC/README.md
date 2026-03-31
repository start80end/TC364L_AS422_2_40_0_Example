# Example 02: SBC (TLE9263) Configuration via SPI

이 예제는 AURIX TC364 MCU와 TLE9263 SBC 간의 SPI 통신을 통해 시스템을 동기식(Synchronous)으로 제어하고, 특히 MCU와 SBC의 보드 전원이 따로 사용하기때문에 일어나는 전원 인가 시점이 다른 환경에서의 안정적인 초기화 방법을 다룹니다.

## 📋 프로젝트 기본 명세 (Specifications)
- **MCU**: Infineon AURIX TC364L (SAK-TC364DP-64F300W)
- **SBC Unit**: Infineon TLE9263-3BQX (SPI Control)
- **Board**: AURIX TC3X4 TriBoard V1.2 + TLE9263 Evaluation Kit EVB2
- **MCAL**: MC-ISAR AS422 V2.40.0
- **Configuration Tool**: EB Tresos 26.2.0
- **Compiler**: TASKING TriCore v6.3r1
- **Debugger**: TRACE32 / Lauterbach

---

## 1. 하드웨어 사양 (Hardware Specification)

| 항목 (Item) | 상세 내용 (Details) | 비고 (Remarks) |
| :--- | :--- | :--- |
| **Interface** | QSPI3 (Master) | Synchronous Mode |
| **Frequency** | 1.0 MHz | SPI Clock |
| **SPI Mode** | Mode 1 (CPOL=0, CPHA=1) | |
| **Bit Order** | LSB First | SBC Protocol 준수 |
| **Frame Length** | 16-bit | 7-bit Addr + 1-bit R/W + 8-bit Data |

### 📍 핀 매핑 (Pin Mapping)
- **SCLK**: P22.0 (ALT3)
- **MRST**: P22.1 (Input)
- **MTSR**: P22.2 (ALT3)
- **CS**: P22.3 (SLS3, ALT3)
<img width="800" height="700" alt="image" src="https://github.com/user-attachments/assets/f95fa70e-8b3c-4c21-bff4-f1b87710b519" />

### ⏱️ SPI Timing Specifications (QSPI3 설정 기준)
안정적인 통신을 위해 실제 MCAL(Spi_PBcfg.c)에 반영된 타이밍 값입니다.
- **Leading Time**: **640 ns** (Enable CS to First CLK edge)
- **Trailing Time**: **640 ns** (Last CLK edge to Disable CS)
- **Idle Time**: **5.12 µs** (Min. delay between consecutive frames)

### ⚙️ SPI Clock Derivation (100MHz Peripheral Clock 기반)
- **f_OSC**: 20 MHz (External Crystal)
- **f_SOURCE**: 300 MHz (PLL Output)
- **f_QSPI**: 100 MHz (Peripheral Clock, f_SOURCE / 3)
- **f_BAUD 상세 계산**:
  - 수식: `f_BAUD = f_QSPI / (Q_eff * TQ_eff * (A + B + C + 1))`
  - 계산: `100 MHz / (2 * 25 * 2) = 1.0 MHz`
  - 파라미터: `(Q_eff = 2 [Q=1], TQ_eff = 25 [TQ=24], A+B+C+1 = 2)`

---

## 2. 핵심 설계 및 구동 원리 (Core Design)

### 🔋 별도 전원 환경 대응 (Alive Polling)
- **Challenge**: MCU가 부팅된 후 SBC 전원이 나중에 켜지는 경우, SBC의 초기 부팅 윈도우(200ms)를 놓치면 무한 리셋에 빠질 수 있습니다.
- **Solution**: `TaskScheduler`에서 100ms 주기로 `Sbc_Init()`을 호출하여 SBC의 응답(Status != 0x0000)을 감지하고 즉시 초기화 시퀀스를 수행합니다.

### 🛡️ 안전한 시작 (Safe Start-up)
- **Handshake**: 인피니언 권장 사양에 따라 첫 번째 SPI 프레임을 워치독 트리거(`WD_CTRL`)로 전송하여 SBC를 `Init Mode`에서 `Normal Mode`로 즉시 전환합니다.
- **Write-Verify**: `Sbc_WriteRegField`를 사용하여 설정 값을 쓰고, 즉시 다시 읽어 검증하는 방식을 통해 통신 신뢰성을 확보합니다.

---

## 3. 동작 모드 선택 (Operation Modes)

`App_Cfg.h`에서 아래 모드를 선택하여 테스트할 수 있습니다.

- **`SBC_MODE_DEVELOPER`**: 워치독 서비스를 중지(Stop)하여 Trace32 디버깅 시 세션이 끊기거나 리셋되는 것을 방지합니다.
- **`SBC_MODE_PRODUCTION`**: 실제 양산 환경과 동일하게 1초 주기로 워치독을 관리하여 시스템 안정성을 확인합니다.

---

## 🚀 테스트 가이드 (Test Guide)

1. **빌드 및 다운로드**: `SBC_MODE_DEVELOPER` 모드로 빌드 후 타겟에 로드합니다.
2. **부팅 순서**: MCU 전원을 먼저 인가한 상태에서 `Go`를 실행하고, 이후 SBC 전원을 켭니다.
3. **상태 확인**: `SPI_Command` 변수에 1(VCC2/3 ON) 혹은 2(OFF)를 입력하여 실물 전원이 제어되는지 확인합니다.
4. **모드 전환**: 초기 검증 완료 후 `SBC_MODE_PRODUCTION`으로 변경하여 장기적인 워치독 서비스 안정성을 테스트합니다.
