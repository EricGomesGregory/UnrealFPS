// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class UWeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatToggleActionEvent, bool, bPressed);

UCLASS()
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** Returns the combat component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "FPS|Combat")
	static UCombatComponent* FindCombatComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UCombatComponent>() : nullptr); }
	
	UWeaponData* GetWeaponsData() const { return WeaponsData; }
	
	UFUNCTION(BlueprintPure, Category = "FPS|Combat")
	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void SpawnInventoryWeapons();
	
	void DestroyInventoryWeapons();
	
	void Equip(AWeapon* Weapon);
	
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
	
public:
	UPROPERTY(BlueprintReadOnly, Replicated, Category="FPS|Weapon")
	bool bAiming;
	
	mutable FCombatToggleActionEvent OnAimWeapon;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category="FPS|Weapon")
	bool bFiring;
	
	mutable FCombatToggleActionEvent OnFireWeapon;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	TObjectPtr<UWeaponData> WeaponsData;

	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TArray<TSubclassOf<AWeapon>> DefaultWeaponClasses;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	float TraceLength;
	
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
	
protected:
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);
	
private:
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentWeapon, meta=(AllowPrivateAccess=true))
	TObjectPtr<AWeapon> CurrentWeapon;
	
	UPROPERTY(Transient, Replicated)
	TArray<AWeapon*> InventoryWeapons;
	
private:
	UFUNCTION(Server, Reliable)
	void Server_AimWeapon(bool bPressed);
	
	void Local_AimWeapon(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(bool bPressed, const FHitResult& HitResult);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon(bool bPressed, const FHitResult& HitResult);
	
	void Local_FireWeapon(bool bPressed);
};
