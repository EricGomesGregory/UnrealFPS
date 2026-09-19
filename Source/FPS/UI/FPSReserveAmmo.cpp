// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSReserveAmmo.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "FPS/Character/FPSCharacter.h"
#include "FPS/Weapon/Weapon.h"
#include "rapidjson/document.h"


void UFPSReserveAmmo::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Image_WeaponIcon->SetRenderOpacity(0.0f);
	Text_Magazine->SetRenderOpacity(0.0f);
	Text_MagazineSize->SetRenderOpacity(0.0f);
	Text_Reserve->SetRenderOpacity(0.0f);
	
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	
	if (auto* FPSCharacter = Cast<AFPSCharacter>(GetOwningPlayer()->GetPawn()))
	{
		OnPossessedPawnChanged(nullptr, FPSCharacter);
		
		if (FPSCharacter->HasWeaponFirstReplicated())
		{
			if (auto* CurrentWeapon = IPlayerInterface::Execute_GetCurrentWeapon(FPSCharacter))
			{
				OnCurrentReserveChanged(IPlayerInterface::Execute_GetReserve(FPSCharacter), CurrentWeapon->GetMagazine(), CurrentWeapon->GetWeaponIcon());
			}
		}
		else
		{
			FPSCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
		}
		
		if (FPSCharacter->HasAuthority())
		{
			const auto* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(FPSCharacter);
			const auto Reserve = IPlayerInterface::Execute_GetReserve(FPSCharacter);
			
			//@Eric TODO: Fix this isn't working in Standalone 
			SetMagazineText(Weapon->GetMagazineSize());
			SetMagazineSizeText(Weapon->GetMagazineSize());
			SetReserveText(Reserve);
		}
	}
}

void UFPSReserveAmmo::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (auto* OldCombatComponent = UCombatComponent::FindCombatComponent(OldPawn))
	{
		OldCombatComponent->OnMagazineChanged.RemoveDynamic(this, &ThisClass::OnMagazineChanged);
		OldCombatComponent->OnRoundFired.RemoveDynamic(this, &ThisClass::OnRoundFired);
		OldCombatComponent->OnCurrentReserveChanged.RemoveDynamic(this, &ThisClass::OnCurrentReserveChanged);
	}
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(NewPawn))
	{
		Image_WeaponIcon->SetRenderOpacity(1.0f);
		Text_Magazine->SetRenderOpacity(1.0f);
		Text_Reserve->SetRenderOpacity(1.0f);
		Text_MagazineSize->SetRenderOpacity(1.0f);
		
		CombatComponent->OnMagazineChanged.AddDynamic(this, &ThisClass::OnMagazineChanged);
		CombatComponent->OnRoundFired.AddDynamic(this, &ThisClass::OnRoundFired);
		CombatComponent->OnCurrentReserveChanged.AddDynamic(this, &ThisClass::OnCurrentReserveChanged);
	}
}

void UFPSReserveAmmo::OnCurrentReserveChanged(int32 InReserves, int32 InWeapon, UMaterialInterface* WeaponIconMaterial)
{
	SetWeaponIcon(WeaponIconMaterial);
	
	SetMagazineText(InWeapon);
	SetMagazineSizeText(InWeapon);

	SetReserveText(InReserves);
}

void UFPSReserveAmmo::OnRoundFired(int32 Current, int32 Size, int32 Reserve)
{
	SetMagazineText(Current);
	SetMagazineSizeText(Size);

	SetReserveText(Reserve);
}

void UFPSReserveAmmo::OnMagazineChanged(UMaterialInstanceDynamic* CrosshairDynMatInst, int32 Current, int32 Size)
{
	SetMagazineText(Current);
	SetMagazineSizeText(Size);
}

void UFPSReserveAmmo::OnWeaponFirstReplicated(AWeapon* Weapon)
{
	auto* FPSCharacter = Cast<AFPSCharacter>(GetOwningPlayer()->GetPawn());
	check(FPSCharacter);
	
	OnCurrentReserveChanged(IPlayerInterface::Execute_GetReserve(FPSCharacter), Weapon->GetMagazine(), Weapon->GetWeaponIcon());
}

void UFPSReserveAmmo::SetWeaponIcon(UMaterialInterface* Material)
{
	check(Material);
	
	FSlateBrush Brush;
	Brush.SetResourceObject(Material);
	Image_WeaponIcon->SetBrush(Brush);
}

void UFPSReserveAmmo::SetMagazineText(const int32 Value) const
{
	FText MagazineText;
	if (Value > 9)
	{
		MagazineText = FText::Format(NSLOCTEXT("MagazineText", "MagazineKey", "{0}|"), Value);
	}
	else
	{
		MagazineText = FText::Format(NSLOCTEXT("MagazineText", "MagazineKey", "0{0}|"), Value);
	}
	
	Text_Magazine->SetText(MagazineText);
}

void UFPSReserveAmmo::SetMagazineSizeText(const int32 Value) const
{
	FText MagazineSizeText;
	if (Value > 9)
	{
		MagazineSizeText = FText::Format(NSLOCTEXT("MagazineText", "MagazineSizeKey", "{0}"), Value);
	}
	else
	{
		MagazineSizeText = FText::Format(NSLOCTEXT("MagazineText", "MagazineSizeKey", "0{0}"), Value);
	}
	
	Text_MagazineSize->SetText(MagazineSizeText);
}

void UFPSReserveAmmo::SetReserveText(const int32 Value) const
{
	const FText ReserveText = FText::Format(NSLOCTEXT("MagazineText", "ReserveKey", "{0}"), Value);
	Text_Reserve->SetText(ReserveText);
}
