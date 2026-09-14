// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSReserveAmmo.generated.h"

class AWeapon;
class UImage;
class UTextBlock;


/**
 * 
 */
UCLASS()
class FPS_API UFPSReserveAmmo : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;

protected:
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnCurrentReserveChanged(int32 InReserves, int32 InWeapon);
	
	UFUNCTION()
	void OnRoundFired(int32 Current, int32 Size, int32 Reserve);
	
	UFUNCTION()
	void OnMagazineChanged(UMaterialInstanceDynamic* CrosshairDynMatInst, int32 Current, int32 Size);
	
	UFUNCTION()
	void OnWeaponFirstReplicated(AWeapon* Weapon);
	
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UImage> Image_WeaponIcon;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Magazine;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_MagazineSize;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Reserve;
	
private:
	void SetMagazineText(int32 Value);
	
	void SetMagazineSizeText(int32 Value);
	
	void SetReserveText(int32 Value);
};
