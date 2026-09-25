// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"

#include "FPS/Combat/CombatComponent.h"
#include "FPS/Character/FPSCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"


AFPSPlayerController::AFPSPlayerController()
{
	bReplicates = true;
	
	bPawnAlive = true;
}

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	const auto* LP = GetLocalPlayer();
	check(LP);
	
	auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);
	
	Subsystem->ClearAllMappings();
		
	for (FFPSInputMappingContext& Entry : MappingContexts)
	{
		Subsystem->AddMappingContext(Entry.MappingContext, Entry.Priority);
	}
	
	auto* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::Input_Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::Input_Look);
	
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFPSPlayerController::Input_Jump);
	
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AFPSPlayerController::Input_Crouch);
	
	
	EnhancedInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Started, this, &AFPSPlayerController::Input_AimWeapon_Pressed);
	EnhancedInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Completed, this, &AFPSPlayerController::Input_AimWeapon_Released);
	
	EnhancedInputComponent->BindAction(CycleWeaponAction, ETriggerEvent::Started, this, &AFPSPlayerController::Input_CycleWeapon);
	
	EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &AFPSPlayerController::Input_FireWeapon_Pressed);
	EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &AFPSPlayerController::Input_FireWeapon_Released);
	
	EnhancedInputComponent->BindAction(ReloadWeaponAction, ETriggerEvent::Started, this, &AFPSPlayerController::Input_ReloadWeapon);
}

void AFPSPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	if (APawn* ControlledPawn = GetPawn())
	{
		const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
		
		if (InputAxisVector.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			ControlledPawn->AddMovementInput(MovementDirection, InputAxisVector.X);
		}

		if (InputAxisVector.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			ControlledPawn->AddMovementInput(MovementDirection, InputAxisVector.Y);
		}
	}
}

void AFPSPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddControllerYawInput(InputAxisVector.X);
		ControlledPawn->AddControllerPitchInput(InputAxisVector.Y);
	}
}

void AFPSPlayerController::Input_Jump(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		if (ControlledCharacter->IsCrouched())
		{
			ControlledCharacter->UnCrouch();
		}
		
		ControlledCharacter->Jump();
	}
}

void AFPSPlayerController::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	if (auto* ControlledCharacter = Cast<AFPSCharacter>(GetCharacter()))
	{
		ControlledCharacter->ToggleCrouch();
	}
}

void AFPSPlayerController::Input_AimWeapon_Pressed(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(GetCharacter()))
	{
		CombatComponent->Initiate_AimWeapon_Pressed();
	}
}

void AFPSPlayerController::Input_AimWeapon_Released(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(GetCharacter()))
	{
		CombatComponent->Initiate_AimWeapon_Released();
	}
}

void AFPSPlayerController::Input_CycleWeapon(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(GetCharacter()))
	{
		CombatComponent->Initiate_CycleWeapon();
	}
}

void AFPSPlayerController::Input_FireWeapon_Pressed(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(GetCharacter()))
	{
		CombatComponent->Initiate_FireWeapon_Pressed();
	}
}

void AFPSPlayerController::Input_FireWeapon_Released(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(GetCharacter()))
	{
		CombatComponent->Initiate_FireWeapon_Released();
	}
}

void AFPSPlayerController::Input_ReloadWeapon(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(GetCharacter()))
	{
		CombatComponent->Initiate_ReloadWeapon();
	}
}
