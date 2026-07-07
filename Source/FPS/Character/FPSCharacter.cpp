// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "FPS/Combat/CombatComponent.h"
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

