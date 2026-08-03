// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class UWeaponData;
struct FFPSCrosshairParams;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatToggleActionEvent, bool, bPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCombatCrosshairChanged, UMaterialInstanceDynamic*, CrosshairDynMatInst, const FFPSCrosshairParams&, CrosshairParams, bool, bTargetingPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCombatMagazineChanged, UMaterialInstanceDynamic*, CrosshairDynMatInst, int32, Current, int32, Size);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatMagazineDelegate, int32, Current, int32, Size);

UCLASS()
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure, Category="FPS")
	APawn* GetOwningPawn() const { return (GetOwner() ? Cast<APawn>(GetOwner()) : nullptr); }
	
	/** Returns the combat component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "FPS|Combat")
	static UCombatComponent* FindCombatComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UCombatComponent>() : nullptr); }
	
	UWeaponData* GetWeaponsData() const { return WeaponsData; }
	
	UFUNCTION(BlueprintPure, Category = "FPS|Combat")
	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }
	
	void InitializeWeaponWidgets();
	
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
	
	UPROPERTY(BlueprintAssignable)
	FCombatCrosshairChanged OnCrosshairChanged;
	
	UPROPERTY(BlueprintAssignable)
	FCombatMagazineChanged OnMagazineChanged;
	
	UPROPERTY(BlueprintAssignable)
	FCombatMagazineDelegate OnRoundFired;
	
	UPROPERTY(BlueprintAssignable)
	FCombatToggleActionEvent OnTargetingPlayer;
	
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
	
	bool bFiring;
	
	bool bHitPlayer;
	bool bHitPlayerLastFrame;
	
	int32 BurstCount;
	
	FTimerHandle FireTimer;
	
private:
	UFUNCTION(Server, Reliable)
	void Server_AimWeapon(bool bPressed);
	
	void Local_AimWeapon(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(const FHitResult& HitResult);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon(const FHitResult& HitResult, int32 AuthAmmo);
	
	void Local_FireWeapon();
	
	void FireTimerFinished();
};
