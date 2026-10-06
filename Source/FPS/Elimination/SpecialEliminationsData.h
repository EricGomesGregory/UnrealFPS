// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/FPSTypes.h"
#include "Engine/DataAsset.h"
#include "SpecialEliminationsData.generated.h"


/**
 * 
 */
USTRUCT(BlueprintType)
struct FSpecialEliminationInfo
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	ESpecialEliminationType EliminationType = ESpecialEliminationType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Message = FString();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> IconTexture;
	
	UPROPERTY(BlueprintReadOnly)
	int32 SequentialEliminationCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 StreakCount = 0;
};

/**
 * 
 */
UCLASS()
class FPS_API USpecialEliminationsData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Eliminations")
	TMap<ESpecialEliminationType, FSpecialEliminationInfo> SpecialEliminations;
};
