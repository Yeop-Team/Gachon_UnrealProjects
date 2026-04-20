// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyManager.generated.h"

class AEnemyBase;
class ABossEnemy;
class UWaveDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveChanged, int32, WaveNumber, bool, bIsBossWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCleared);

UCLASS(Blueprintable)
class INTERIOR_API AEnemyManager : public AActor
{
	GENERATED_BODY()

public:
	AEnemyManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<TObjectPtr<UWaveDataAsset>> Waves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Spawning")
	TArray<FTransform> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Spawning")
	float InitialDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Spawning")
	float WaveGap = 2.5f;

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveChanged OnWaveChanged;

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnAllWavesCleared OnAllWavesCleared;

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWaves();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	int32 GetCurrentWaveNumber() const { return CurrentWaveIndex + 1; }

protected:
	virtual void BeginPlay() override;

	void StartNextWave();
	void SpawnOneEnemy();
	void SpawnBoss();

	UFUNCTION()
	void HandleEnemyDied(AActor* DamageCauser);

	int32 CurrentWaveIndex = -1;
	int32 SpawnedThisWave = 0;
	int32 AliveEnemies = 0;
	FTimerHandle SpawnTimerHandle;
	FTimerHandle BetweenWaveTimerHandle;
};
