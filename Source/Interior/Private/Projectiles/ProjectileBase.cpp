// Copyright Yeop. All Rights Reserved.

#include "Projectiles/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Combat/Damageable.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionSphere->InitSphereRadius(12.f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	CollisionSphere->SetGenerateOverlapEvents(false);
	SetRootComponent(CollisionSphere);

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	BulletMesh->SetupAttachment(CollisionSphere);
	BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BulletMesh->SetRelativeScale3D(FVector(0.4f, 0.1f, 0.1f));

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	MovementComp->InitialSpeed = InitialSpeed;
	MovementComp->MaxSpeed = InitialSpeed;
	MovementComp->ProjectileGravityScale = 0.f;
	MovementComp->bRotationFollowsVelocity = true;

	InitialLifeSpan = LifeSeconds;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	MovementComp->InitialSpeed = InitialSpeed;
	MovementComp->MaxSpeed = InitialSpeed;
	MovementComp->Velocity = GetActorForwardVector() * InitialSpeed;

	CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileBase::HandleHit);
	SetLifeSpan(LifeSeconds);
}

void AProjectileBase::HandleHit(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, FVector, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		if (OtherActor->Implements<UDamageable>())
		{
			IDamageable::Execute_TakeDamageEvent(OtherActor, Damage, GetOwner());
		}
	}

	if (HitNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, HitNiagara, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint, 0.5f);
	}
	Destroy();
}
