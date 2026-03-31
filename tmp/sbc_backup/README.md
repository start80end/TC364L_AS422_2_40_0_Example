# 🚀 Example 02: SBC (TLE9263) Configuration via SPI

이 예제는 AURIX **TC364L** MCU와 **TLE9263-3BQX** SBC(System Basis Chip) 간의 SPI 통신을 통해 시스템을 안전하게 초기화하고 관리하는 방법을 설명합니다. 특히 MCU와 SBC의 전원을 개별적으로 인가하는 환경과 디버깅 편의성을 위한 수동 제어 방식을 제시합니다.

---

## 📋 프로젝트 기본 명세 (Specifications)
- **MCU**: Infineon AURIX **TC364L** (SAK-TC364DP-64F300W)
- **SBC Unit**: Infineon **TLE9263-3BQX** (SPI Control)
- **Board**: AURIX TC3X4 TriBoard V1.2 + TLE9263 Evaluation Kit EVB2
- **MCAL**: MC-ISAR AS422 V2.40.0
- **Configuration Tool**: EB Tresos 26.2.0
- **Compiler**: TASKING TriCore v6.3r1
- **Debugger**: TRACE32 / Lauterbach

---

## 🛠️ 개발 환경 및 설정 (Development Setup)

### 1. SPI Hardware Specification & Pin Mapping
| 항목 | 명세 (Specification) |
| :--- | :--- |
| **Interface** | QSPI3 (Master) |
| **Frequency** | 1.0 MHz |
| **SPI Mode** | Mode 1 (CPOL=0, CPHA=1) |
| **Bit Order** | **LSB First** |
| **Pin Mapping** | **SCLK**: P22.0 / **MRST**: P22.1 / **MTSR**: P22.2 / **CS**: P22.3 |

#### [하드웨어 연결도 (Pin Connection)]
[SPI Pin Connection Diagram]
<img width="800" height="700" alt="image" src="https://github.com/user-attachments/assets/f95fa70e-8b3c-4c21-bff4-f1b87710b519" />

### 2. SPI Clock Derivation (100MHz Peripheral Clock 기반)
- **f_OSC**: 20 MHz (External Crystal)
- **f_SOURCE**: 300 MHz (PLL Output)
- **f_QSPI**: 100 MHz (Peripheral Clock, f_SOURCE / 3)
- **f_BAUD 인스턴스 계산**:
  - `f_BAUD = f_QSPI / (Q_eff * TQ_eff * (A + B + C + 1))`
  - `100 MHz / (2 * 25 * 2) = 1.0 MHz`
  - *(Q_eff=2[Q=1], TQ_eff=25[TQ=24], A+B+C+1=2)*

---

## 💡 프로토콜 명세 (Protocol Specification)

### Frame Structure (16-bit)
- **TX Frame (MCU -> SBC)**:
  - `[0:6]` Address
  - `[7]` R/W (Read=0, Write=1)
  - `[8:15]` Data
- **RX Frame (SBC -> MCU)**:
  - `[0:7]` Status Word (Global Status Byte)
  - `[8:15]` Data Content

---

## 🚀 예제 상세 가이드 (Example Details)

### [Point 01] 별도 전원 환경 대응 (Safe Start-up)
- **전원 분리**: MCU(Triboard)와 SBC(EVKIT)의 전원이 따로 들어오는 상황을 고려합니다.
- **초기 핸드셰이크**: 첫 SPI 명령으로 워치독 트리거(`WD_CTRL`)를 전송하여 SBC를 즉시 `Normal Mode`로 진입시킵니다.

### [Point 02] 동작 모드 선택 (SBC Mode)
- **`SBC_MODE_DEVELOPER`**: 자동 통신을 하지 않습니다. SBC 전원 투입 후 사용자가 **`SPI_Command = 1`**을 수동 입력할 때만 초기화를 수행하여 불필요한 SPI 노이즈를 방지합니다.
- **`SBC_MODE_PRODUCTION`**: 양산용 모드입니다. 100ms마다 자동으로 SBC를 감지하여 초기화하고 1초 주기로 워치독 서비스를 관리합니다.

---

## 📂 폴더 구조 및 주요 소스 (Structure)

| 파일명 | 설명 |
| :--- | :--- |
| `Sbc_SPI_Control.c` | TLE9263 레지스터 제어 및 SPI 송수신 로직 |
| `Sbc_SPI_Control.h` | SBC 상수 정의 및 레지스터 맵 |
| `App_Cfg.h` | SBC 모드 및 예제 스위치 관리 |

---

## 🔍 테스트 방법 및 관찰 포인트 (Usage)

1. **개발자 모드 테스트**:
   - MCU 실행 후 SPI 파형이 없는 것을 확인하고, **`SPI_Command = 1`**을 입력하여 초기화가 1회 수행되는지 관찰합니다.
2. **양산 모드 테스트**:
   - `SBC_MODE_PRODUCTION` 설정 시 SBC 전원을 켜는 즉시 자동으로 통신이 시작되는지 확인합니다.
3. **SPI 신호 관찰**: 
   - 오실로스코프 또는 논리 분석기로 QSPI3 핀들의 파형이 규격(1MHz, Mode 1)에 맞는지 확인합니다.

---

> [!NOTE]
> 이 예제는 Infineon TLE9263 칩셋 전용이며, 타 칩셋 사용 시 레지스터 맵 수정이 필요할 수 있습니다.
