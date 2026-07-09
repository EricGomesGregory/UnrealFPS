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