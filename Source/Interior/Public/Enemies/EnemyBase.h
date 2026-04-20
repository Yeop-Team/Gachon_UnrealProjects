// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/Damageable.h"
#include "EnemyBase.generated.h"

class UStaticMeshComponent;
class UFloatingPawnMovement;
class UHealthComponent;
class UEnemyStatsAsset;
class AProjectileBase;
class UNiagaraSystem;
class USoundBase;

UCLASS(Blueprintable)
class INTERIOR_API AEnemyBase : public AActor, public IDamageable
{
	GENERATED_BODY()

public:
	AEnemyBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<UEnemyStatsAsset> StatsAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|FX")
	TObjectPtr<UNiagaraSystem> DeathNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|FX")
	TObjectPtr<USoundBase> DeathSound;

	// IDamageable
	virtual void TakeDamageEvent_Implementation(float Damage, AActor* DamageCauser) override;
	virtual void Heal_Implementation(float Amount) override;
	virtual float GetHealthNormalized_Implementation() const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void HandleDied(AActor* DamageCauser);

	void FireTowardsPlayer();
	void ApplyStatsFromAsset();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
	TObjectPtr<UHealthComponent> Health;

	// Runtime stats
	float MoveSpeed = 400.f;
	float FireInterval = 1.5f;
	float BulletDamage = 8.f;
	int32 ScoreReward = 100;
	TSubclassOf<AProjectileBase> BulletClass;

	float FireTimer = 0.f;
};
