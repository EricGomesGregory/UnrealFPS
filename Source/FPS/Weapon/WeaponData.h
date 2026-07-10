// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponData.generated.h"

class UBlendSpace;
class UAnimMontage;
class UAnimSequence;

/**
 * FPlayerAnimations
 */
USTRUCT(BlueprintType)
struct FPlayerAnimations
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> IdleSequence = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> AimIdleSequence = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> CrouchIdleSequence = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> SprintSequence = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> AimOffset_Hip = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> AimOffset_Aiming = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> Strafe_Standing = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> Strafe_Crouched = nullptr;
};

/**
 * FPlayerMontages
 * 
 * 
 */
USTRUCT(BlueprintType)
struct FPlayerMontages
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> EquipMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> ReloadMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FireMontage = nullptr;
};

/**
 * 
 */
UCLASS()
class FPS_API UWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UWeaponData();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	TMap<FGameplayTag, FName> GripPoints;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	TMap<FGameplayTag, FPlayerMontages> WeaponMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Animations|FirstPerson")
	TMap<FGameplayTag, FPlayerAnimations> FirstPersonAnimations;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Animations|FirstPerson")
	TMap<FGameplayTag, FPlayerMontages> FirstPersonMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Animations|ThirdPerson")
	TMap<FGameplayTag, FPlayerAnimations> ThirdPersonAnimations;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Animations|ThirdPerson")
	TMap<FGameplayTag, FPlayerMontages> ThirdPersonMontages;
};
