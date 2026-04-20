# UE5 3D Flight Shooter — C++ + Python

Unreal Engine 5.7 기반 3D 플라이트 슈팅 게임입니다. 

---

## 포함된 기능

| 기능 | 구현 위치 |
| --- | --- |
| HP / 피격 무적 / 사망 이벤트 | `Source/Interior/Public/Components/HealthComponent.h` |
| 데미지 인터페이스 | `Source/Interior/Public/Combat/Damageable.h` |
| 3인칭 ↔ 1인칭 카메라 전환 (기본 키: `V`) | `AFlightPawn` (SpringArm TPS + Cockpit FPS) |
| 점수 · 콤보 · SaveGame 최고점수 | `AShootingGameMode`, `UShootingSaveGame` |
| 웨이브 매니저 (DataAsset 기반) | `AEnemyManager` + `UWaveDataAsset` |
| 보스 AI (패턴 3종 + Phase2 전환) | `ABossEnemy` |
| UMG HUD (ProgressBar / 점수 / 콤보 / 웨이브) | `UPlayerHUDWidget` |
| Editor Python 자동 세팅 | `Tools/ue_editor/setup_project.py` |

기본 입력(마우스+WASD 비행, 좌클릭 발사, Q/E 롤, Shift 부스트, V 카메라 전환)은 BP에서
Enhanced Input Asset 으로 쉽게 연결할 수 있게 모두 `UInputAction` UPROPERTY 로 노출
되어 있습니다.

---

## 최초 실행 순서 (1회만)

### 1. Visual Studio 프로젝트 파일 생성
Windows 탐색기에서 `Interior.uproject` 에 **우클릭 → "Generate Visual Studio project files"**.

### 2. C++ 빌드
- `Interior.sln` 을 Visual Studio 2022 로 열기
- Solution Configuration: **Development Editor**, Platform: **Win64**
- Build → Build Solution (또는 F7). 최초 빌드는 2–5분.

### 3. 에디터 실행
`Interior.uproject` 더블클릭.
(에디터가 뜨면 **Output Log** 창도 열어두세요. `LogInterior` 카테고리 확인용.)

### 4. Blueprint · DataAsset 자동 생성 (Python)
에디터 상단 메뉴 **Tools → Execute Python Script…** 에서
`Tools/ue_editor/setup_project.py` 를 선택하면 다음이 자동 생성됩니다:

```
Content/Blueprints/Generated/
    BP_Flight          (AFlightPawn 상속)
    BP_EnemyFighter    (AEnemyBase 상속)
    BP_BossEnemy       (ABossEnemy 상속)
    BP_Bullet_Player   (AProjectileBase 상속)
    BP_Bullet_Enemy    (AProjectileBase 상속)
    BP_EnemyManager    (AEnemyManager 상속)
    BP_ShootingGM      (AShootingGameMode 상속)
Content/UI/Generated/
    WBP_PlayerHUD      (UPlayerHUDWidget 상속)
    WBP_GameOver       (UUserWidget 상속)
Content/Data/Generated/
    DA_Enemy_Scout / Fighter / Bomber / Boss01 / Boss02   (UEnemyStatsAsset)
    DA_Wave_01..06                                        (UWaveDataAsset)
```
`BP_EnemyManager` 의 **Waves** 배열에 생성된 웨이브 에셋들도 자동으로 채워집니다.

### 5. 프로젝트 세팅
**Project Settings → Maps & Modes** 에서
- *Default GameMode* 를 `BP_ShootingGM` 으로 교체
- *Game Default Map* 을 `/Game/Maps/ShootingMap` 으로 교체 (또는 새 맵 생성)

### 6. Enhanced Input 바인딩 (한 번만)
`Content/Input/` 에 `IA_Move`, `IA_Look`, `IA_Roll`, `IA_Fire`, `IA_ToggleCamera`,
`IA_Boost` 와 `IMC_Flight` 를 만들어 주세요.
각 InputAction 을 `BP_Flight` 의 Details 패널 *Flight|Input* 카테고리에 드래그하면 끝.

권장 키 맵핑:

| Action | Key | Value Type |
| --- | --- | --- |
| IA_Move | WASD (2D 축) | Axis2D |
| IA_Look | Mouse XY | Axis2D |
| IA_Roll | Q (−1) / E (+1) | Axis1D |
| IA_Fire | Left Mouse | Digital |
| IA_ToggleCamera | **V** | Digital |
| IA_Boost | Left Shift | Digital |

### 7. 레벨에 배치
1. `ShootingMap` 을 열어 `BP_EnemyManager` 를 월드에 드래그.
2. `BP_EnemyManager` 디테일 패널 **Spawn Points** 배열에 TargetPoint 3~5개 지정
   (비워 두면 플레이어 앞쪽에서 자동 스폰).
