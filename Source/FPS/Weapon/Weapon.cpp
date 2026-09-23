// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "FPS/FPS.h"
#include "KismetTraceUtils.h"
#include "GameFramework/Pawn.h"
#include "FPS/FPSGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "FPS/Interfaces/PlayerInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	FirstPersonMesh->bReceivesDecals = false;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetHiddenInGame(true);
	SetRootComponent(FirstPersonMesh);
	
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	ThirdPersonMesh->bReceivesDecals = false;
	ThirdPersonMesh->SetupAttachment(FirstPersonMesh);
	ThirdPersonMesh->SetHiddenInGame(true);
	
	WeaponTypeTag = ShooterGameplayTags::Weapon_Type_None;
	AimFieldOfView = 65.0f;
	TraceRadius = 5.0f;
	FireMode = EFPSFireType::SemiAuto;
	RoundsPerMinute = 300.0f;
	BurstCount = 3;
	
	MagazineSize = 10;
	ReservesSize = 20;
	
	Sequence = 0;
	WeaponStatus = EFPSWeaponStatus::Unequipped;
}

void AWeapon::SetWeaponStatus(const EFPSWeaponStatus NewStatus)
{
	switch (NewStatus)
	{
	case Idle:
		WeaponStatus = NewStatus;
		break;
	case Firing:
		if (WeaponStatus == EFPSWeaponStatus::Idle)
		{
			WeaponStatus = NewStatus;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to set new state from=%s to=%s"), *GetWeaponStatusName(WeaponStatus), *GetWeaponStatusName(NewStatus));
		}
		break;
	case Reloading:
		if (WeaponStatus == EFPSWeaponStatus::Idle)
		{
			WeaponStatus = NewStatus;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to set new state from=%s to=%s"), *GetWeaponStatusName(WeaponStatus), *GetWeaponStatusName(NewStatus));
		}
		break;
	case Cycling:
		if (WeaponStatus == EFPSWeaponStatus::Idle)
		{
			WeaponStatus = NewStatus;
		}
		else if (WeaponStatus == EFPSWeaponStatus::Cycling)
		{
			WeaponStatus = NewStatus;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to set new state from=%s to=%s"), *GetWeaponStatusName(WeaponStatus), *GetWeaponStatusName(NewStatus));
		}
		break;
	case Unequipped:
		if (WeaponStatus == EFPSWeaponStatus::Cycling)
		{
			WeaponStatus = NewStatus;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to set new state from=%s to=%s"), *GetWeaponStatusName(WeaponStatus), *GetWeaponStatusName(NewStatus));
		}
		break;
	}
	UE_LOG(LogTemp, Display, TEXT("[%s] Current=%d Next=%d"), *GetName(), WeaponStatus, NewStatus);
}

int32 AWeapon::GetMagazine() const
{
	if (GetInstigator())
	{
		return Magazine;
	}
	
	return GetMagazineSize();
}

void AWeapon::SetMagazine(const int32 NewMagazine)
{
	Magazine = NewMagazine;
}

int32 AWeapon::GetReserves() const
{
	if (GetInstigator())
	{
		return Reserves;
	}
	
	return GetReservesSize();
}

void AWeapon::SetReserves(const int32 NewReserves)
{
	Reserves = NewReserves;
}

void AWeapon::SetFirstPersonMeshHiddenInGame(bool NewHidden)
{
	FirstPersonMesh->SetHiddenInGame(NewHidden);
}

void AWeapon::SetThirdPersonMeshHiddenInGame(bool NewHidden)
{
	ThirdPersonMesh->SetHiddenInGame(NewHidden);
}

UMaterialInstanceDynamic* AWeapon::GetCrosshairDynamicMaterialInstance()
{
	if (!IsValid(DynMatInst_Crosshair))
	{
		DynMatInst_Crosshair = UMaterialInstanceDynamic::Create(CrosshairMaterial, this);
	}
		
	return DynMatInst_Crosshair;
	
	
}

UMaterialInstanceDynamic* AWeapon::GetMagazineDynamicMaterialInstance()
{
	if (!IsValid(DynMatInst_Magazine))
	{
		DynMatInst_Magazine = UMaterialInstanceDynamic::Create(MagazineMaterial, this);
	}
	
	return DynMatInst_Magazine;
}

