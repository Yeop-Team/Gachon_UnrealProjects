// Copyright Yeop. All Rights Reserved.

#include "Pawns/FlightPawn.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/HealthComponent.h"
#include "Projectiles/ProjectileBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Interior.h"

AFlightPawn::AFlightPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	SetRootComponent(ShipMesh);
	ShipMesh->SetCollisionProfileName(TEXT("Pawn"));
	ShipMesh->SetSimulatePhysics(false);

	// TPS (follow) camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(ShipMesh);
	SpringArm->TargetArmLength = 650.f;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 120.f);
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 12.f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;

	TpsCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TpsCamera"));
	TpsCamera->SetupAttachment(SpringArm);
	TpsCamera->SetActive(true);

	// FPS (cockpit) camera
	FpsCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FpsCamera"));
	FpsCamera->SetupAttachment(ShipMesh);
	FpsCamera->SetRelativeLocation(FVector(40.f, 0.f, 40.f));
	FpsCamera->SetActive(false);

	MuzzleLeft = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzleLeft"));
	MuzzleLeft->SetupAttachment(ShipMesh);
	MuzzleLeft->SetRelativeLocation(FVector(120.f, -80.f, 0.f));

	MuzzleRight = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzleRight"));
	MuzzleRight->SetupAttachment(ShipMesh);
	MuzzleRight->SetRelativeLocation(FVector(120.f, 80.f, 0.f));

	MuzzleCenter = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzleCenter"));
	MuzzleCenter->SetupAttachment(ShipMesh);
	MuzzleCenter->SetRelativeLocation(FVector(140.f, 0.f, 0.f));

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Health->MaxHealth = 150.f;
	Health->InvincibleTime = 0.35f;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AFlightPawn::BeginPlay()
{
	Super::BeginPlay();

	if (Health)
	{
		Health->OnDied.AddDynamic(this, &AFlightPawn::HandleDied);
	}

	// Register Enhanced Input mapping
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (InputMapping)
			{
				Subsystem->AddMappingContext(InputMapping, 0);
			}
		}
	}

	UpdateActiveCamera();
}

void AFlightPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Apply rotational inputs
	if (!InputRotRate.IsNearlyZero())
	{
		AddActorLocalRotation(InputRotRate * DeltaSeconds);
		InputRotRate = FRotator::ZeroRotator;
	}

	// Forward throttle (always cruise forward, throttle adjusts speed)
	const float TargetSpeed = bBoost ? MaxSpeed * 1.8f : MaxSpeed;
	const FVector Forward = GetActorForwardVector();
	const FVector Desired = Forward * TargetSpeed;
	CurrentVelocity = FMath::VInterpTo(CurrentVelocity, Desired, DeltaSeconds, 2.f);
	AddActorWorldOffset(CurrentVelocity * DeltaSeconds, true);

	// Auto-level roll gently
	FRotator Rot = GetActorRotation();
	Rot.Roll = FMath::FInterpTo(Rot.Roll, 0.f, DeltaSeconds, 1.5f);
	SetActorRotation(Rot);

	// Weapon cooldown / auto-fire
	if (FireCooldown > 0.f) { FireCooldown -= DeltaSeconds; }
	if (bFireHeld && FireCooldown <= 0.f)
	{
		FireOnce();
		FireCooldown = FireInterval;
	}
}

void AFlightPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_Move)          EIC->BindAction(IA_Move,          ETriggerEvent::Triggered, this, &AFlightPawn::OnMove);
		if (IA_Look)          EIC->BindAction(IA_Look,          ETriggerEvent::Triggered, this, &AFlightPawn::OnLook);
		if (IA_Roll)          EIC->BindAction(IA_Roll,          ETriggerEvent::Triggered, this, &AFlightPawn::OnRoll);
		if (IA_Fire)
		{
			EIC->BindAction(IA_Fire, ETriggerEvent::Started,   this, &AFlightPawn::OnFireStarted);
			EIC->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AFlightPawn::OnFireCompleted);
		}
		if (IA_ToggleCamera)  EIC->BindAction(IA_ToggleCamera,  ETriggerEvent::Started,   this, &AFlightPawn::OnToggleCamera);
		if (IA_Boost)
		{
			EIC->BindAction(IA_Boost, ETriggerEvent::Started,   this, &AFlightPawn::OnBoostStarted);
			EIC->BindAction(IA_Boost, ETriggerEvent::Completed, this, &AFlightPawn::OnBoostCompleted);
		}
	}
}

