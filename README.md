# 🚀 AURIX TC364L MCAL Starter Project (AS422 V2.40.0)

이 프로젝트는 Infineon **AURIX TC364L** 마이크로컨트롤러 환경에서 **AUTOSAR MCAL(AS422 V2.40.0)**의 기초적인 사용법과 초기화 과정을 학습하기 위한 베이스 예제입니다. 

---

## 📋 프로젝트 기본 명세 (Specifications)
| 항목 | 상세 내용 |
| :--- | :--- |
| **MCU** | Infineon **AURIX TC364L** (SAK-TC364DP-64F300W) |
| **Core** | TriCore™ TC1.6.2P |
| **Board** | AURIX TC3X4 TriBoard V1.2 |
| **MCAL** | Infineon MC-ISAR AS422 V2.40.0 |

---

## 🛠️ 개발 환경 및 도구 (Development Environment)
- **Configuration Tool**: **EB Tresos 26.2.0** (MCAL 설정 및 코드 생성)
- **Compiler**: **TASKING TriCore v6.3r1** (Tasking IDE 사용)
- **Debugger**: **TRACE32 / Lauterbach** (CMM 스크립트 기반 자동 로드)

---

## 📂 프로젝트 폴더 구조 (Directory Structure)

| 폴더명 | 설명 |
| :--- | :--- |
| **`App/Main/`** | 프로젝트 진입점(`Cpu0_Main.c`), 전역 스케줄러, 시스템 설정(`App_Cfg.h`) |
| **`App/XX_Feature_Name/`** | **기능별 독립 예제 폴더** (예: GTM_LED, SPI_SBC, ADC, PWM 등) |
| **`Config/`** | EB Tresos 프로젝트 구성 파일 및 자동 생성된(Generated) 코드 |
| **`Drivers/`** | MCAL 정적 라이브러리, SFR 레지스터 정의 및 로우레벨 인프라 드라이버 |
| **`Debug/`** | 빌드 결과물(ELF, HEX, MAP) 및 컴파일 관련 출력 파일 |

---

## 🔍 시작하기 (Quick Start)
1. Tasking IDE에서 본 프로젝트 폴더를 활성화합니다.
2. `App/Main/App_Cfg.h`에서 테스트하고자 하는 예제의 매크로 스위치(USE_EXAMPLE_xxx)를 활성화합니다.
3. 빌드를 수행한 후 Trace32를 통해 생성된 `.elf` 파일을 타겟 보드에 로드하여 실행합니다.
