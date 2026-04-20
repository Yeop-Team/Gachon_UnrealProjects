// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WaveDataAsset.generated.h"

class AEnemyBase;
class ABossEnemy;
class UEnemyStatsAsset;

UCLASS(BlueprintType)
class INTERIOR_API UWaveDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 WaveIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	bool bIsBossWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (EditCondition = "!bIsBossWave"))
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (EditCondition = "!bIsBossWave"))
	TObjectPtr<UEnemyStatsAsset> EnemyStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (EditCondition = "!bIsBossWave"))
	int32 EnemyCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (EditCondition = "!bIsBossWave"))
	float SpawnInterval = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (EditCondition = "bIsBossWave"))
	TSubclassOf<ABossEnemy> BossClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	FText Announcement;
};
