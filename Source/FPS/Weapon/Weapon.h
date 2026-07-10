// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

enum EPhysicalSurface : int;

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
	
	void WeaponTrace(FHitResult& OutHitResult, float TraceLength) const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon", Meta = (Categories = "Weapon.Type"))
	FGameplayTag WeaponTypeTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	float AimFieldOfView;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon|Trace")
	float TraceRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon|Debug")
	bool bDebugWeapon;
	
	void Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson);
	
protected:
	
	/** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	
	/** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void FireEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, EPhysicalSurface ImpactSurfaceType, bool bIsFirstPerson);
	
	virtual void OnRep_Instigator() override;
	
	void SetMeshVisibilities(const APawn* OwningPawn) const;
};
