// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCrosshairWidget.h"

#include "Components/Image.h"
#include "FPS/Weapon/Weapon.h"
#include "FPS/Character/FPSCharacter.h"
#include "Materials/MaterialInstanceDynamic.h"


void UFPSCrosshairWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	
	if (auto* FPSCharacter = Cast<AFPSCharacter>(GetOwningPlayer()->GetPawn()))
	{
		OnPossessedPawnChanged(nullptr, FPSCharacter);
		
		if (FPSCharacter->HasWeaponFirstReplicated())
		{
			if (auto* CurrentWeapon = IPlayerInterface::Execute_GetCurrentWeapon(FPSCharacter))
			{
				OnCrosshairChanged(CurrentWeapon->GetCrosshairDynamicMaterialInstance());
				
				const int32 Magazine = CurrentWeapon->GetMagazine();
				const int32 MagazineSize = CurrentWeapon->GetMagazineSize();
				OnMagazineChanged(CurrentWeapon->GetMagazineDynamicMaterialInstance(), Magazine, MagazineSize);
			}
		}
		else
		{
			FPSCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
		}
	}
}

void UFPSCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UFPSCrosshairWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (auto* OldCombatComponent = UCombatComponent::FindCombatComponent(OldPawn))
	{
		OldCombatComponent->OnCrosshairChanged.RemoveDynamic(this, &ThisClass::OnCrosshairChanged);
		OldCombatComponent->OnMagazineChanged.RemoveDynamic(this, &ThisClass::OnMagazineChanged);	
	}
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(NewPawn))
	{
		CombatComponent->OnCrosshairChanged.AddDynamic(this, &ThisClass::OnCrosshairChanged);
		CombatComponent->OnMagazineChanged.AddDynamic(this, &ThisClass::OnMagazineChanged);
	}
}

void UFPSCrosshairWidget::OnWeaponFirstReplicated(AWeapon* Weapon)
{
	OnCrosshairChanged(Weapon->GetCrosshairDynamicMaterialInstance());
	
	const int32 Magazine = Weapon->GetMagazine();
	const int32 MagazineSize = Weapon->GetMagazineSize();
	OnMagazineChanged(Weapon->GetMagazineDynamicMaterialInstance(), Magazine, MagazineSize);
}

void UFPSCrosshairWidget::OnCrosshairChanged(UMaterialInstanceDynamic* CrosshairDynMatInst)
{
	CurrentCrosshair_DynMatInst = CrosshairDynMatInst;
	
	FSlateBrush Brush;
	Brush.SetResourceObject(CrosshairDynMatInst);
	
	check(Image_Crosshair)
	Image_Crosshair->SetBrush(Brush);
}

void UFPSCrosshairWidget::OnMagazineChanged(UMaterialInstanceDynamic* MagazineDynMatInst, int32 Current, int32 Size)
{
	CurrentMagazine_DynMatInst = MagazineDynMatInst;
	
	FSlateBrush Brush;
	Brush.SetResourceObject(MagazineDynMatInst);
	
	check(Image_MagazineCounter)
	Image_MagazineCounter->SetBrush(Brush);
}