void AFlightPawn::OnMove(const FInputActionValue& Value)
{
	// Value2D: X = throttle (W/S), Y = strafe (A/D) -> small sideways nudge
	const FVector2D V = Value.Get<FVector2D>();
	// Throttle nudges MaxSpeed smoothly; for simplicity we let Tick accelerate forward
	// Strafe applies a lateral offset to velocity
	const FVector Right = GetActorRightVector();
	CurrentVelocity += Right * V.Y * Acceleration * GetWorld()->GetDeltaSeconds();

	// Pitch-up/down coupling with throttle down for a more "arcade" feel
	if (!FMath::IsNearlyZero(V.X))
	{
		// W (positive) = speed boost bias; S (negative) = brake bias — handled by MaxSpeed already
	}
}

void AFlightPawn::OnLook(const FInputActionValue& Value)
{
	const FVector2D V = Value.Get<FVector2D>();
	// Mouse: X = yaw, Y = pitch (inverted)
	InputRotRate.Yaw   += V.X * YawRateDeg;
	InputRotRate.Pitch += -V.Y * PitchRateDeg;
}

void AFlightPawn::OnRoll(const FInputActionValue& Value)
{
	const float F = Value.Get<float>();
	InputRotRate.Roll += F * RollRateDeg;
}

void AFlightPawn::OnFireStarted(const FInputActionValue&)    { bFireHeld = true; }
void AFlightPawn::OnFireCompleted(const FInputActionValue&)  { bFireHeld = false; }

void AFlightPawn::OnToggleCamera(const FInputActionValue&)
{
	bUseTpsCamera = !bUseTpsCamera;
	UpdateActiveCamera();
}

void AFlightPawn::OnBoostStarted(const FInputActionValue&)   { bBoost = true; }
void AFlightPawn::OnBoostCompleted(const FInputActionValue&) { bBoost = false; }

void AFlightPawn::UpdateActiveCamera()
{
	if (TpsCamera) TpsCamera->SetActive(bUseTpsCamera);
	if (FpsCamera) FpsCamera->SetActive(!bUseTpsCamera);

	UE_LOG(LogInterior, Log, TEXT("Camera view: %s"),
		bUseTpsCamera ? TEXT("Third-Person") : TEXT("First-Person (Cockpit)"));
}

void AFlightPawn::FireOnce()
{
	if (!BulletClass) return;

	auto SpawnAt = [this](UArrowComponent* Arrow)
	{
		if (!Arrow) return;
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FTransform T = Arrow->GetComponentTransform();
		if (AProjectileBase* Proj = GetWorld()->SpawnActor<AProjectileBase>(
				BulletClass, T.GetLocation(), T.Rotator(), Params))
		{
			Proj->Damage = BulletDamage;
		}
	};

	switch (WeaponLevel)
	{
		case 1:
			SpawnAt(MuzzleCenter);
			break;
		case 2:
			SpawnAt(MuzzleLeft);
			SpawnAt(MuzzleRight);
			break;
		default: // 3+
			SpawnAt(MuzzleLeft);
			SpawnAt(MuzzleRight);
			SpawnAt(MuzzleCenter);
			break;
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation(), 0.6f);
	}
}

// ---------- IDamageable ----------
void AFlightPawn::TakeDamageEvent_Implementation(float Damage, AActor* DamageCauser)
{
	if (Health) Health->ApplyDamage(Damage, DamageCauser);

	if (HitShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(HitShakeClass, 1.f);
		}
	}
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
}

void AFlightPawn::Heal_Implementation(float Amount)
{
	if (Health) Health->HealBy(Amount);
}

float AFlightPawn::GetHealthNormalized_Implementation() const
{
	return Health ? Health->GetHealthNormalized() : 0.f;
}

void AFlightPawn::HandleDied(AActor* DamageCauser)
{
	UE_LOG(LogInterior, Log, TEXT("Player died (caused by %s)"), *GetNameSafe(DamageCauser));
	DisableInput(Cast<APlayerController>(GetController()));

	// GameMode will handle the GameOver UI via its own delegate.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetCinematicMode(true, true, true, true, true);
	}
}
