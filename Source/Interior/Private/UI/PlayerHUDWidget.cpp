// Copyright Yeop. All Rights Reserved.

#include "UI/PlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameMode/ShootingGameMode.h"
#include "Components/HealthComponent.h"
#include "Pawns/FlightPawn.h"
#include "Kismet/GameplayStatics.h"

void UPlayerHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WireUpBindings();
}

void UPlayerHUDWidget::WireUpBindings()
{
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (AFlightPawn* Flight = Cast<AFlightPawn>(Pawn))
		{
			if (UHealthComponent* H = Flight->GetHealth())
			{
				H->OnHealthChanged.AddUniqueDynamic(this, &UPlayerHUDWidget::HandleHealthChanged);
				// Initial push
				HandleHealthChanged(H->GetHealthNormalized(), 0.f, H->MaxHealth);
			}
		}
	}

	if (AShootingGameMode* GM = Cast<AShootingGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->OnScoreChanged.AddUniqueDynamic(this,     &UPlayerHUDWidget::HandleScoreChanged);
		GM->OnComboChanged.AddUniqueDynamic(this,     &UPlayerHUDWidget::HandleComboChanged);
		GM->OnHighScoreUpdated.AddUniqueDynamic(this, &UPlayerHUDWidget::HandleHighScoreUpdated);

		HandleScoreChanged(GM->CurrentScore);
		HandleComboChanged(GM->ComboCount, GM->ComboMultiplier);
		HandleHighScoreUpdated(GM->HighScore);
	}
}

void UPlayerHUDWidget::HandleHealthChanged(float Value01, float Current, float Max)
{
	if (PB_Health) PB_Health->SetPercent(Value01);
	if (TB_HPText)
	{
		TB_HPText->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f / %.0f"), Current, Max)));
	}
}

void UPlayerHUDWidget::HandleScoreChanged(int32 NewScore)
{
	if (TB_Score)
	{
		TB_Score->SetText(FText::FromString(FString::Printf(TEXT("%07d"), NewScore)));
	}
}

void UPlayerHUDWidget::HandleComboChanged(int32 Count, float Multiplier)
{
	if (TB_Combo)
	{
		TB_Combo->SetText(FText::FromString(
			Count <= 1
				? TEXT("")
				: *FString::Printf(TEXT("x%.1f  COMBO %d"), Multiplier, Count)));
	}
}

void UPlayerHUDWidget::HandleHighScoreUpdated(int32 NewHigh)
{
	if (TB_HighScore)
	{
		TB_HighScore->SetText(FText::FromString(
			FString::Printf(TEXT("HI %07d"), NewHigh)));
	}
}
