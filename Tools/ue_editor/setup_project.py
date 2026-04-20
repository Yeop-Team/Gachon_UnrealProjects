# -*- coding: utf-8 -*-
"""
UE5 Flight Shooter — Auto Setup

Run this *inside the UE editor* after you have compiled the C++ module at least
once. In UE editor: Tools → Execute Python Script → pick this file.

It performs:
  1. Creates Blueprint child assets that parent to the C++ classes
       BP_Flight          -> AFlightPawn
       BP_EnemyFighter    -> AEnemyBase
       BP_BossEnemy       -> ABossEnemy
       BP_Bullet_Player   -> AProjectileBase
       BP_Bullet_Enemy    -> AProjectileBase
       BP_EnemyManager    -> AEnemyManager
       BP_ShootingGM      -> AShootingGameMode
       WBP_PlayerHUD      -> UPlayerHUDWidget
       WBP_GameOver       -> UUserWidget
  2. Creates UEnemyStatsAsset instances (Scout / Fighter / Boss) from
       Tools/ue_editor/enemy_stats.csv
  3. Creates UWaveDataAsset instances (DA_Wave_01..06) from waves.json
  4. Wires the default GameMode in DefaultEngine.ini (optional, commented).

Safe to re-run: existing assets are reused and properties re-applied.
"""

from __future__ import annotations

import csv
import json
import os
from pathlib import Path
from typing import Optional

import unreal


# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
SCRIPT_DIR = Path(__file__).resolve().parent
CSV_FILE   = SCRIPT_DIR / "enemy_stats.csv"
WAVES_FILE = SCRIPT_DIR / "waves.json"

BP_DIR    = "/Game/Blueprints/Generated"
DATA_DIR  = "/Game/Data/Generated"
UI_DIR    = "/Game/UI/Generated"


# ---------------------------------------------------------------------------
# Small wrappers
# ---------------------------------------------------------------------------
ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EDITOR_ASSET_LIB = unreal.EditorAssetLibrary


def log(msg: str) -> None:
    unreal.log(f"[FlightShooterSetup] {msg}")


def ensure_dir(pkg: str) -> None:
    if not EDITOR_ASSET_LIB.does_directory_exist(pkg):
        EDITOR_ASSET_LIB.make_directory(pkg)


def load_cpp_class(class_name: str) -> Optional[type]:
    """Load a C++ class that was exposed via UCLASS reflection."""
    cls = unreal.load_class(None, f"/Script/Interior.{class_name}")
    if cls is None:
        log(f"!! Could not load /Script/Interior.{class_name} — did you compile?")
    return cls


def create_or_load_blueprint(name: str, package_path: str, parent_class) -> Optional[unreal.Blueprint]:
    full_path = f"{package_path}/{name}"
    existing = EDITOR_ASSET_LIB.load_asset(full_path)
    if existing:
        log(f"reuse  {full_path}")
        return existing

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    bp = ASSET_TOOLS.create_asset(
        asset_name=name,
        package_path=package_path,
        asset_class=unreal.Blueprint,
        factory=factory,
    )
    if bp:
        EDITOR_ASSET_LIB.save_loaded_asset(bp)
        log(f"create {full_path}")
    return bp


def create_or_load_data_asset(name: str, package_path: str, asset_class) -> Optional[unreal.Object]:
    full_path = f"{package_path}/{name}"
    existing = EDITOR_ASSET_LIB.load_asset(full_path)
    if existing:
        return existing

    # UPrimaryDataAsset uses plain DataAssetFactory with override class.
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", asset_class)
    asset = ASSET_TOOLS.create_asset(
        asset_name=name,
        package_path=package_path,
        asset_class=asset_class,
        factory=factory,
    )
    log(f"create {full_path}")
    return asset


# ---------------------------------------------------------------------------
# Step 1 — Blueprint children of C++ classes
# ---------------------------------------------------------------------------
def step1_blueprints() -> dict:
    ensure_dir(BP_DIR)
    ensure_dir(UI_DIR)

    classes = {
        "FlightPawn":         load_cpp_class("FlightPawn"),
        "EnemyBase":          load_cpp_class("EnemyBase"),
        "BossEnemy":          load_cpp_class("BossEnemy"),
        "ProjectileBase":     load_cpp_class("ProjectileBase"),
        "EnemyManager":       load_cpp_class("EnemyManager"),
        "ShootingGameMode":   load_cpp_class("ShootingGameMode"),
        "PlayerHUDWidget":    load_cpp_class("PlayerHUDWidget"),
    }
    user_widget = unreal.load_class(None, "/Script/UMG.UserWidget")

    blueprints = {
        "BP_Flight":        create_or_load_blueprint("BP_Flight",        BP_DIR, classes["FlightPawn"]),
        "BP_EnemyFighter":  create_or_load_blueprint("BP_EnemyFighter",  BP_DIR, classes["EnemyBase"]),
        "BP_BossEnemy":     create_or_load_blueprint("BP_BossEnemy",     BP_DIR, classes["BossEnemy"]),
        "BP_Bullet_Player": create_or_load_blueprint("BP_Bullet_Player", BP_DIR, classes["ProjectileBase"]),
        "BP_Bullet_Enemy":  create_or_load_blueprint("BP_Bullet_Enemy",  BP_DIR, classes["ProjectileBase"]),
        "BP_EnemyManager":  create_or_load_blueprint("BP_EnemyManager",  BP_DIR, classes["EnemyManager"]),
        "BP_ShootingGM":    create_or_load_blueprint("BP_ShootingGM",    BP_DIR, classes["ShootingGameMode"]),
        "WBP_PlayerHUD":    create_or_load_blueprint("WBP_PlayerHUD",    UI_DIR, classes["PlayerHUDWidget"]),
        "WBP_GameOver":     create_or_load_blueprint("WBP_GameOver",     UI_DIR, user_widget),
    }

    # Hook GameMode BP to auto-use HUD widget generated above.
    gm_bp = blueprints["BP_ShootingGM"]
    hud_bp = blueprints["WBP_PlayerHUD"]
    if gm_bp and hud_bp:
        cdo = unreal.get_default_object(gm_bp.generated_class())
        cdo.set_editor_property("hud_widget_class", hud_bp.generated_class())
        EDITOR_ASSET_LIB.save_loaded_asset(gm_bp)

    return blueprints


