// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UNiagaraSystem;
class USoundBase;

UCLASS(Blueprintable)
class INTERIOR_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float Damage = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float InitialSpeed = 6500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float LifeSeconds = 3.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|FX")
	TObjectPtr<UNiagaraSystem> HitNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|FX")
	TObjectPtr<USoundBase> HitSound;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|Components")
	TObjectPtr<UStaticMeshComponent> BulletMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|Components")
	TObjectPtr<UProjectileMovementComponent> MovementComp;
};
