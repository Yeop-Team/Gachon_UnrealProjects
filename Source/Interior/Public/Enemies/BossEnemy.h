// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "BossEnemy.generated.h"

class UArrowComponent;
class UNiagaraSystem;
class USoundBase;

/**
 * Boss with 3 firing patterns and a Phase2 transition at <= 50% HP.
 */
UCLASS(Blueprintable)
class INTERIOR_API ABossEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ABossEnemy();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Timing")
	float PatternCooldownPhase1 = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Timing")
	float PatternCooldownPhase2 = 1.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|FX")
	TObjectPtr<UNiagaraSystem> EnrageNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|FX")
	TObjectPtr<USoundBase> EnrageSound;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void HandleHealthChanged(float Value01, float Current, float Max);

	void ChoosePattern();
	void PatternSpreadShot();
	void PatternAimedBurst();
	void PatternCircleShot();
	void SpawnBulletToward(const FVector& WorldLocation, const FRotator& WorldRot);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Components")
	TArray<TObjectPtr<UArrowComponent>> Muzzles;

	uint8 PhaseIndex = 0;
	FTimerHandle PatternTimer;
	FTimerHandle BurstTimer;
	int32 BurstShotsRemaining = 0;
};
