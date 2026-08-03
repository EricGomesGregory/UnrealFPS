// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCrosshairWidget.h"

#include "LightmapResRatioAdjust.h"
#include "Components/Image.h"
#include "FPS/Weapon/Weapon.h"
#include "FPS/Character/FPSCharacter.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace Ammo
{
	const FName Rounds_Current = FName("Rounds_Current");
	const FName Rounds_Max = FName("Rounds_Max");
}

namespace Crosshair
{
	const FName RoundedCornerScale = FName("RoundedCornerScale");
	const FName ShapeCutThickness = FName("ShapeCutThickness");
	const FName CrosshairColor = FName("Inner_RGBA");
}

void UFPSCrosshairWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Image_Crosshair->SetRenderOpacity(0.0f);
	Image_MagazineCounter->SetRenderOpacity(0.0f);
	
	RoundFired_CornerScaleFactor = 0.0f;
	RoundFired_ShapeCutThicknessFactor = 0.0f;
	
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	
	if (auto* FPSCharacter = Cast<AFPSCharacter>(GetOwningPlayer()->GetPawn()))
	{
		OnPossessedPawnChanged(nullptr, FPSCharacter);
		
		if (FPSCharacter->HasWeaponFirstReplicated())
		{
			if (auto* CurrentWeapon = IPlayerInterface::Execute_GetCurrentWeapon(FPSCharacter))
			{
				OnCrosshairChanged(CurrentWeapon->GetCrosshairDynamicMaterialInstance(), CurrentWeapon->GetCrosshairParams(), bTargetingPlayer);
				
				const int32 Magazine = CurrentWeapon->GetMagazine();
				const int32 MagazineSize = CurrentWeapon->GetMagazineSize();
				OnMagazineChanged(CurrentWeapon->GetMagazineDynamicMaterialInstance(), Magazine, MagazineSize);
			}
		}
		else
		{
			FPSCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
		}
		
		if (FPSCharacter->HasAuthority())
		{
			if (auto* CurrentWeapon = IPlayerInterface::Execute_GetCurrentWeapon(FPSCharacter))
			{
				OnCrosshairChanged(CurrentWeapon->GetCrosshairDynamicMaterialInstance(), CurrentWeapon->GetCrosshairParams(), bTargetingPlayer);
				
				const int32 Magazine = CurrentWeapon->GetMagazine();
				const int32 MagazineSize = CurrentWeapon->GetMagazineSize();
				OnMagazineChanged(CurrentWeapon->GetMagazineDynamicMaterialInstance(), Magazine, MagazineSize);
			}
		}
	}
}

void UFPSCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	float Speed = CurrentCrosshairParams.RoundFireInterpolationSpeed;
	RoundFired_CornerScaleFactor = FMath::FInterpTo(RoundFired_CornerScaleFactor, 0.0f, InDeltaTime, Speed);
	RoundFired_ShapeCutThicknessFactor = FMath::FInterpTo(RoundFired_ShapeCutThicknessFactor, 0.0f, InDeltaTime, Speed);

	Speed = CurrentCrosshairParams.AimInterpolationSpeed;
	const float ScaleTarget = bAiming ? CurrentCrosshairParams.ScaleFactor_Aiming : BaseCornerScaleFactor;
	const float ShapeCutTarget =  bAiming ? CurrentCrosshairParams.ShapeCutFactor_Aiming : BaseShapeCutThicknessFactor;
	Aim_CornerScaleFactor = FMath::FInterpTo(Aim_CornerScaleFactor, ScaleTarget, InDeltaTime, Speed);
	Aim_ShapeCutThicknessFactor = FMath::FInterpTo(Aim_ShapeCutThicknessFactor, ShapeCutTarget, InDeltaTime, Speed);
	
	CornerScaleFactor = RoundFired_CornerScaleFactor + Aim_CornerScaleFactor;
	ShapeCutThicknessFactor = RoundFired_ShapeCutThicknessFactor + Aim_ShapeCutThicknessFactor;
	
	if (CurrentCrosshair_DynMatInst.IsValid())
	{
		CurrentCrosshair_DynMatInst->SetScalarParameterValue(Crosshair::RoundedCornerScale, CornerScaleFactor);
		CurrentCrosshair_DynMatInst->SetScalarParameterValue(Crosshair::ShapeCutThickness, ShapeCutThicknessFactor);
	}
}

void UFPSCrosshairWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (auto* OldCombatComponent = UCombatComponent::FindCombatComponent(OldPawn))
	{
		OldCombatComponent->OnCrosshairChanged.RemoveDynamic(this, &ThisClass::OnCrosshairChanged);
		OldCombatComponent->OnMagazineChanged.RemoveDynamic(this, &ThisClass::OnMagazineChanged);
		OldCombatComponent->OnAimWeapon.RemoveDynamic(this, &ThisClass::OnAimChanged);
		OldCombatComponent->OnTargetingPlayer.RemoveDynamic(this, &ThisClass::OnTargetingPlayerChanged);
		OldCombatComponent->OnRoundFired.RemoveDynamic(this, &ThisClass::OnRoundFired);
	}
	
	if (auto* CombatComponent = UCombatComponent::FindCombatComponent(NewPawn))
	{
		Image_Crosshair->SetRenderOpacity(1.0f);
		Image_MagazineCounter->SetRenderOpacity(1.0f);
		CombatComponent->OnCrosshairChanged.AddDynamic(this, &ThisClass::OnCrosshairChanged);
		CombatComponent->OnMagazineChanged.AddDynamic(this, &ThisClass::OnMagazineChanged);
		CombatComponent->OnAimWeapon.AddDynamic(this, &ThisClass::OnAimChanged);
		CombatComponent->OnTargetingPlayer.AddDynamic(this, &ThisClass::OnTargetingPlayerChanged);
		CombatComponent->OnRoundFired.AddDynamic(this, &ThisClass::OnRoundFired);
	}
}

void UFPSCrosshairWidget::OnWeaponFirstReplicated(AWeapon* Weapon)
{
	OnCrosshairChanged(Weapon->GetCrosshairDynamicMaterialInstance(), Weapon->GetCrosshairParams(), bTargetingPlayer);
	
	const int32 Magazine = Weapon->GetMagazine();
	const int32 MagazineSize = Weapon->GetMagazineSize();
	OnMagazineChanged(Weapon->GetMagazineDynamicMaterialInstance(), Magazine, MagazineSize);
}

void UFPSCrosshairWidget::OnCrosshairChanged(UMaterialInstanceDynamic* CrosshairDynMatInst, const FFPSCrosshairParams& CrosshairParams, const bool bInTargetingPlayer)
{
	CurrentCrosshairParams = CrosshairParams;
	CurrentCrosshair_DynMatInst = CrosshairDynMatInst;
	if (CurrentCrosshair_DynMatInst.IsValid())
	{
		CurrentCrosshair_DynMatInst->GetScalarParameterValue(Crosshair::RoundedCornerScale, BaseCornerScaleFactor);
		CurrentCrosshair_DynMatInst->GetScalarParameterValue(Crosshair::ShapeCutThickness, BaseShapeCutThicknessFactor);
		auto TargetColor = bInTargetingPlayer ? FLinearColor::Red : FLinearColor::White;
		CurrentCrosshair_DynMatInst->GetVectorParameterValue(Crosshair::CrosshairColor, TargetColor);
	}
	
	FSlateBrush Brush;
	Brush.SetResourceObject(CrosshairDynMatInst);
	
	check(Image_Crosshair)
	Image_Crosshair->SetBrush(Brush);
}

void UFPSCrosshairWidget::OnMagazineChanged(UMaterialInstanceDynamic* MagazineDynMatInst, int32 Current, int32 Size)
{
	CurrentMagazine_DynMatInst = MagazineDynMatInst;
	CurrentMagazine_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Current, Current);
	CurrentMagazine_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Max, Size);
	
	FSlateBrush Brush;
	Brush.SetResourceObject(MagazineDynMatInst);
	
	check(Image_MagazineCounter)
	Image_MagazineCounter->SetBrush(Brush);
}

void UFPSCrosshairWidget::OnAimChanged(bool bInAiming)
{
	bAiming = bInAiming;
}

void UFPSCrosshairWidget::OnTargetingPlayerChanged(bool bInTargetingPlayer)
{
	bTargetingPlayer = bInTargetingPlayer;
	
	if (CurrentCrosshair_DynMatInst.IsValid())
	{
		const FLinearColor TargetingColor = FLinearColor::Red; // @Eric TODO: Move to a PlayerSetting 
		const FLinearColor NewColor = bTargetingPlayer ? TargetingColor : FLinearColor::White;
		CurrentCrosshair_DynMatInst->SetVectorParameterValue(Crosshair::CrosshairColor, NewColor);
	}
}

void UFPSCrosshairWidget::OnRoundFired(int32 Current, int32 Size)
{
	RoundFired_CornerScaleFactor += CurrentCrosshairParams.ScaleFactor_RoundFired;
	RoundFired_ShapeCutThicknessFactor += CurrentCrosshairParams.ShapeCutFactor_RoundFired;
	
	if (CurrentMagazine_DynMatInst.IsValid())
	{
		CurrentMagazine_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Current, Current);
		CurrentMagazine_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Max, Size);	
	}
}
