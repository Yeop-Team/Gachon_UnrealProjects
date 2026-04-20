// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class AShootingGameMode;
class UHealthComponent;

/**
 * Base class for WBP_PlayerHUD.
 * BP child should define widgets named:
 *   PB_Health, TB_HPText, TB_Score, TB_HighScore, TB_Combo, TB_Wave
 */
UCLASS(Abstract)
class INTERIOR_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleHealthChanged(float Value01, float Current, float Max);

	UFUNCTION()
	void HandleScoreChanged(int32 NewScore);

	UFUNCTION()
	void HandleComboChanged(int32 Count, float Multiplier);

	UFUNCTION()
	void HandleHighScoreUpdated(int32 NewHigh);

	// BindWidget optional entries (BP designer is free to not include them)
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UProgressBar> PB_Health;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TB_HPText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TB_Score;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TB_HighScore;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TB_Combo;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TB_Wave;

private:
	void WireUpBindings();
};
