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


UENUM(meta=(BitFlags))
enum class ESpecialEliminationType : uint16 
{
	None = 0,
	HeadShot		= 1 << 0, //		00000000 00000001
	Sequential		= 1 << 1, //		00000000 00000010
	Streak 			= 1 << 2, //		00000000 00000100
	Revenge			= 1 << 3, //		00000000 00001000
	Dethrone		= 1 << 4, //		00000000 00010000
	ShowStopper		= 1 << 5, //		00000000 00100000
	FirstBlood		= 1 << 6, //		00000000 01000000
	GainedTheLead	= 1 << 7, //		00000000 10000000
	TiedTheLeader	= 1 << 8, //		00000001 00000000
	LostTheLead		= 1 << 9, //		00000010 00000000
	//@Eric TODO: Add additional special eliminations
};

ENUM_CLASS_FLAGS(ESpecialEliminationType)


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
