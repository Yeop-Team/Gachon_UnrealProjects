// Copyright Yeop. All Rights Reserved.

#include "GameMode/ShootingGameMode.h"
#include "GameMode/ShootingSaveGame.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Interior.h"

AShootingGameMode::AShootingGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AShootingGameMode::BeginPlay()
{
	Super::BeginPlay();

	LoadHighScore();

	if (HUDWidgetClass)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			HUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport();
			}
		}
	}

	OnScoreChanged.Broadcast(CurrentScore);
	OnComboChanged.Broadcast(ComboCount, ComboMultiplier);
}

void AShootingGameMode::AddScore(int32 Amount, AActor* Causer)
{
	if (Amount <= 0) return;

	++ComboCount;
	ComboMultiplier = FMath::Clamp(1.f + ComboCount * 0.1f, 1.f, 3.f);

	const int32 Gained = FMath::RoundToInt(Amount * ComboMultiplier);
	CurrentScore += Gained;

	OnScoreChanged.Broadcast(CurrentScore);
	OnComboChanged.Broadcast(ComboCount, ComboMultiplier);

	if (CurrentScore > HighScore)
	{
		HighScore = CurrentScore;
		OnHighScoreUpdated.Broadcast(HighScore);
	}

	// Reset combo timer (3s default)
	GetWorldTimerManager().ClearTimer(ComboTimerHandle);
	GetWorldTimerManager().SetTimer(ComboTimerHandle, this,
		&AShootingGameMode::ResetCombo, ComboWindowSeconds, false);
}

void AShootingGameMode::ResetCombo()
{
	ComboCount = 0;
	ComboMultiplier = 1.f;
	OnComboChanged.Broadcast(ComboCount, ComboMultiplier);
}

void AShootingGameMode::SetCurrentWave(int32 WaveNumber)
{
	CurrentWave = WaveNumber;
}

void AShootingGameMode::NotifyPlayerKilled()
{
	SaveHighScore();
	OnPlayerKilled.Broadcast();

	if (GameOverWidgetClass)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (UUserWidget* Go = CreateWidget<UUserWidget>(PC, GameOverWidgetClass))
			{
				Go->AddToViewport(10);
				PC->bShowMouseCursor = true;
				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(Go->TakeWidget());
				PC->SetInputMode(Mode);
			}
		}
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void AShootingGameMode::LoadHighScore()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		if (UShootingSaveGame* Save = Cast<UShootingSaveGame>(
				UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
		{
			HighScore = Save->HighScore;
			UE_LOG(LogInterior, Log, TEXT("Loaded HighScore: %d"), HighScore);
		}
	}
}

void AShootingGameMode::SaveHighScore()
{
	UShootingSaveGame* Save = Cast<UShootingSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UShootingSaveGame::StaticClass()));
	if (!Save) return;
	Save->HighScore = HighScore;
	Save->BestWave  = CurrentWave;
	Save->LastSavedAt = FDateTime::Now();
	UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
	UE_LOG(LogInterior, Log, TEXT("Saved HighScore: %d  BestWave: %d"), HighScore, CurrentWave);
}
