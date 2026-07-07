// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/Interfaces/PlayerInterface.h"
#include "GameFramework/Character.h"
#include "FPSCharacter.generated.h"

class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;

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
	//~End IPlayerInterface
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	void ToggleCrouch();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnAiming(bool bIsAiming);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Character")
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void BeginDestroy() override;
};
