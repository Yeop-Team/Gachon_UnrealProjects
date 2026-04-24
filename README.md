# UE5 Shooting Game Blueprint Only

UE5로 제작한 블루프린트 전용 슈팅 게임입니다.  
이 프로젝트는 **C++ 없이 Blueprint만으로 구현**되어 있으며, `lody`, `load`, `game` 3개 맵을 기준으로 구성되어 있습니다.

## 프로젝트 개요

- `lody`
  - 로비 맵
  - 게임 시작 버튼
  - 게임 종료 버튼

- `load`
  - 캐릭터 선택 맵
  - 빨간색 / 파란색 선택
  - 선택 후 게임 맵으로 이동

- `game`
  - 실제 플레이 맵
  - 플레이어 이동
  - 총알 발사
  - 적 / 보스 생성
  - 체력, 점수, 게임 종료 처리

## 조작법

- `W / A / S / D`
  - 이동

- `Left Mouse Button`
  - 발사

- `V`
  - 1인칭 / 3인칭 카메라 전환

- `ESC` 또는 `TAB`
  - 메뉴 열기 / 닫기

## 블루프린트 구성

- `BP_GameInstance`
  - 캐릭터 선택값 저장

- `BP_GameMode`
  - 게임 시작 / 종료 처리
  - 보스 처치 후 종료 UI 표시

- `BP_Player`
  - 플레이어 이동
  - 발사
  - 카메라 전환

- `BP_Player_Red`
  - 빨간 팀 플레이어

- `BP_Player_Blue`
  - 파란 팀 플레이어

- `BP_Enemy`
  - 일반 적

- `BP_Boss`
  - 보스
  - 체력 감소
  - 보스 HP UI 연동

- `WBP_Lobby`
  - 로비 UI

- `WBP_Load`
  - 캐릭터 선택 UI
  - 로딩 UI

- `WBP_HP`
  - 플레이어 체력 UI

- `WBP_보스`
  - 보스 체력 UI

- `WBP_ESC`
  - ESC / TAB 메뉴 UI

- `WBP_끗`
  - 게임 클리어 UI

- `WBP_실패`
  - 게임 오버 UI

## 주요 기능

- 로비에서 게임 시작 / 종료
- 캐릭터 선택 후 게임 진입
- `GameInstance`를 이용한 선택값 유지
- 적 생성 및 전투
- 보스 생성 및 체력 관리
- 보스 HP UI 연동
- 보스 처치 시 게임 종료 UI 표시
- `V` 키를 통한 1인칭 / 3인칭 카메라 전환
- `ESC` / `TAB` 메뉴 UI 처리

## 동작 흐름

1. `lody`에서 게임 시작 버튼을 누르면 `load`로 이동
2. `load`에서 빨간색 또는 파란색을 선택
3. 선택값을 `GameInstance`에 저장
4. `game`으로 이동
5. `game`에서 저장된 선택값에 따라 플레이어를 생성
6. 플레이 중 적과 보스를 생성
7. 보스 처치 시 게임 종료 UI 표시

## 추천 폴더 구조

```text
Content/
  Maps/
    lody.umap
    load.umap
    game.umap
  Blueprints/
    BP_GameInstance.uasset
    BP_GameMode.uasset
    BP_Player.uasset
    BP_Player_Red.uasset
    BP_Player_Blue.uasset
    BP_Enemy.uasset
    BP_Boss.uasset
  Blueprints/UI/
    WBP_Lobby.uasset
    WBP_Load.uasset
    WBP_HP.uasset
    WBP_보스.uasset
    WBP_ESC.uasset
    WBP_끗.uasset
    WBP_실패.uasset
```

## 프로젝트 설정 체크

- `Project Settings > Maps & Modes`
  - `Game Default Map` = `lody`
  - `Editor Startup Map` = `lody`
  - `Game Instance Class` = `BP_GameInstance`
  - `Default GameMode` = `BP_GameMode`

- `Project Settings > Packaging`
  - `lody`, `load`, `game` 맵이 패키징 목록에 포함되어야 함

## 비고

- 이 프로젝트는 블루프린트 전용입니다.
- C++ 빌드는 사용하지 않습니다.
- 패키징 빌드에서 맵 이동이 안 되면, 먼저 패키징 목록에 맵이 포함되어 있는지 확인해야 합니다.

