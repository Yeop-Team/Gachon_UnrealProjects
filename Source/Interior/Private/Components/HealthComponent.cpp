// Copyright Yeop. All Rights Reserved.

#include "Components/HealthComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(1.f, CurrentHealth, MaxHealth);
}

void UHealthComponent::ApplyDamage(float Damage, AActor* Causer)
{
	if (bIsDead || bInvincible || Damage <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(GetHealthNormalized(), CurrentHealth, MaxHealth);

	// Short invincibility frames on hit
	bInvincible = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			InvincibleHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]() { bInvincible = false; }),
			InvincibleTime,
			false);
	}

	if (CurrentHealth <= KINDA_SMALL_NUMBER && !bIsDead)
	{
		bIsDead = true;
		OnDied.Broadcast(Causer);
	}
}

void UHealthComponent::HealBy(float Amount)
{
	if (bIsDead || Amount <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(GetHealthNormalized(), CurrentHealth, MaxHealth);
}

float UHealthComponent::GetHealthNormalized() const
{
	return (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;
}
