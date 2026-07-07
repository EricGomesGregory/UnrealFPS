// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class FPS_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

	UFUNCTION(BlueprintCallable, Category="FPS|Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	
	UFUNCTION(BlueprintCallable, Category="FPS|Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; }
	
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon", Meta = (Categories = "Weapon.Type"))
	FGameplayTag WeaponTypeTag;
	
	/** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	
	/** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;
	
protected:
	virtual void BeginPlay() override;

};