3. 월드에 `PostProcessVolume` 을 놓고 *Infinite Extent* 체크 (저체력 비네팅용, 선택).

### 8. Play!
에디터 상단 **Play ▶** 버튼. 콘솔에서 아래 로그가 뜨면 정상:
```
LogInterior: Camera view: Third-Person
LogInterior: Loaded HighScore: 0
```

---

## 조작키

| 입력 | 동작 |
| --- | --- |
| W / S | 가속 / 감속 (Tick에서 MaxSpeed 로 수렴) |
| A / D | 좌우 스트레이프 |
| 마우스 | 피치 / 요 |
| Q / E | 롤 (좌 / 우) |
| Left Mouse | 발사 (Hold 시 자동 연사, `FireInterval` 설정값) |
| **V** | **3인칭 ↔ 1인칭 카메라 전환** |
| Shift | 부스트 (MaxSpeed × 1.8) |

`WeaponLevel` 을 Blueprint에서 1→2→3 으로 바꾸면 총알 스프레드가 늘어납니다 (Muzzle 3개 활용).

---

## 파일 구조

```
Interior/
├─ Interior.uproject                    # 모듈 & 플러그인 활성화
├─ Source/
│  ├─ Interior.Target.cs
│  ├─ InteriorEditor.Target.cs
│  └─ Interior/
│     ├─ Interior.Build.cs
│     ├─ Public/
│     │  ├─ Interior.h                  # Log category
│     │  ├─ Combat/Damageable.h
│     │  ├─ Components/HealthComponent.h
│     │  ├─ Pawns/FlightPawn.h
│     │  ├─ Projectiles/ProjectileBase.h
│     │  ├─ Enemies/{EnemyBase,BossEnemy,EnemyManager}.h
│     │  ├─ Data/{EnemyStatsAsset,WaveDataAsset}.h
│     │  ├─ GameMode/{ShootingGameMode,ShootingSaveGame}.h
│     │  └─ UI/PlayerHUDWidget.h
│     └─ Private/                       # 위 헤더들의 .cpp
├─ Tools/
│  └─ ue_editor/
│     ├─ setup_project.py               # 전부 자동 세팅
│     ├─ enemy_stats.csv                # 적 스탯 테이블
│     └─ waves.json                     # 웨이브 난이도 커브
└─ Content/                             # 기존 에셋 + 생성 결과
```

---

## 자주 겪는 이슈

### "Could not load /Script/Interior.FlightPawn"
C++ 모듈이 아직 컴파일되지 않았습니다. 위 **2단계** 를 먼저 수행하세요.

### 에디터에 `Generate Visual Studio project files` 항목이 안 보임
설치된 Visual Studio 에 "C++ by Game Development with Unreal Engine" 워크로드가
빠져 있습니다. Visual Studio Installer 에서 추가 설치하세요.

### `IA_Move` 등을 BP에 연결 안 했는데 움직이지 않음
위 6단계 참고. C++ 쪽은 입력 바인딩 구조만 잡아두고, 실제 InputAction 에셋은
프로젝트에서 직접 만들어야 합니다 (Blueprint 수정만 요함).

### 보스를 잡았는데 다음 웨이브가 안 시작
`BP_EnemyManager` 의 *WaveGap* 이 0이면 곧장 넘어가지만, 기본 2.5s 딜레이가 있습니다.
또한 보스 BP 가 `ABossEnemy` 가 아니라 `AEnemyBase` 를 상속한 경우 OnDied 바인딩이
보스에는 걸리지 않습니다 — 반드시 `BP_BossEnemy` 를 웨이브 `BossClass` 에 지정하세요.

---

## 확장 아이디어

- `UEnemyStatsAsset::DropTable` 필드를 추가해 아이템(힐/무기업/실드) 확률 드랍 구현.
- `UNiagaraSystem` 레퍼런스를 DataAsset 으로 빼서 적마다 다른 폭발 이펙트 부여.
- `AShootingGameMode::OnPlayerKilled` 델리게이트를 Blueprint Level Blueprint 에서
  구독해 슬로우 모션 + 카메라 줌으로 게임오버 연출.
- Python 쪽에 몬테카를로 밸런싱 시뮬레이터(`Tools/balancing/`) 추가 — 본 과제 명세서
  Part B 참조.

---

## 라이선스 / 크레딧
`Content/Drone`, `Content/SpaceShip`, `Content/Fire_EXP_Vol01_Free`, `star-wars-blaster`
등은 Epic 프리셋 또는 Marketplace 무료 에셋입니다. 재배포 시 원저작자 약관을
따르세요.

게임플레이 · 자동화 코드: Yeop (kkuing.ai@gmail.com), 2026.
