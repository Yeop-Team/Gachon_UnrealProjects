// Copyright Yeop. All Rights Reserved.

#include "Enemies/EnemyBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HealthComponent.h"
#include "Data/EnemyStatsAsset.h"
#include "Projectiles/ProjectileBase.h"
#include "GameMode/ShootingGameMode.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Interior.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("Pawn"));
	SetRootComponent(Mesh);

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Health->MaxHealth = 30.f;
	Health->InvincibleTime = 0.f;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	ApplyStatsFromAsset();

	if (Health)
	{
		Health->OnDied.AddDynamic(this, &AEnemyBase::HandleDied);
	}
}

void AEnemyBase::ApplyStatsFromAsset()
{
	if (!StatsAsset) return;

	if (Health)
	{
		Health->MaxHealth = StatsAsset->MaxHealth;
	}
	MoveSpeed    = StatsAsset->MoveSpeed;
	FireInterval = StatsAsset->FireInterval;
	BulletDamage = StatsAsset->BulletDamage;
	ScoreReward  = StatsAsset->ScoreReward;
	BulletClass  = StatsAsset->BulletClass;

	if (Mesh && !StatsAsset->MeshAsset.IsNull())
	{
		if (UStaticMesh* LoadedMesh = StatsAsset->MeshAsset.LoadSynchronous())
		{
			Mesh->SetStaticMesh(LoadedMesh);
		}
	}
}

void AEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Simple behavior: drift forward (towards player's general area) and fire periodically.
	const FVector Forward = GetActorForwardVector();
	AddActorWorldOffset(Forward * MoveSpeed * DeltaSeconds, true);

	FireTimer += DeltaSeconds;
	if (FireTimer >= FireInterval)
	{
		FireTimer = 0.f;
		FireTowardsPlayer();
	}
}

void AEnemyBase::FireTowardsPlayer()
{
	if (!BulletClass) return;

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player) return;

	const FVector Start = GetActorLocation() + GetActorForwardVector() * 100.f;
	const FVector Dir   = (Player->GetActorLocation() - Start).GetSafeNormal();
	const FRotator Rot  = Dir.Rotation();

	FActorSpawnParameters P;
	P.Owner = this;
	P.Instigator = nullptr;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AProjectileBase* Proj = GetWorld()->SpawnActor<AProjectileBase>(BulletClass, Start, Rot, P))
	{
		Proj->Damage = BulletDamage;
	}
}

// ---------- IDamageable ----------
void AEnemyBase::TakeDamageEvent_Implementation(float Damage, AActor* Causer)
{
	if (Health) Health->ApplyDamage(Damage, Causer);
}

void AEnemyBase::Heal_Implementation(float Amount)
{
	if (Health) Health->HealBy(Amount);
}

float AEnemyBase::GetHealthNormalized_Implementation() const
{
	return Health ? Health->GetHealthNormalized() : 0.f;
}

void AEnemyBase::HandleDied(AActor* DamageCauser)
{
	if (DeathNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, DeathNiagara, GetActorLocation(), GetActorRotation());
	}
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	if (AShootingGameMode* GM = Cast<AShootingGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->AddScore(ScoreReward, DamageCauser);
	}

	UE_LOG(LogInterior, Verbose, TEXT("%s died. Score +%d"), *GetName(), ScoreReward);
	Destroy();
}
