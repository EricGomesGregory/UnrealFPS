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

	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; }
	
	//UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	//FGameplayTag GetWeaponTypeTag() const { return WeaponTypeTag; }
	
	UFUNCTION(BlueprintCallable, Category="FPS|Weapon")
	void SetFirstPersonMeshHiddenInGame(bool NewHidden);
	
	UFUNCTION(BlueprintCallable, Category="FPS|Weapon")
	void SetThirdPersonMeshHiddenInGame(bool NewHidden);
	
	void AttachToOwningPawn() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon", Meta = (Categories = "Weapon.Type"))
	FGameplayTag WeaponTypeTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	float AimFieldOfView;
	
protected:
	
	/** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	
	/** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;
	
protected:
	virtual void BeginPlay() override;

	virtual void OnRep_Instigator() override;
	
	void SetMeshVisibilities(const APawn* OwningPawn) const;
};
