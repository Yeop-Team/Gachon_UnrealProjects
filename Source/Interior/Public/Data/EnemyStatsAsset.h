// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyStatsAsset.generated.h"

class UStaticMesh;
class AProjectileBase;

UCLASS(BlueprintType)
class INTERIOR_API UEnemyStatsAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float MaxHealth = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	int32 ScoreReward = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Movement")
	float MoveSpeed = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Weapon")
	float FireInterval = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Weapon")
	float BulletDamage = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TSoftObjectPtr<UStaticMesh> MeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Weapon")
	TSubclassOf<AProjectileBase> BulletClass;
};
