# Tools/ue_editor — UE5 Editor Python Scripts

## What this does

`setup_project.py` is a one-shot setup script that you run from **inside the UE
editor** (Tools → Execute Python Script…). It creates Blueprint children of all
C++ classes, then fills in DataAssets from `enemy_stats.csv` and `waves.json`.

## Requirements

1. The C++ module `Interior` must be compiled at least once.
2. Plugins `PythonScriptPlugin` and `EditorScriptingUtilities` must be enabled.
   (They are, in `Interior.uproject`.)
3. You must run the script **inside the editor**, not from an external shell.

## Data format

### enemy_stats.csv
```
Name,MaxHealth,ScoreReward,MoveSpeed,FireInterval,BulletDamage,MeshPath
Scout,20,100,500,1.6,6,/Game/Drone/Drone
```
- `Name` maps to `UEnemyStatsAsset.Id` and is used in waves.json.
- `MeshPath` is a UE package path to any `UStaticMesh`. Leave blank to skip.

### waves.json
```json
{ "waves": [
  { "index": 1, "enemy_stats": "Scout", "count": 6, "interval": 1.8, "boss": false },
  { "index": 4, "boss": true, "announcement": "BOSS INCOMING" }
]}
```

## Re-running

Safe. Existing assets are detected via `EditorAssetLibrary.load_asset` and their
properties are re-applied. To force-create, delete the corresponding asset in
the Content Browser first.

## Adding a new wave

1. Add a row to `enemy_stats.csv` (if you need a new enemy type).
2. Add an entry to `waves.json`.
3. Tools → Execute Python Script → `setup_project.py`.
4. `BP_EnemyManager` default `Waves` will be refreshed automatically.
