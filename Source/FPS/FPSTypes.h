// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FPSTypes.generated.h"

UENUM(BlueprintType)
enum class EFPSTurningInPlace : uint8
{
	NotTurning UMETA(DisplayName = "NotTurning"),
	Left  UMETA(DisplayName = "TurningLeft"),
	Right UMETA(DisplayName = "TurningRight"),
};

USTRUCT(BlueprintType)
struct FFPSCrosshairParams
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Aiming Shape Cut Thickness Factor"))
	float ShapeCutFactor_Aiming = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Aiming Rounded Corner Scale"))
	float ScaleFactor_Aiming = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AimInterpolationSpeed = 15.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Fireing Shape Cut Thickness Factor"))
	float ShapeCutFactor_RoundFired = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Fireing Rounded Corner Scale"))
	float ScaleFactor_RoundFired = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fireing")
	float RoundFireInterpolationSpeed = 20.0f;
	
	//@Eric TODO: Maybe implement targeting scale factor and shape cut factors 
};
