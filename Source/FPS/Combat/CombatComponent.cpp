// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "FPS/Weapon/Weapon.h"
#include "GameFramework/Pawn.h"
#include "FPS/Weapon/WeaponData.h"
#include "Animation/AnimInstance.h"
#include "FPS/Interfaces/PlayerInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "FPS/FPS.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	TraceLength = 20000.0f;
	bFiring = false;
	BurstCount = 0;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, InventoryWeapons);
	DOREPLIFETIME(ThisClass, CurrentWeapon);
	
	DOREPLIFETIME_CONDITION(ThisClass, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CurrentReserves, COND_OwnerOnly);
}

void UCombatComponent::InitializeWeaponWidgets()
{
	if (IsValid(CurrentWeapon))
	{
		auto* CrosshairDynMatInst = CurrentWeapon->GetCrosshairDynamicMaterialInstance();
		OnCrosshairChanged.Broadcast(CrosshairDynMatInst, CurrentWeapon->GetCrosshairParams(), bHitPlayer);
		
		auto* MagazineDynMatInst = CurrentWeapon->GetMagazineDynamicMaterialInstance();
		const int32 Magazine = CurrentWeapon->GetMagazine();
		const int32 MagazineSize = CurrentWeapon->GetMagazineSize();
		OnMagazineChanged.Broadcast(MagazineDynMatInst, Magazine, MagazineSize);
	}
}

void UCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	auto* OwningPawn = Cast<APawn>(GetOwner());
	checkf(OwningPawn, TEXT("\nCombatComponent must be owned by a Pawn derived class"));
	
	if (OwningPawn->IsLocallyControlled())
	{
		if (auto* PC = Cast<APlayerController>(OwningPawn->GetController()))
		{
			FVector EyesWorldLocation;
			FRotator EyesWorldRotation;
			PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
			const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
			
			const FVector Start = EyesWorldLocation;
			const FVector End = Start + EyesWorldDirection * TraceLength;
			
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(OwningPawn);
			
			FCollisionResponseParams ResponseParams;
			ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
			ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
			ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
			
			GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, FPSTraceChannels::ECC_Weapon, QueryParams, ResponseParams);
			
			bHitPlayer = IsValid(HitResult.GetActor()) && HitResult.GetActor()->Implements<UPlayerInterface>();
			
			if (bHitPlayer != bHitPlayerLastFrame)
			{
				OnTargetingPlayer.Broadcast(bHitPlayer);
			}
			
			bHitPlayerLastFrame = bHitPlayer;
		}
	}
}

void UCombatComponent::SpawnInventoryWeapons()
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		for (const TSubclassOf<AWeapon> WeaponClass : DefaultWeaponClasses)
		{
			AWeapon* WeaponInstance = SpawnWeapon(WeaponClass);
			if (IsValid(WeaponInstance))
			{
				WeaponInstance->AttachToOwningPawn();
			}
		
			InventoryWeapons.AddUnique(WeaponInstance);
			Reserves.Add(WeaponInstance->WeaponTypeTag, WeaponInstance->GetReservesSize());
		}
	
		if (InventoryWeapons.Num() > 0)
		{
			Equip(InventoryWeapons[0]);
			InitializeWeaponWidgets();
		}	
	}
}

void UCombatComponent::DestroyInventoryWeapons()
{
	for (AWeapon* Weapon : InventoryWeapons)
	{
		if (IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}
}

void UCombatComponent::Equip(AWeapon* Weapon)
{
	CurrentWeapon = Weapon;
	CurrentWeapon->AttachToOwningPawn();
	
	CurrentReserves = Reserves.FindChecked(CurrentWeapon->WeaponTypeTag);
	OnCurrentReserveChanged.Broadcast(CurrentReserves, Weapon->GetMagazine(), Weapon->GetWeaponIcon());
}

void UCombatComponent::Initiate_AimWeapon_Pressed()
{
	Local_AimWeapon(true);
	Server_AimWeapon(true);
	
	auto* Owner = GetOwner();
	check(Owner);
	
	OnAimWeapon.Broadcast(true);
}

void UCombatComponent::Initiate_AimWeapon_Released()
{
	Local_AimWeapon(false);
	Server_AimWeapon(false);
	
	OnAimWeapon.Broadcast(false);
}

void UCombatComponent::Initiate_CycleWeapon()
{
	UE_LOG(LogTemp, Display, TEXT("CycleWeapon"));
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetMagazine() > 0)
		{
			bFiring = true;
			Local_FireWeapon();		
		}
		else
		{
			CurrentWeapon->DryFireEffects();
		}
	}
}

void UCombatComponent::Initiate_FireWeapon_Released()
{
	bFiring = false;
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	UE_LOG(LogTemp, Display, TEXT("ReloadWeapon"));
}

