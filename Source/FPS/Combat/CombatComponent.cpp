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
}

void UCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::SpawnInventoryWeapons()
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		for (TSubclassOf<AWeapon> WeaponClass : DefaultWeaponClasses)
		{
			AWeapon* WeaponInstance = SpawnWeapon(WeaponClass);
			if (IsValid(WeaponInstance))
			{
				WeaponInstance->AttachToOwningPawn();
			}
		
			InventoryWeapons.AddUnique(WeaponInstance);
		}
	
		if (InventoryWeapons.Num() > 0)
		{
			Equip(InventoryWeapons[0]);
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
