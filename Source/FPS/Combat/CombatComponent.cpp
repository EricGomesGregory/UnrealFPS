// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "FPS/Weapon/Weapon.h"
#include "GameFramework/Pawn.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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
	Local_Aim(true);
	Server_Aim(true);
	
	auto* Owner = GetOwner();
	check(Owner);
	
	OnAimWeapon.Broadcast(true);
}

void UCombatComponent::Initiate_AimWeapon_Released()
{
	Local_Aim(false);
	Server_Aim(false);
	
	OnAimWeapon.Broadcast(false);
}

void UCombatComponent::Initiate_CycleWeapon()
{
	UE_LOG(LogTemp, Display, TEXT("CycleWeapon"));
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	UE_LOG(LogTemp, Display, TEXT("FireWeapon::Pressed"));
}

void UCombatComponent::Initiate_FireWeapon_Released()
{
	UE_LOG(LogTemp, Display, TEXT("FireWeapon::Released"));
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

void UCombatComponent::Server_Aim_Implementation(bool bPressed)
{
	Local_Aim(bPressed);
}

void UCombatComponent::Local_Aim(bool bPressed)
{
	bAiming = bPressed;
}