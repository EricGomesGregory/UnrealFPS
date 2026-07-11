// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSCrosshairWidget.generated.h"

class AWeapon;
class UImage;

/**
 * 
 */
UCLASS()
class FPS_API UFPSCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Crosshair;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_MagazineCounter;
	
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentCrosshair_DynMatInst;
	
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentMagazine_DynMatInst;
	
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnWeaponFirstReplicated(AWeapon* Weapon);
	
	UFUNCTION()
	void OnCrosshairChanged(UMaterialInstanceDynamic* CrosshairDynMatInst);

	UFUNCTION()
	void OnMagazineChanged(UMaterialInstanceDynamic* MagazineDynMatInst, int32 Current, int32 Size);
};
