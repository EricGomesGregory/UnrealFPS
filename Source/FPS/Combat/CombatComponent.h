// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class UWeaponData;


UCLASS()
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	/** Returns the combat component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "FPS|Combat")
	static UCombatComponent* FindCombatComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UCombatComponent>() : nullptr); }
	
	UWeaponData* GetWeaponsData() const { return WeaponsData; }
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void SpawnInventoryWeapons();
	
	void DestroyInventoryWeapons();
	
	/**  */
	void Initiate_AimWeapon_Pressed();
	
	/**  */
	void Initiate_AimWeapon_Released();
	
	/** Begin cycle to next weapon in inventory */
	void Initiate_CycleWeapon();
	
	/**  */
	void Initiate_FireWeapon_Pressed();
	
	/**  */
	void Initiate_FireWeapon_Released();
	
	/**  */
	void Initiate_ReloadWeapon();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	TObjectPtr<UWeaponData> WeaponsData;

	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
};
