// Copyright Yeop. All Rights Reserved.

#include "Enemies/BossEnemy.h"
#include "Components/ArrowComponent.h"
#include "Components/HealthComponent.h"
#include "Projectiles/ProjectileBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Interior.h"

ABossEnemy::ABossEnemy()
{
	// 3 directional muzzles
	UArrowComponent* M1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle0"));
	UArrowComponent* M2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle1"));
	UArrowComponent* M3 = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle2"));
	M1->SetupAttachment(RootComponent);
	M2->SetupAttachment(RootComponent);
	M3->SetupAttachment(RootComponent);
	M1->SetRelativeLocation(FVector(250.f, -120.f, 0.f));
	M2->SetRelativeLocation(FVector(260.f,    0.f, 0.f));
	M3->SetRelativeLocation(FVector(250.f,  120.f, 0.f));
	Muzzles = { M1, M2, M3 };

	// Bosses are tanky and slow
	if (Health)
	{
		Health->MaxHealth = 1500.f;
	}
}

void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (Health)
	{
		Health->OnHealthChanged.AddDynamic(this, &ABossEnemy::HandleHealthChanged);
	}

	// Start pattern loop
	GetWorldTimerManager().SetTimer(PatternTimer, this,
		&ABossEnemy::ChoosePattern, PatternCooldownPhase1, true, 2.0f);
}

void ABossEnemy::Tick(float DeltaSeconds)
{
	// Skip EnemyBase auto-forward + auto-fire; bosses stand their ground and run patterns.
	// Very slow forward drift for presence:
	AddActorWorldOffset(GetActorForwardVector() * 80.f * DeltaSeconds, true);
}

void ABossEnemy::HandleHealthChanged(float Value01, float, float)
{
	if (PhaseIndex == 0 && Value01 <= 0.5f)
	{
		PhaseIndex = 1;
		UE_LOG(LogInterior, Log, TEXT("Boss enraged (Phase 2)"));

		if (EnrageNiagara)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				this, EnrageNiagara, GetActorLocation(), GetActorRotation());
		}
		if (EnrageSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EnrageSound, GetActorLocation());
		}

		// Hit-stop: briefly slow time, then restore
		UGameplayStatics::SetGlobalTimeDilation(this, 0.2f);
		FTimerHandle ResetTime;
		GetWorldTimerManager().SetTimer(ResetTime,
			FTimerDelegate::CreateWeakLambda(this, [this]() {
				UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
			}),
			0.15f, false);

		// Faster pattern cadence
		GetWorldTimerManager().ClearTimer(PatternTimer);
		GetWorldTimerManager().SetTimer(PatternTimer, this,
			&ABossEnemy::ChoosePattern, PatternCooldownPhase2, true, 0.8f);
	}
}

void ABossEnemy::ChoosePattern()
{
	if (!BulletClass) return;

	// Phase1: 2 patterns. Phase2: all 3, weighted toward circle.
	int32 Choice = 0;
	if (PhaseIndex == 0)
	{
		Choice = FMath::RandRange(0, 1);
	}
	else
	{
		Choice = FMath::RandRange(0, 2);
	}

	switch (Choice)
	{
		case 0: PatternSpreadShot(); break;
		case 1: PatternAimedBurst(); break;
		default: PatternCircleShot(); break;
	}
}

void ABossEnemy::SpawnBulletToward(const FVector& WorldLocation, const FRotator& WorldRot)
{
	if (!BulletClass) return;

	FActorSpawnParameters P;
	P.Owner = this;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AProjectileBase* Proj = GetWorld()->SpawnActor<AProjectileBase>(
			BulletClass, WorldLocation, WorldRot, P))
	{
		Proj->Damage = BulletDamage;
	}
}

void ABossEnemy::PatternSpreadShot()
{
	for (UArrowComponent* M : Muzzles)
	{
		if (!M) continue;
		const FTransform T = M->GetComponentTransform();
		for (int i = -1; i <= 1; ++i)
		{
			const FRotator Rot = T.Rotator() + FRotator(0.f, i * 12.f, 0.f);
			SpawnBulletToward(T.GetLocation(), Rot);
		}
	}
}

void ABossEnemy::PatternAimedBurst()
{
	// 5 aimed shots with 0.08s spacing
	BurstShotsRemaining = 5;
	auto DoShot = [this]()
	{
		if (BurstShotsRemaining <= 0) { GetWorldTimerManager().ClearTimer(BurstTimer); return; }
		--BurstShotsRemaining;
		APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
		if (!Player) return;

		if (Muzzles.Num() > 0 && Muzzles[1])
		{
			const FVector From = Muzzles[1]->GetComponentLocation();
			const FVector Dir  = (Player->GetActorLocation() - From).GetSafeNormal();
			SpawnBulletToward(From, Dir.Rotation());
		}
	};
	DoShot();
	GetWorldTimerManager().SetTimer(BurstTimer,
		FTimerDelegate::CreateWeakLambda(this, DoShot),
		0.08f, true);
}

void ABossEnemy::PatternCircleShot()
{
	const FVector C = GetActorLocation();
	for (int32 Deg = 0; Deg < 360; Deg += 20)
	{
		const FRotator R(0.f, Deg, 0.f);
		const FVector  F = R.Vector();
		SpawnBulletToward(C + F * 120.f, R);
	}
}
