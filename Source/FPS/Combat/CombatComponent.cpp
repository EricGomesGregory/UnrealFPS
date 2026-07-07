// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Engine/World.h"
#include "FPS/Weapon/Weapon.h"
#include "GameFramework/Pawn.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::SpawnInventoryWeapons()
{
	AWeapon* NewWeapon = SpawnWeapon(DefaultWeaponClass);
	if (IsValid(NewWeapon))
	{
		NewWeapon->AttachToOwningPawn();
	}
}

void UCombatComponent::DestroyInventoryWeapons()
{
	//@Eric TODO: Implement this 
}

void UCombatComponent::Initiate_AimWeapon_Pressed()
{
	UE_LOG(LogTemp, Display, TEXT("AimWeapon::Pressed"));
}

void UCombatComponent::Initiate_AimWeapon_Released()
{
	UE_LOG(LogTemp, Display, TEXT("AimWeapon::Released"));
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
