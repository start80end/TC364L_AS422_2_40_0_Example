# Example 02: SBC (TLE9263) Configuration via Asynchronous SPI

이 예제는 AURIX TC364 MCU와 TLE9263 SBC 간의 SPI 통신을 **비동기식(Asynchronous, Non-blocking)** 상태 머신 기반으로 제어하는 방법을 다룹니다. 특히 MCU와 SBC의 전원 인가 시점이 다른 환경에서의 안정적인 초기화와 통신 지연에 의한 시스템 행(Hang) 방지 로직을 포함합니다.

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
| **Interface** | QSPI3 (Master) | Asynchronous Mode (Non-blocking) |
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

---

## 2. 핵심 설계 및 구동 원리 (Core Design & Architecture)

### 🔄 비차단형 상태 머신 (STM-based State Machine)
- **Challenge**: SPI 통신 중 Wait/Delay가 발생하면 전체 시스템 루프가 멈추거나 다른 태스크(LED 등)가 영향을 받습니다.
- **Solution**: `STM0` 타이머와 `Sbc_SPI_Test()` 상태 머신을 사용하여 대기 시간(부팅 1.5s, 명령 간 2s) 동안 CPU를 점유하지 않고 비동기적으로 시퀀스를 처리합니다.

### 🚀 DMA 기반 데이터 전송 (Direct Memory Access)
비동기식 SPI의 성능을 극대화하기 위해 DMA를 사용하여 CPU 부하를 제거했습니다.
- **DMA Channel 0 (TX)**: QSPI3 전송 요청 시 메모리의 TX 버퍼 데이터를 하드웨어 전송 레지스터로 자동 이동.
- **DMA Channel 1 (RX)**: QSPI3 수신 완료 시 수신 레지스터 데이터를 지정된 RX 버퍼로 자동 복사.
- **장점**: 대량의 데이터 전송 시에도 CPU는 인터럽트가 발생할 때까지 다른 연산을 수행할 수 있습니다.

### 🛡️ MCAL 표준 인터럽트 구조 (Standard IRQ Framework)
프로젝트 유지보수성을 위해 커스텀 ISR 기술 방식 대신 **Infineon 공식 MCAL SSC 원본 모듈**을 채택했습니다.
- **적용 모듈**: `Spi_Irq.c`, `Dma_Irq.c` (Drivers/Mcal/ssc/irq 하위)
- **구동 원리**: Tresos에서 설정된 Priority와 TOS(Type of Service) 정보를 바탕으로 하드웨어 벡터 테이블이 자동으로 해당 MCAL 핸들러를 호출합니다.

---

## 3. 비동기식/DMA 사용 시 주요 고려 사항 (Technical Considerations)

### ⚠️ 인터럽트 우선순위 및 TOS (Type of Service)
- DMA 채널의 인터럽트 우선순위는 SPI 하드웨어 우선순위보다 높게 설정하여 수신 데이터 유실을 방지해야 합니다.
- `TOS` 설정에 따라 특정 CPU 코어가 인터럽트를 전담하게 되며, 본 프로젝트는 **Core 0(TOS=0)**에서 일괄 처리합니다.

### 🔗 DMA 하드웨어 요청 활성화 (SRE - Service Request Enable)
- MCAL `Spi_Init()`만으로는 하드웨어 전송 요청이 DMA까지 전달되지 않을 수 있습니다. 
- `Cpu0_Main.c`의 초기화 단계에서 `SRC_QSPI3TX`, `SRC_QSPI3RX` 레지스터의 **SRE(Service Request Enable)** 비트를 명시적으로 `1`로 설정해야 비동기 전송이 원활하게 시작됩니다.

### 🛡️ 타임아웃 복구 로직 (Error Recovery)
- 비동기 전송 특성상 하드웨어 결함으로 인해 `SPI_SEQ_PENDING` 상태가 무한 지속될 수 있습니다.
- 루프 내에 강제 타임아웃 카운터를 두어, 일정 시간 경과 시 `Spi_DeInit/Init`을 통해 드라이버를 초기 상태로 강제 복구(Cold Booting)하는 로직을 포함했습니다.

---

## 🚀 테스트 가이드 (Test Guide)

1. **빌드 및 다운로드**: `SBC_MODE_DEVELOPER` 모드로 빌드 후 타겟에 로드합니다.
2. **비동기 시퀀스 확인**: `Go` 실행 후 SBC 전원을 켭니다. 상태 머신이 STM 타이머에 의해 자동으로 초기화 시퀀스를 진행합니다.
3. **병렬 태스크 확인**: SPI 제어 시퀀스 중간 대기 시간에도 LED Toggling 등 다른 백그라운드 태스크가 멈추지 않고 동작하는지 확인합니다.
4. **상태 확인**: `s_testState` 및 `g_SbcError` 변수를 모니터링하여 전체 시퀀스가 `SBC_STATE_DONE`까지 정상 도달하는지 확인합니다.
