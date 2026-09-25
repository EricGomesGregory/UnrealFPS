// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

class UInputAction;
struct FInputActionValue;
class UInputMappingContext;


USTRUCT(BlueprintType)
struct FFPSInputMappingContext
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> MappingContext = nullptr;
	
	UPROPERTY(EditAnywhere)
	int32 Priority = 1;
};

/**
 * 
 */
UCLASS()
class FPS_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFPSPlayerController();

protected:
	UPROPERTY(EditDefaultsOnly, Category="FPS|Input")
	TArray<FFPSInputMappingContext> MappingContexts;

	UPROPERTY(EditDefaultsOnly, Category="FPS|Input|Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category="FPS|Input|Movement")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Input|Movement")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Input|Movement")
	TObjectPtr<UInputAction> CrouchAction;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Input|Combat")
	TObjectPtr<UInputAction> AimWeaponAction;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Input|Combat")
	TObjectPtr<UInputAction> CycleWeaponAction;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Input|Combat")
	TObjectPtr<UInputAction> FireWeaponAction;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Input|Combat")
	TObjectPtr<UInputAction> ReloadWeaponAction;
	
public:
	bool bPawnAlive;
	
protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
	
private:
	/**  */
	void Input_Move(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_Look(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_Jump(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_Crouch(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_AimWeapon_Pressed(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_AimWeapon_Released(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_CycleWeapon(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_FireWeapon_Pressed(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_FireWeapon_Released(const FInputActionValue& InputActionValue);
	
	/**  */
	void Input_ReloadWeapon(const FInputActionValue& InputActionValue);
};
