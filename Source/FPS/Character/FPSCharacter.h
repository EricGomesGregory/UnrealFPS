// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/FPSTypes.h"
#include "FPS/Combat/CombatComponent.h"
#include "FPS/Interfaces/PlayerInterface.h"
#include "GameFramework/Character.h"
#include "FPSCharacter.generated.h"

class USpringArmComponent;
class UHealthComponent;
class UCombatComponent;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFPSWeaponFirstReplicated, AWeapon*, Weapon);

UCLASS()
class FPS_API AFPSCharacter : public ACharacter,
public IPlayerInterface
{
	GENERATED_BODY()

public:
	AFPSCharacter();

	//~IPlayerInterface
	virtual FName GetWeaponAttachPointSocketName_Implementation(const FGameplayTag& WeaponTyeTag) const override;
	virtual USkeletalMeshComponent* GetFirstPersonSkeletalMeshComponent_Implementation() const override;
	virtual USkeletalMeshComponent* GetThirdPersonSkeletalMeshComponent_Implementation() const override;
	virtual void WeaponReplicated_Implementation() override;
	virtual AWeapon* GetCurrentWeapon_Implementation() const override;
	virtual int32 GetMagazineSize_Implementation() const override;
	virtual int32 GetReserve_Implementation() const override;
	virtual void Notify_CycleWeapon_Implementation() override;
	virtual void Notify_ReloadWeapon_Implementation() override;
	virtual void AddAmmoReserves_Implementation(const FGameplayTag& WeaponTypeTag, int32 Amount) override;
	virtual bool DoDamage_Implementation(float DamageAmount, AActor* DamageInstigator) override;
	//~End IPlayerInterface
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	void ToggleCrouch();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnAiming(bool bIsAiming);
	
	UFUNCTION(BlueprintCallable, Category="FPS|Character|Animation")
	FRotator GetFixedAimedRotation() const;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|Character|Animation")
	FTransform FABRICK_SocketTransform;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|Character|Animation")
	float AO_Yaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|Character|Animation")
	float MovementOffsetYaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|Character|Animation")
	EFPSTurningInPlace TurningStatus;
	
	UFUNCTION(BlueprintCallable, Category="FPS|Character|Animation")
	bool HasCurrentWeapon() const { return (CombatComponent ? CombatComponent->GetCurrentWeapon() != nullptr : false); }
	
	bool HasWeaponFirstReplicated() const { return bWeaponFirstReplicated; }
	
	/** Delegate for client synchronization */
	UPROPERTY(BlueprintAssignable)
	FFPSWeaponFirstReplicated OnWeaponFirstReplicated;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Character")
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Character")
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(EditAnywhere, Category="FPS|Character|Animation")
	float TurnInPlaceInterpolationSpeed;
	
	UPROPERTY(EditAnywhere, Category="FPS|Character|Animation")
	float TurnInPlaceMinYaw = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Character|Animation")
	TArray<TObjectPtr<UAnimMontage>> HitReactMontages;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void BeginDestroy() override;
	
	virtual void OnRep_PlayerState() override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HitReact(int32 MontageIndex);
	
	UFUNCTION()
	void OnDeathStarted(UHealthComponent* InHealthComponent);
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathStarted"))
	void K2_OnDeathStarted();
	
private:
	void FABRIK_CalculateSocketTransform();
	
	void CalculateTurnInPlaceParameters(float DeltaTime);
	
	void TurnInPlace(float DeltaTime);
	
private:
	FRotator StartingRotation;
	
	float InterpAO_Yaw;
	
	bool bWeaponFirstReplicated;
};
