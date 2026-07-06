// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"

#include "FPS/Character/FPSCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"


AFPSPlayerController::AFPSPlayerController()
{
	bReplicates = true;
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
}

void AFPSPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
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
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
	{
		ControlledPawn->AddControllerYawInput(InputAxisVector.X);
		ControlledPawn->AddControllerPitchInput(InputAxisVector.Y);
	}
}

void AFPSPlayerController::Input_Jump(const FInputActionValue& InputActionValue)
{
	ACharacter* ControlledCharacter = GetCharacter();
	
	if (ControlledCharacter)
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
	auto* ControlledCharacter = Cast<AFPSCharacter>(GetCharacter());
	if (ControlledCharacter)
	{
		ControlledCharacter->ToggleCrouch();
	}
}
