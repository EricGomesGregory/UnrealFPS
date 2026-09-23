// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

class AWeapon;
struct FGameplayTag;


UINTERFACE()
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FPS_API IPlayerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FName GetWeaponAttachPointSocketName(const FGameplayTag& WeaponTyeTag) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonSkeletalMeshComponent() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonSkeletalMeshComponent() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void WeaponReplicated();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AWeapon* GetCurrentWeapon() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetMagazineSize() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetReserve() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Notify_CycleWeapon();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Notify_ReloadWeapon();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddAmmoReserves(const FGameplayTag& WeaponTypeTag, int32 Amount);
};
