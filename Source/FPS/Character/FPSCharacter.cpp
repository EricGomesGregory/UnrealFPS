// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "FPS/FPS.h"
#include "FPS/Combat/CombatComponent.h"
#include "FPS/Health/HealthComponent.h"
#include "FPS/Player/FPSPlayerController.h"
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
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetIsReplicated(true);
	
	TurningStatus = EFPSTurningInPlace::NotTurning;
	TurnInPlaceInterpolationSpeed = 4.0f;
	
	bWeaponFirstReplicated = false;
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

void AFPSCharacter::WeaponReplicated_Implementation()
{
	if (!bWeaponFirstReplicated)
	{
		bWeaponFirstReplicated = true;
		OnWeaponFirstReplicated.Broadcast(CombatComponent->GetCurrentWeapon());
	}
}

AWeapon* AFPSCharacter::GetCurrentWeapon_Implementation() const
{
	return CombatComponent->GetCurrentWeapon();
}

int32 AFPSCharacter::GetMagazineSize_Implementation() const
{
	return CombatComponent->GetCurrentWeapon()->GetMagazine();
}

int32 AFPSCharacter::GetReserve_Implementation() const
{
	return CombatComponent->CurrentReserves;
}

void AFPSCharacter::Notify_CycleWeapon_Implementation()
{
	CombatComponent->Notify_CycleWeapon();
}

void AFPSCharacter::Notify_ReloadWeapon_Implementation()
{
	CombatComponent->Notify_ReloadWeapon();
}

void AFPSCharacter::AddAmmoReserves_Implementation(const FGameplayTag& WeaponTypeTag, int32 Amount)
{
	if (HasAuthority() && IsValid(CombatComponent))
	{
		CombatComponent->AddAmmoReserves(WeaponTypeTag, Amount);
	}
}

bool AFPSCharacter::DoDamage_Implementation(float DamageAmount, AActor* DamageInstigator)
{
	const int32 HitMontageIndex = FMath::RandRange(0, HitReactMontages.Num() - 1);
	Multicast_HitReact(HitMontageIndex);
	
	HealthComponent->ChangeHealthByAmount(-DamageAmount, DamageInstigator);
	return false; 
}

void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	StartingRotation = FRotator(0.0f,  GetBaseAimRotation().Yaw, 0.0f);
	
	check(CombatComponent);
	CombatComponent->OnAimWeapon.AddDynamic(this, &AFPSCharacter::OnAiming);
	
	check(HealthComponent)
	HealthComponent->OnDeathStarted.AddDynamic(this, &AFPSCharacter::OnDeathStarted);
	
	if (auto* ShooterPC = Cast<AFPSPlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			ShooterPC->bPawnAlive = true;
		}
	}
}

void AFPSCharacter::BeginDestroy()
{
	Super::BeginDestroy();
	
	if (IsValid(CombatComponent))
	{
		CombatComponent->DestroyInventoryWeapons();	
	}
}

void AFPSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (CombatComponent)
	{
		CombatComponent->InitializeWeaponWidgets();	
	}
}

void AFPSCharacter::Multicast_HitReact_Implementation(int32 MontageIndex)
{
	if (GetNetMode() != NM_DedicatedServer && !IsLocallyControlled())
	{
		if (HitReactMontages.IsValidIndex(MontageIndex))
		{
			UAnimMontage* Montage = HitReactMontages[MontageIndex];
			
			for (const FSlotAnimationTrack& SlotAnimationTrack : Montage->SlotAnimTracks)
			{
				if (!GetMesh()->GetAnimInstance()->IsSlotActive(SlotAnimationTrack.SlotName))
				{
					GetMesh()->GetAnimInstance()->Montage_Play(Montage);
				}
			}
		}
		else
		{
			//@Eric TODO: Log error 
		}
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

void AFPSCharacter::OnDeathStarted(UHealthComponent* InHealthComponent)
{
	check(InHealthComponent);
	
	if (InHealthComponent != HealthComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnDeathStarted: Provided HealthComponent [%s] does not match character's HealthComponent [%s]"), *InHealthComponent->GetName(), *HealthComponent->GetName());
		return;
	}
	
	if (HasAuthority())
	{
		CombatComponent->DestroyInventoryWeapons();
	}
	
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (auto* ShooterPC = Cast<AFPSPlayerController>(GetController()))
		{
			DisableInput(ShooterPC);
			
			if (IsLocallyControlled())
			{
				ShooterPC->bPawnAlive = false;
			}
		}
		
		K2_OnDeathStarted();	
	}
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(FPSTraceChannels::ECC_Weapon, ECR_Ignore);
	
	GetMesh()->SetCollisionResponseToChannel(FPSTraceChannels::ECC_Weapon, ECR_Ignore);
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

