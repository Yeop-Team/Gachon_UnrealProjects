// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShootingGameMode.generated.h"

class UUserWidget;
class UShootingSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboChanged, int32, Count, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighScoreUpdated, int32, NewHighScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerKilled);

UCLASS(Blueprintable)
class INTERIOR_API AShootingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShootingGameMode();

	// --- Config ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score")
	float ComboWindowSeconds = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Save")
	FString SaveSlotName = TEXT("ShootingSave");

	// --- Runtime (readonly for BP) ---
	UPROPERTY(BlueprintReadOnly, Category = "Score") int32 CurrentScore = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Score") int32 HighScore    = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Wave")  int32 CurrentWave  = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Score") int32 ComboCount   = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Score") float ComboMultiplier = 1.f;

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Score") FOnScoreChanged       OnScoreChanged;
	UPROPERTY(BlueprintAssignable, Category = "Score") FOnComboChanged       OnComboChanged;
	UPROPERTY(BlueprintAssignable, Category = "Score") FOnHighScoreUpdated   OnHighScoreUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Flow")  FOnPlayerKilled       OnPlayerKilled;

	// --- API ---
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Amount, AActor* Causer);

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void NotifyPlayerKilled();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void SetCurrentWave(int32 WaveNumber);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadHighScore();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveHighScore();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ResetCombo();

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> HUDWidget;

private:
	FTimerHandle ComboTimerHandle;
};
