// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Shared damage interface implemented by Player, Enemy, Boss.
 * Keeps damage pipeline engine-agnostic and easy to extend.
 */
class INTERIOR_API IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Damage")
	void TakeDamageEvent(float Damage, AActor* DamageCauser);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Damage")
	void Heal(float Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Damage")
	float GetHealthNormalized() const;
};
