# 🚀 Example 01: LED Toggle with GTM Timer

이 예제는 AURIX **TC364L** 마이크로컨트롤러의 **GTM(Generic Timer Module)**을 활용하여 시스템 표준 타이머를 구축하고, LED를 주기적으로 제어하는 방법을 다룹니다.

---

## 📋 프로젝트 기본 명세 (Specifications)
- **MCU**: Infineon AURIX **TC364L** (SAK-TC364DP-64F300W)
- **Board**: AURIX TC3X4 TriBoard V1.2
- **MCAL**: MC-ISAR AS422 V2.40.0
- **Configuration Tool**: EB Tresos 26.2.0
- **Compiler**: TASKING TriCore v6.3r1
- **Debugger**: TRACE32 / Lauterbach

---

## 🛠️ 개발 환경 및 설정 (Development Setup)

### 1. 주요 MCAL 모듈 (Configured Modules)
- **MCU**: 시스템 클럭(fSYS=100MHz), PLL 설정 및 클럭 배분
- **PORT & DIO**: LED1(P33.4), LED2(P33.5) 설정 및 제어
- **GPT & GTM**: 1ms 단위의 시스템 틱 생성을 위한 GTM TOM 설정
- **IRQ / MCL**: 타이머 인터럽트(GTM) 서비스 요청 및 엔진 관리

### 2. 클럭 구성 (Clock Configuration)
- **External Crystal**: 20 MHz
- **Target System Frequency (fSYS)**: 100 MHz
- **Timer Source (fGTM)**: 100 MHz (기본 분주율 적용)

---

## 💡 예제 상세 가이드 (Example Details)
- **Timer Module**: **GTM TOM0** (Timer Output Module)
- **GPT Channel**: Ch 0 (System Tick Generation)
- **Tick Period**: **1 ms** (1000 Hz)
- **Target LED**: 
  - **LED1 (P33.4)**: 100ms Interval Toggle
  - **LED2 (P33.5)**: 1000ms Interval Toggle

---

## 🛠️ 주요 기능 (Main Features)

### 1. GTM 기반 협동형 스케줄러 (Cooperative Scheduler)
- GTM TOM 엔진을 통해 생성된 1ms 인터럽트를 기반으로 소프트웨어 카운터를 증폭시킵니다.
- Modulo 연산으로 정확한 주기에 태스크를 실행합니다.

### 2. 주기적 태스크 실행
- **100ms Task**: LED1 상태 반전
- **1000ms Task**: LED2 상태 반전

---

## 📂 폴더 구조 및 주요 소스 (Structure)

| 파일명 | 설명 |
| :--- | :--- |
| `LedToggleTask.c` | LED 하드웨어 제어 루틴 (P33.4, P33.5) |
| `TaskScheduler.c` | 1ms 틱 관리 및 주기별 태스크 실행 엔진 |
| `TaskScheduler.h` | 스케줄러 구조체 및 태스크 주기 정의 |
| `App_Cfg.h` | 예제 활성화 및 모드 설정 상수 관리 |

---

## 🔍 테스트 방법 및 관찰 포인트 (Usage)

1. **빌드 및 실행**: 프로젝트를 컴파일한 후 Trace32를 통해 타겟 보드에 로드합니다.
2. **LED 동작 확인**:
   - 보드 상의 **LED1**이 매우 빠르게(100ms) 깜빡이는지 확인합니다.
   - **LED2**가 1초(1000ms)마다 상태가 반전되는지 확인합니다.
3. **변수 모니터링**: 
   - Trace32의 Watch창에 `TaskControl` 구조체를 등록합니다.
   - `TickCount`가 실시간으로 증가하며, `TaskRun`에 현재 실행 중인 태스크 주기가 표시되는지 관찰합니다.

---

> [!NOTE]
> 이 프로젝트는 모든 타이밍 소스를 GTM GPT 드라이버를 통해 조절하며, 시스템의 안정적인 주기 제어를 목표로 합니다.