# ---------------------------------------------------------------------------
# Step 2 — EnemyStatsAsset from CSV
# ---------------------------------------------------------------------------
def step2_enemy_data(blueprints: dict) -> dict:
    ensure_dir(DATA_DIR)
    stats_class = load_cpp_class("EnemyStatsAsset")
    if not stats_class or not CSV_FILE.exists():
        log(f"skip enemy stats — class or CSV missing ({CSV_FILE})")
        return {}

    bullet_enemy_bp = blueprints.get("BP_Bullet_Enemy")
    bullet_enemy_class = bullet_enemy_bp.generated_class() if bullet_enemy_bp else None

    created: dict = {}
    with CSV_FILE.open(encoding="utf-8") as f:
        for row in csv.DictReader(f):
            name = f"DA_Enemy_{row['Name']}"
            asset = create_or_load_data_asset(name, DATA_DIR, stats_class)
            if not asset:
                continue
            asset.set_editor_property("id",             unreal.Name(row["Name"]))
            asset.set_editor_property("max_health",     float(row["MaxHealth"]))
            asset.set_editor_property("score_reward",   int(row["ScoreReward"]))
            asset.set_editor_property("move_speed",     float(row["MoveSpeed"]))
            asset.set_editor_property("fire_interval",  float(row["FireInterval"]))
            asset.set_editor_property("bullet_damage",  float(row["BulletDamage"]))
            if bullet_enemy_class:
                asset.set_editor_property("bullet_class", bullet_enemy_class)

            # Try to hook the mesh if the CSV has a MeshPath
            mesh_path = row.get("MeshPath") or ""
            if mesh_path:
                mesh_obj = EDITOR_ASSET_LIB.load_asset(mesh_path)
                if mesh_obj:
                    asset.set_editor_property("mesh_asset",
                        unreal.SoftObjectPath(mesh_path))
            EDITOR_ASSET_LIB.save_loaded_asset(asset)
            created[row["Name"]] = asset
    return created


# ---------------------------------------------------------------------------
# Step 3 — WaveDataAsset from JSON
# ---------------------------------------------------------------------------
def step3_wave_data(blueprints: dict, enemy_stats: dict) -> list:
    ensure_dir(DATA_DIR)
    wave_class = load_cpp_class("WaveDataAsset")
    if not wave_class or not WAVES_FILE.exists():
        log(f"skip wave data — class or JSON missing ({WAVES_FILE})")
        return []

    enemy_bp = blueprints.get("BP_EnemyFighter")
    enemy_class = enemy_bp.generated_class() if enemy_bp else None

    boss_bp = blueprints.get("BP_BossEnemy")
    boss_class = boss_bp.generated_class() if boss_bp else None

    data = json.loads(WAVES_FILE.read_text(encoding="utf-8"))

    created = []
    for entry in data.get("waves", []):
        idx = int(entry["index"])
        name = f"DA_Wave_{idx:02d}"
        asset = create_or_load_data_asset(name, DATA_DIR, wave_class)
        if not asset:
            continue
        asset.set_editor_property("wave_index",    idx)
        asset.set_editor_property("b_is_boss_wave", bool(entry.get("boss", False)))

        if entry.get("boss"):
            if boss_class:
                asset.set_editor_property("boss_class", boss_class)
        else:
            if enemy_class:
                asset.set_editor_property("enemy_class", enemy_class)
            stats_name = entry.get("enemy_stats") or "Scout"
            if stats_name in enemy_stats:
                asset.set_editor_property("enemy_stats", enemy_stats[stats_name])
            asset.set_editor_property("enemy_count",   int(entry.get("count", 8)))
            asset.set_editor_property("spawn_interval", float(entry.get("interval", 1.5)))

        text = unreal.Text(entry.get("announcement", f"WAVE {idx}"))
        asset.set_editor_property("announcement", text)

        EDITOR_ASSET_LIB.save_loaded_asset(asset)
        created.append(asset)
    return created


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main() -> None:
    log("=== Flight Shooter auto-setup start ===")
    blueprints = step1_blueprints()
    enemy_stats = step2_enemy_data(blueprints)
    waves = step3_wave_data(blueprints, enemy_stats)

    # Assign generated waves into BP_EnemyManager default
    mgr_bp = blueprints.get("BP_EnemyManager")
    if mgr_bp and waves:
        cdo = unreal.get_default_object(mgr_bp.generated_class())
        cdo.set_editor_property("waves", waves)
        EDITOR_ASSET_LIB.save_loaded_asset(mgr_bp)

    log(f"Blueprints: {len(blueprints)}  Enemy DA: {len(enemy_stats)}  Waves: {len(waves)}")
    log("=== done ===")


if __name__ == "__main__":
    main()
