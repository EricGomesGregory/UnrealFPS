// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "FPS/Combat/CombatComponent.h"
#include "FPS/Weapon/Weapon.h"
#include "FPS/Weapon/WeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"


AFPSCharacter::AFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 15.0f;
	SpringArm->bUsePawnControlRotation = true;
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(SpringArm);
	FirstPersonCamera->bUsePawnControlRotation = false;
	
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->bOnlyOwnerSee = true;
	FirstPersonMesh->bOwnerNoSee = false;
	FirstPersonMesh->bCastDynamicShadow = false;
	FirstPersonMesh->bReceivesDecals = false;
	FirstPersonMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
	FirstPersonMesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	auto* ThirdPersonMesh = GetMesh();
	ThirdPersonMesh->bOnlyOwnerSee = false;
	ThirdPersonMesh->bOwnerNoSee = true;
	ThirdPersonMesh->bReceivesDecals = false;
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);
	
	TurningStatus = EFPSTurningInPlace::NotTurning;
	TurnInPlaceInterpolationSpeed = 4.0f;
}

FName AFPSCharacter::GetWeaponAttachPointSocketName_Implementation(const FGameplayTag& WeaponTyeTag) const
{
	auto* WeaponsData = CombatComponent->GetWeaponsData();
	checkf(WeaponsData, TEXT("Weapons data has not been setup"));

	return WeaponsData->GripPoints.FindChecked(WeaponTyeTag);
}

USkeletalMeshComponent* AFPSCharacter::GetFirstPersonSkeletalMeshComponent_Implementation() const
{
	return FirstPersonMesh;
}

USkeletalMeshComponent* AFPSCharacter::GetThirdPersonSkeletalMeshComponent_Implementation() const
{
	return GetMesh();
}

void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	StartingRotation = FRotator(0.0f,  GetBaseAimRotation().Yaw, 0.0f);
	
	check(CombatComponent);
	CombatComponent->OnAimWeapon.AddDynamic(this, &AFPSCharacter::OnAiming);
}

void AFPSCharacter::BeginDestroy()
{
	Super::BeginDestroy();
	
	if (IsValid(CombatComponent))
	{
		CombatComponent->DestroyInventoryWeapons();	
	}
}

void AFPSCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateTurnInPlaceParameters(DeltaTime);
	FABRIK_CalculateSocketTransform();
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AFPSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	check(CombatComponent);
	
	CombatComponent->SpawnInventoryWeapons();
}

void AFPSCharacter::ToggleCrouch()
{
	if (IsCrouched())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

FRotator AFPSCharacter::GetFixedAimedRotation() const
{
	FRotator AimRotation = GetBaseAimRotation();
	if ((AimRotation.Pitch > 90.0f || AimRotation.Pitch < 0.0f) && !IsLocallyControlled())
	{
		const FVector2D InRange(270.0f, 360.0f);
		const FVector2D OutRange(-90.0f, 0.0f);
		
		AimRotation.Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimRotation.Pitch);
	}
	return AimRotation;
}

void AFPSCharacter::FABRIK_CalculateSocketTransform()
{
	if (IsValid(CombatComponent))
	{
		if (const auto* CurrentWeapon = CombatComponent->GetCurrentWeapon())
		{
			const auto* WeaponThirdPersonMesh = CurrentWeapon->GetThirdPersonMesh();
			if (IsValid(WeaponThirdPersonMesh))
			{
				FABRICK_SocketTransform = WeaponThirdPersonMesh->GetSocketTransform("FABRIK_Socket", RTS_World);
				
				FVector OutLocation = FVector::ZeroVector;
				FRotator OutRotation = FRotator::ZeroRotator;
				
				GetMesh()->TransformToBoneSpace(
					"hand_r", 
					FABRICK_SocketTransform.GetLocation(), 
					FABRICK_SocketTransform.GetRotation().Rotator(), 
					OutLocation, OutRotation);
			
				FABRICK_SocketTransform.SetLocation(OutLocation);
				FABRICK_SocketTransform.SetRotation(OutRotation.Quaternion());
			}
		}
	}
}

void AFPSCharacter::CalculateTurnInPlaceParameters(const float DeltaTime)
{
	const FVector Velocity = GetVelocity();
	const float Speed = Velocity.Size2D();
	const bool bIsFalling = GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.0f && !bIsFalling)
	{
		const FRotator CurrentRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, StartingRotation);
		AO_Yaw =  DeltaRotation.Yaw;
		
		if (TurningStatus == EFPSTurningInPlace::NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.0f || bIsFalling)
	{
		StartingRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;

		const FRotator AimRotation = GetBaseAimRotation();
		const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		TurningStatus = EFPSTurningInPlace::NotTurning;
	}
	
	AO_Yaw *= -1.0f;
}

void AFPSCharacter::TurnInPlace(const float DeltaTime)
{
	if (AO_Yaw > 90.0f)
	{
		TurningStatus = EFPSTurningInPlace::Right;
	}
	else if (AO_Yaw < -90.0f)
	{
		TurningStatus = EFPSTurningInPlace::Left;
	}
	
	if (TurningStatus != EFPSTurningInPlace::NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, TurnInPlaceInterpolationSpeed);
		AO_Yaw = InterpAO_Yaw;
		
		if (FMath::Abs(AO_Yaw) < TurnInPlaceMinYaw)
		{
			TurningStatus = EFPSTurningInPlace::NotTurning;
			StartingRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