AWeapon* UCombatComponent::SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const
{
	auto* OwingPawn = Cast<APawn>(GetOwner());
	check(OwingPawn);
	
	if (OwingPawn->GetLocalRole() < ROLE_Authority) return nullptr;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = OwingPawn;
	SpawnParams.Owner = OwingPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	return GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnParams);
}

void UCombatComponent::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->AttachToOwningPawn();
		IPlayerInterface::Execute_WeaponReplicated(GetOwner());
		InitializeWeaponWidgets();
	}
}

void UCombatComponent::OnRep_CurrentReserves()
{
	if (IsValid(CurrentWeapon))
	{
		OnCurrentReserveChanged.Broadcast(CurrentReserves, CurrentWeapon->GetMagazine(), CurrentWeapon->GetWeaponIcon());
	}
}

void UCombatComponent::Server_AimWeapon_Implementation(bool bPressed)
{
	Local_AimWeapon(bPressed);
}

void UCombatComponent::Local_AimWeapon(bool bPressed)
{
	bAiming = bPressed;
}

void UCombatComponent::Server_FireWeapon_Implementation(const FHitResult& HitResult)
{
	const bool bIsLocalHost = GetNetMode() != NM_ListenServer;
	if (bIsLocalHost || !GetOwningPawn()->IsLocallyControlled())
	{
		CurrentWeapon->Auth_Fire();
	}
	Multicast_FireWeapon(HitResult, CurrentWeapon->GetMagazine());
}

void UCombatComponent::Multicast_FireWeapon_Implementation(const FHitResult& HitResult, int32 AuthAmmo)
{
	if (GetOwningPawn()->IsLocallyControlled())
	{
		CurrentWeapon->Rep_Fire(AuthAmmo);
	}
	else
	{
		ensure(WeaponsData);
		
		if (const auto* ThirdPersonMesh = IPlayerInterface::Execute_GetThirdPersonSkeletalMeshComponent(GetOwner()))
		{
			const auto& ThirdPersonMontages = WeaponsData->ThirdPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag);
			UAnimMontage* ThirdPersonMontage = ThirdPersonMontages.FireMontage;
			
			ThirdPersonMesh->GetAnimInstance()->Montage_Play(ThirdPersonMontage);

			const EPhysicalSurface ImpactSurfaceType = HitResult.PhysMaterial.IsValid(false) 
			? HitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType1;
			
			CurrentWeapon->Local_Fire(HitResult.ImpactPoint, HitResult.ImpactNormal, ImpactSurfaceType, false);
		}
	}
}

void UCombatComponent::Local_FireWeapon()
{
	ensure(WeaponsData);
	
	if (IsValid(CurrentWeapon))
	{
		if (const auto* FirstPersonMesh = IPlayerInterface::Execute_GetFirstPersonSkeletalMeshComponent(GetOwner()))
		{
			const auto& FirsPersonMontages = WeaponsData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag);
			UAnimMontage* FirstPersonMontage = FirsPersonMontages.FireMontage;
		
			FirstPersonMesh->GetAnimInstance()->Montage_Play(FirstPersonMontage);
		}
	
		FHitResult HitResult;
		CurrentWeapon->WeaponTrace(HitResult, TraceLength);

		const EPhysicalSurface ImpactSurfaceType = HitResult.PhysMaterial.IsValid(false) 
		? HitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType1;
			
		CurrentWeapon->Local_Fire(HitResult.ImpactPoint, HitResult.ImpactNormal, ImpactSurfaceType, true);
		if (CurrentWeapon->GetFireMode() == EFPSFireType::Burst)
		{
			UE_LOG(LogTemp, Display, TEXT("Burst=%d"), BurstCount);
			BurstCount++;
		}
		
		const int32 Magazine = CurrentWeapon->GetMagazine();
		const int32 MagazineSize = CurrentWeapon->GetMagazineSize();
		OnRoundFired.Broadcast(Magazine, MagazineSize, CurrentReserves);
		GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, CurrentWeapon->GetFireRate());
		
		Server_FireWeapon(HitResult);	
	}
}

void UCombatComponent::FireTimerFinished()
{
	if (CurrentWeapon->GetFireMode() == EFPSFireType::Auto)
	{
		if (bFiring && CurrentWeapon->GetMagazine() > 0)
		{
			Local_FireWeapon();
		}
	}
	else if (CurrentWeapon->GetFireMode() == EFPSFireType::Burst)
	{
		const int32 RemainingBurst = FMath::Max(CurrentWeapon->GetBurstCount() - BurstCount, 0);
		if (BurstCount < CurrentWeapon->GetBurstCount() && CurrentWeapon->GetMagazine() >= RemainingBurst)
		{
			Local_FireWeapon();
		}
		else
		{
			BurstCount = 0;
		}
	}
	
}
