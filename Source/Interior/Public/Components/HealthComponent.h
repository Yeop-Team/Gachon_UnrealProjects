// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, Value01, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDied, AActor*, DamageCauser);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Health Component")
class INTERIOR_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	// --- Tunables ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float InvincibleTime = 0.4f;

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDied OnDied;

	// --- API ---
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamage(float Damage, AActor* Causer);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HealBy(float Amount);

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealthNormalized() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	FORCEINLINE bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetInvincible(bool bNewValue) { bInvincible = bNewValue; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	float CurrentHealth = 0.f;

	bool bIsDead = false;
	bool bInvincible = false;
	FTimerHandle InvincibleHandle;
};
