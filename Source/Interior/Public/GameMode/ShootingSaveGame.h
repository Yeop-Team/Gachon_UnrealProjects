// Copyright Yeop. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShootingSaveGame.generated.h"

UCLASS(BlueprintType)
class INTERIOR_API UShootingSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	int32 HighScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	int32 BestWave = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	FDateTime LastSavedAt;
};
