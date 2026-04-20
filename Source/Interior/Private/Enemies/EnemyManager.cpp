// Copyright Yeop. All Rights Reserved.

#include "Enemies/EnemyManager.h"
#include "Enemies/EnemyBase.h"
#include "Enemies/BossEnemy.h"
#include "Data/WaveDataAsset.h"
#include "Components/HealthComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Interior.h"

AEnemyManager::AEnemyManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyManager::BeginPlay()
{
	Super::BeginPlay();

	if (Waves.Num() == 0)
	{
		UE_LOG(LogInterior, Warning, TEXT("AEnemyManager has no waves assigned"));
		return;
	}

	GetWorldTimerManager().SetTimer(BetweenWaveTimerHandle, this,
		&AEnemyManager::StartNextWave, InitialDelay, false);
}

void AEnemyManager::StartWaves()
{
	CurrentWaveIndex = -1;
	StartNextWave();
}

void AEnemyManager::StartNextWave()
{
	++CurrentWaveIndex;

	if (CurrentWaveIndex >= Waves.Num())
	{
		OnAllWavesCleared.Broadcast();
		UE_LOG(LogInterior, Log, TEXT("All waves cleared!"));
		return;
	}

	UWaveDataAsset* Wave = Waves[CurrentWaveIndex];
	if (!Wave) { StartNextWave(); return; }

	SpawnedThisWave = 0;
	AliveEnemies = 0;
	OnWaveChanged.Broadcast(CurrentWaveIndex + 1, Wave->bIsBossWave);

	if (Wave->bIsBossWave)
	{
		SpawnBoss();
	}
	else
	{
		if (Wave->EnemyClass && Wave->EnemyCount > 0)
		{
			GetWorldTimerManager().SetTimer(SpawnTimerHandle, this,
				&AEnemyManager::SpawnOneEnemy, FMath::Max(Wave->SpawnInterval, 0.1f), true, 0.2f);
		}
	}
}

static FTransform PickSpawn(const TArray<FTransform>& Points, AActor* Ref)
{
	if (Points.Num() > 0)
	{
		return Points[FMath::RandRange(0, Points.Num() - 1)];
	}
	// Fallback: spawn in front of manager
	FTransform T = Ref ? Ref->GetActorTransform() : FTransform::Identity;
	T.AddToTranslation(FVector(3000.f, FMath::FRandRange(-1500.f, 1500.f), FMath::FRandRange(-400.f, 400.f)));
	T.SetRotation(FQuat(FRotator(0.f, 180.f, 0.f))); // face -X toward player
	return T;
}

void AEnemyManager::SpawnOneEnemy()
{
	if (CurrentWaveIndex < 0 || CurrentWaveIndex >= Waves.Num()) return;
	UWaveDataAsset* Wave = Waves[CurrentWaveIndex];
	if (!Wave || !Wave->EnemyClass) return;

	if (SpawnedThisWave >= Wave->EnemyCount)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	const FTransform T = PickSpawn(SpawnPoints, this);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(Wave->EnemyClass, T, P))
	{
		Enemy->StatsAsset = Wave->EnemyStats;
		if (UHealthComponent* EH = Enemy->FindComponentByClass<UHealthComponent>())
		{
			EH->OnDied.AddDynamic(this, &AEnemyManager::HandleEnemyDied);
		}
		++SpawnedThisWave;
		++AliveEnemies;
	}
}

void AEnemyManager::SpawnBoss()
{
	UWaveDataAsset* Wave = Waves[CurrentWaveIndex];
	if (!Wave || !Wave->BossClass) return;

	const FTransform T = PickSpawn(SpawnPoints, this);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ABossEnemy* Boss = GetWorld()->SpawnActor<ABossEnemy>(Wave->BossClass, T, P))
	{
		SpawnedThisWave = 1;
		AliveEnemies = 1;
		if (UHealthComponent* EH = Boss->FindComponentByClass<UHealthComponent>())
		{
			EH->OnDied.AddDynamic(this, &AEnemyManager::HandleEnemyDied);
		}
	}
}

void AEnemyManager::HandleEnemyDied(AActor* /*DamageCauser*/)
{
	AliveEnemies = FMath::Max(AliveEnemies - 1, 0);
	UWaveDataAsset* Wave = (CurrentWaveIndex >= 0 && CurrentWaveIndex < Waves.Num())
		? Waves[CurrentWaveIndex] : nullptr;
	if (!Wave) return;

	const int32 Target = Wave->bIsBossWave ? 1 : Wave->EnemyCount;
	if (SpawnedThisWave >= Target && AliveEnemies == 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		GetWorldTimerManager().SetTimer(BetweenWaveTimerHandle, this,
			&AEnemyManager::StartNextWave, WaveGap, false);
	}
}
