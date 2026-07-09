// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "FPS/Combat/CombatComponent.h"
#include "FPS/Weapon/Weapon.h"
#include "FPS/Weapon/WeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


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

void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
					OutLocation, 
					OutRotation);
			
				FABRICK_SocketTransform.SetLocation(OutLocation);
				FABRICK_SocketTransform.SetRotation(OutRotation.Quaternion());
			}
		}
	}
}