void AWeapon::AttachToOwningPawn(const APawn* OwningPawn) const
{
	if (IsValid(OwningPawn) && OwningPawn->Implements<UPlayerInterface>())
	{
		SetMeshVisibilities(OwningPawn);
		
		const FName AttachmentSocketName = IPlayerInterface::Execute_GetWeaponAttachPointSocketName(OwningPawn, WeaponTypeTag);
		USkeletalMeshComponent* PawnFirstPersonMesh = IPlayerInterface::Execute_GetFirstPersonSkeletalMeshComponent(OwningPawn);
		USkeletalMeshComponent* PawnThirdPersonMesh = IPlayerInterface::Execute_GetThirdPersonSkeletalMeshComponent(OwningPawn);
	
		FirstPersonMesh->AttachToComponent(PawnFirstPersonMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachmentSocketName);
		ThirdPersonMesh->AttachToComponent(PawnThirdPersonMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachmentSocketName);
	}
}

void AWeapon::DetachFromOwningPawn()
{
	FirstPersonMesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	FirstPersonMesh->SetHiddenInGame(true);
	
	ThirdPersonMesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	ThirdPersonMesh->SetHiddenInGame(true);
}

void AWeapon::WeaponTrace(FHitResult& OutHitResult, float TraceLength) const
{
	ensure(GetInstigator());
	
	if (auto* PC = Cast<APlayerController>(GetInstigator()->GetController()))
	{
		FCollisionQueryParams TraceParameters;
		TraceParameters.bReturnPhysicalMaterial = true;
		TraceParameters.AddIgnoredActor(GetOwner());
	
		FCollisionResponseParams ResponseParams;
		ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
		ResponseParams.CollisionResponse.SetResponse(ECC_WorldStatic, ECR_Block);
		ResponseParams.CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Block);
		ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
		ResponseParams.CollisionResponse.SetResponse(FPSTraceChannels::ECC_Weapon, ECR_Ignore);
		
		FVector EyesWorldLocation;
		FRotator EyesWorldRotation;
		PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
		const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
		
		const FVector Start = EyesWorldLocation;
		const FVector End = Start + (EyesWorldDirection * TraceLength);
		
		const bool bHit = GetWorld()->SweepSingleByChannel(
			OutHitResult, 
			Start, 
			End, 
			FQuat::Identity, 
			FPSTraceChannels::ECC_Weapon,
			FCollisionShape::MakeSphere(TraceRadius), 
			TraceParameters,
			ResponseParams);
	
		if (bHit == false)
		{
			OutHitResult.ImpactPoint = End;
		}
		
#if !UE_BUILD_SHIPPING
		if (bDebugWeapon)
		{
			DrawDebugSphereTraceSingle(
			GetWorld(),
			Start,
			End,
			TraceRadius,
			EDrawDebugTrace::ForDuration,
			bHit,
			OutHitResult,
			FColor::Green,
			FColor::Red,
			5.0f);
		}
#endif
	}
}

void AWeapon::Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson)
{	
	FireEffects(ImpactPoint, ImpactNormal, ImpactSurfaceType, bIsFirstPerson);
	
	if (GetInstigator()->IsLocallyControlled())
	{
		Magazine = FMath::Clamp(Magazine - 1, 0, MagazineSize);
		if (!GetInstigator()->HasAuthority())
		{
			++Sequence;
		}
	}
}

int32 AWeapon::Auth_Fire()
{
	Magazine = FMath::Clamp(Magazine - 1, 0, MagazineSize);
	return Magazine;
}

void AWeapon::Rep_Fire(const int32 AuthAmmo)
{
	if (GetInstigator()->IsLocallyControlled() && !GetInstigator()->HasAuthority())
	{
		Magazine = AuthAmmo;
		--Sequence;
		Magazine = FMath::Clamp(Magazine - Sequence, 0, MagazineSize);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	Magazine = MagazineSize;
	Reserves = ReservesSize;
}

void AWeapon::SetMeshVisibilities(const APawn* OwningPawn) const
{
	check(OwningPawn);
	
	const bool bVisibleInFirstPerson = !OwningPawn->IsLocallyControlled();
	const bool bVisibleInThirdPerson = OwningPawn->IsLocallyControlled();
	
	FirstPersonMesh->SetHiddenInGame(bVisibleInFirstPerson);
	ThirdPersonMesh->SetHiddenInGame(bVisibleInThirdPerson);
}

FString AWeapon::GetWeaponStatusName(const EFPSWeaponStatus Status)
{
	switch (Status) {
	case Idle:
		return "Idle";
	case Firing:
		return "Firing";
	case Reloading:
		return "Reloading";
	case Cycling:
		return "Cycling";
	case Unequipped:
		return "Unequipped";
	}
	return "NULL";
}




