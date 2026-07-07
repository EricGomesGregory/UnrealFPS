// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "FPS/FPSGameplayTags.h"


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
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}


