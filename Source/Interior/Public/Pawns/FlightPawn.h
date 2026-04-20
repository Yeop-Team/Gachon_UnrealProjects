// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Combat/Damageable.h"
#include "FlightPawn.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UArrowComponent;
class UFloatingPawnMovement;
class UHealthComponent;
class UInputAction;
class UInputMappingContext;
class UInputComponent;
class AProjectileBase;
class USoundBase;
class UCameraShakeBase;
struct FInputActionValue;

/**
 * Player-controlled flight pawn.
 * - APawn (not ACharacter) so we can freely rotate / pitch / roll in 3D.
 * - Two cameras (TPS SpringArm + FPS Cockpit). 'V' toggles.
 * - Enhanced Input bindings for Move / Look / Fire / ToggleCamera.
 */
UCLASS(Blueprintable)
class INTERIOR_API AFlightPawn : public APawn, public IDamageable
{
	GENERATED_BODY()

public:
	AFlightPawn();

	// --- Component access ---
	UFUNCTION(BlueprintPure, Category = "Flight|Components")
	UHealthComponent* GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Flight|Camera")
	void ToggleCameraView();

	// --- IDamageable ---
	virtual void TakeDamageEvent_Implementation(float Damage, AActor* DamageCauser) override;
	virtual void Heal_Implementation(float Amount) override;
	virtual float GetHealthNormalized_Implementation() const override;

	// --- Flight tuning ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Movement")
	float MaxSpeed = 2200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Movement")
	float Acceleration = 1800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Movement")
	float PitchRateDeg = 70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Movement")
	float YawRateDeg = 70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Movement")
	float RollRateDeg = 120.f;

	// --- Weapon ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Weapon")
	TSubclassOf<AProjectileBase> BulletClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Weapon")
	float FireInterval = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Weapon", meta = (ClampMin = "1", ClampMax = "5"))
	int32 WeaponLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Weapon")
	float BulletDamage = 15.f;

	// --- Input Assets (assign in BP child) ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Input")
	TObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Input")
	TObjectPtr<UInputAction> IA_Move;    // Vector2D (X = throttle, Y = strafe)

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Input")
	TObjectPtr<UInputAction> IA_Look;    // Vector2D (X = yaw, Y = pitch)

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Input")
	TObjectPtr<UInputAction> IA_Roll;    // float

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Input")
	TObjectPtr<UInputAction> IA_Fire;    // bool (held)

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Input")
	TObjectPtr<UInputAction> IA_ToggleCamera; // bool

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|Input")
	TObjectPtr<UInputAction> IA_Boost;   // bool held

	// --- Feedback ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|FX")
	TSubclassOf<UCameraShakeBase> HitShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|FX")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Flight|FX")
	TObjectPtr<USoundBase> FireSound;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void HandleDied(AActor* DamageCauser);

	// Input handlers
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnRoll(const FInputActionValue& Value);
	void OnFireStarted(const FInputActionValue& Value);
	void OnFireCompleted(const FInputActionValue& Value);
	void OnToggleCamera(const FInputActionValue& Value);
	void OnBoostStarted(const FInputActionValue& Value);
	void OnBoostCompleted(const FInputActionValue& Value);

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Components")
	TObjectPtr<UStaticMeshComponent> ShipMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Components")
	TObjectPtr<UCameraComponent> TpsCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Components")
	TObjectPtr<UCameraComponent> FpsCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Components")
	TObjectPtr<UArrowComponent> MuzzleLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Components")
	TObjectPtr<UArrowComponent> MuzzleRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Components")
	TObjectPtr<UArrowComponent> MuzzleCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Components")
	TObjectPtr<UHealthComponent> Health;

private:
	void FireOnce();
	void UpdateActiveCamera();

	FVector CurrentVelocity = FVector::ZeroVector;
	FRotator InputRotRate = FRotator::ZeroRotator;
	bool bFireHeld = false;
	bool bBoost = false;
	bool bUseTpsCamera = true;
	float FireCooldown = 0.f;
};
