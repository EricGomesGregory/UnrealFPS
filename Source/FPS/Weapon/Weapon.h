// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/FPSTypes.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Weapon.generated.h"

class UMaterialInstanceDynamic;

enum EPhysicalSurface : int;


UENUM(BlueprintType)
enum EFPSFireType : uint8
{
	SemiAuto UMETA(DisplayName="SemiAutomatic"),
	Burst UMETA(DisplayName="Burst"),
	Auto UMETA(DisplayName="Automatic"),
};


UENUM(BlueprintType)
enum EFPSWeaponStatus : uint8
{
	Idle UMETA(DisplayName="Idle, can go to any state"),
	Firing UMETA(DisplayName="Firing, can't Reload/Cycle"),
	Reloading UMETA(DisplayName="Reloading, can't Reload/Cycle"),
	Cycling UMETA(DisplayName="Cycling, can't Fire/Reload/Cycle"),
	Unequipped UMETA(DisplayName="Unequipped, can only Cycle"),
};


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
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	EFPSWeaponStatus GetWeaponStatus() const { return WeaponStatus; }
	
	UFUNCTION(BlueprintCallable, Category="FPS|Weapon")
	void SetWeaponStatus(const EFPSWeaponStatus NewStatus);
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	float GetBaseDamage() const { return BaseDamage; }
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	EFPSFireType GetFireMode() const { return FireMode; }
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	int32 GetBurstCount() const { return BurstCount; }
	
	/** Returns the time between shots in seconds */
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	float GetFireRate() const { return 36.0f / RoundsPerMinute; }
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	int32 GetMagazineSize() const { return MagazineSize; }
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	int32 GetMagazine() const;
	
	void SetMagazine(const int32 NewMagazine);
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	int32 GetReservesSize() const { return ReservesSize; }
	
	UFUNCTION(BlueprintPure, Category="FPS|Weapon")
	int32 GetReserves() const;
	
	void SetReserves(const int32 NewReserves);
	
	UFUNCTION(BlueprintCallable, Category="FPS|Weapon")
	void SetFirstPersonMeshHiddenInGame(bool NewHidden);
	
	UFUNCTION(BlueprintCallable, Category="FPS|Weapon")
	void SetThirdPersonMeshHiddenInGame(bool NewHidden);
	
	/**  */
	UMaterialInstanceDynamic* GetCrosshairDynamicMaterialInstance();
	
	/**  */
	UMaterialInstanceDynamic* GetMagazineDynamicMaterialInstance();
	
	FFPSCrosshairParams GetCrosshairParams() const { return CrosshairParams; }
	
	UMaterialInterface* GetWeaponIcon() const { return WeaponIcon; }
	
	void AttachToOwningPawn(const APawn* OwningPawn) const;
	
	void DetachFromOwningPawn();
	
	void WeaponTrace(FHitResult& OutHitResult, float TraceLength) const;
	
	void Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson);
	
	int32 Auth_Fire();
	
	void Rep_Fire(int32 AuthAmmo);
	
	UFUNCTION(BlueprintImplementableEvent)
	void DryFireEffects();
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon", Meta = (Categories = "Weapon.Type"))
	FGameplayTag WeaponTypeTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	float AimFieldOfView;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon|Trace")
	float TraceRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon|Debug")
	bool bDebugWeapon;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	float BaseDamage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	TEnumAsByte<EFPSFireType> FireMode;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon", meta=(EditConditionHides, EditCondition="FireMode==EFPSFireType::Burst"))
	int32 BurstCount;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon", meta=(ClampMin = 150.0f, ClampMax = 900.0f, EditConditionHides, EditCondition="FireMode!=EFPSFireType::SemiAuto"))
	float RoundsPerMinute;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	int32 MagazineSize;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="FPS|Weapon")
	int32 Magazine;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	int32 ReservesSize;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="FPS|Weapon")
	int32 Reserves;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon|UI")
	TObjectPtr<UMaterialInterface> CrosshairMaterial;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon|UI")
	TObjectPtr<UMaterialInterface> MagazineMaterial;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon|UI")
	FFPSCrosshairParams CrosshairParams;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon|UI")
	TObjectPtr<UMaterialInterface> WeaponIcon;
	
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
	
	void SetMeshVisibilities(const APawn* OwningPawn) const;
	
private:
	/** Magazine client-side prediction counter */
	int32 Sequence;
	
	/**  */
	EFPSWeaponStatus WeaponStatus;
	
	/**  */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynMatInst_Crosshair;
	
	/**  */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynMatInst_Magazine;
	
private:
	static FString GetWeaponStatusName(const EFPSWeaponStatus Status);
};
