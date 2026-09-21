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
	
	Local_WeaponIndex = 0;
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
	const APawn* OwningPawn = CastChecked<APawn>(GetOwner());
	
	if (OwningPawn->GetLocalRole() == ROLE_Authority)
	{
		for (const TSubclassOf<AWeapon> WeaponClass : DefaultWeaponClasses)
		{
			AWeapon* WeaponInstance = SpawnWeapon(WeaponClass);
			if (IsValid(WeaponInstance))
			{
				WeaponInstance->AttachToOwningPawn(OwningPawn);
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
	const APawn* OwningPawn = CastChecked<APawn>(GetOwner());
	
	CurrentWeapon = Weapon;
	CurrentWeapon->SetWeaponStatus(EFPSWeaponStatus::Idle);
	CurrentWeapon->AttachToOwningPawn(OwningPawn);
	
	CurrentReserves = Reserves.FindChecked(CurrentWeapon->WeaponTypeTag);
	OnCurrentReserveChanged.Broadcast(CurrentReserves, Weapon->GetMagazine(), Weapon->GetWeaponIcon());
}

void UCombatComponent::Local_EquipWeapon(AWeapon* Weapon)
{
	const APawn* OwningPawn = CastChecked<APawn>(GetOwner());
	
	if (!IsValid(Weapon)) return;
	
	if (OwningPawn->GetLocalRole() == ROLE_Authority)
	{
		SetCurrentWeapon(Weapon, CurrentWeapon);
	}
	else
	{
		Server_EquipWeapon(Weapon);
	}
}

void UCombatComponent::Server_EquipWeapon_Implementation(AWeapon* Weapon)
{
	Local_EquipWeapon(Weapon);
}

void UCombatComponent::Initiate_AimWeapon_Pressed()
{
	const bool bIsWeaponCycling = CurrentWeapon->GetWeaponStatus() != EFPSWeaponStatus::Cycling;
	const bool bIsWeaponUnequipped = CurrentWeapon->GetWeaponStatus() != EFPSWeaponStatus::Unequipped;
	if (bIsWeaponCycling || bIsWeaponUnequipped)
	{
		Local_AimWeapon(true);
		Server_AimWeapon(true);
	
		auto* Owner = GetOwner();
		check(Owner);
	
		OnAimWeapon.Broadcast(true);
	}
	else
	{
		Initiate_AimWeapon_Released();
	}
}

void UCombatComponent::Initiate_AimWeapon_Released()
{
	Local_AimWeapon(false);
	Server_AimWeapon(false);
	
	OnAimWeapon.Broadcast(false);
}

void UCombatComponent::Initiate_CycleWeapon()
{
	if (!IsValid(CurrentWeapon)) return;
	
	if (CurrentWeapon->GetWeaponStatus() == EFPSWeaponStatus::Cycling) return;

	AdvanceWeaponIndex();
	Local_CycleWeapon(Local_WeaponIndex);
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetWeaponStatus() == EFPSWeaponStatus::Idle)
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
}

void UCombatComponent::Initiate_FireWeapon_Released()
{
	bFiring = false;
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	UE_LOG(LogTemp, Display, TEXT("ReloadWeapon"));
}

void UCombatComponent::Notify_CycleWeapon()
{
	if (!IsValid(CurrentWeapon)) return;
	
	AWeapon* NewWeapon = InventoryWeapons[Local_WeaponIndex];
	if (IsValid(NewWeapon))
	{
		Local_EquipWeapon(NewWeapon);
	}
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
	APawn* OwningPawn = CastChecked<APawn>(GetOwner());
	
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
	
	IPlayerInterface::Execute_WeaponReplicated(OwningPawn);
	InitializeWeaponWidgets();
}

void UCombatComponent::OnRep_CurrentReserves()
{
	if (IsValid(CurrentWeapon))
	{
		OnCurrentReserveChanged.Broadcast(CurrentReserves, CurrentWeapon->GetMagazine(), CurrentWeapon->GetWeaponIcon());
	}
}

void UCombatComponent::BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	const APawn* OwningPawn = CastChecked<APawn>(GetOwner());
	UAnimInstance* AnimInstance = IPlayerInterface::Execute_GetFirstPersonSkeletalMeshComponent(OwningPawn)->GetAnimInstance();
	if (IsValid(AnimInstance) && AnimInstance->OnMontageBlendingOut.IsAlreadyBound(this, &ThisClass::BlendOut_CycleWeapon))
	{
		AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &ThisClass::BlendOut_CycleWeapon);
	}
	
	CurrentWeapon->SetWeaponStatus(EFPSWeaponStatus::Idle);
}

void UCombatComponent::Server_AimWeapon_Implementation(bool bPressed)
{
	Local_AimWeapon(bPressed);
}

void UCombatComponent::SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* OldWeapon)
{
	const APawn* OwningPawn = CastChecked<APawn>(GetOwner());
	
	AWeapon* LocalLastWeapon = nullptr;
	if (IsValid(OldWeapon))
	{
		LocalLastWeapon = OldWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}
	
	if (IsValid(LocalLastWeapon))
	{
		LocalLastWeapon->DetachFromOwningPawn();
		LocalLastWeapon->SetWeaponStatus(EFPSWeaponStatus::Unequipped);
	}
	
	CurrentWeapon = NewWeapon;
	CurrentWeapon->SetWeaponStatus(EFPSWeaponStatus::Idle);
	CurrentWeapon->AttachToOwningPawn(OwningPawn);
	
	if (OwningPawn->HasAuthority() && IsValid(CurrentWeapon))
	{
		CurrentReserves = Reserves.FindChecked(CurrentWeapon->WeaponTypeTag);
	}
}

void UCombatComponent::Local_AimWeapon(bool bPressed)
{
	bAiming = bPressed;
}

void UCombatComponent::Server_CycleWeapon_Implementation(const int32 WeaponIndex)
{
	Local_WeaponIndex = WeaponIndex;
	Multicast_CycleWeapon_Implementation(WeaponIndex);
}

void UCombatComponent::Multicast_CycleWeapon_Implementation(const int32 WeaponIndex)
{
	APawn* OwningPawn = CastChecked<APawn>(GetOwner());
	
	if (!OwningPawn->IsLocallyControlled())
	{
		Local_WeaponIndex = WeaponIndex;
		Local_CycleWeapon(WeaponIndex);
	}
}

void UCombatComponent::Local_CycleWeapon(const int32 WeaponIndex)
{
	AWeapon* NextWeapon = InventoryWeapons[WeaponIndex];
	if (!IsValid(NextWeapon) || !IsValid(WeaponsData)) return;
	CurrentWeapon->SetWeaponStatus(EFPSWeaponStatus::Cycling);
	NextWeapon->SetWeaponStatus(EFPSWeaponStatus::Cycling);
	
	APawn* OwningPawn = CastChecked<APawn>(GetOwner());
	
	if (OwningPawn->IsLocallyControlled())
	{
		const auto& Montages = WeaponsData->FirstPersonMontages.FindChecked(NextWeapon->WeaponTypeTag);
		const auto* Mesh = IPlayerInterface::Execute_GetFirstPersonSkeletalMeshComponent(OwningPawn);
	
		if (IsValid(Mesh))
		{
			if (IsValid(Montages.EquipMontage))
			{
				Mesh->GetAnimInstance()->Montage_Play(Montages.EquipMontage);
			}
			
			Mesh->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &ThisClass::BlendOut_CycleWeapon);
		}
		
		Server_CycleWeapon(WeaponIndex);
	}
	else
	{
		const auto& Montages = WeaponsData->ThirdPersonMontages.FindChecked(NextWeapon->WeaponTypeTag);
		const auto* Mesh = IPlayerInterface::Execute_GetThirdPersonSkeletalMeshComponent(OwningPawn);
	
		if (IsValid(Mesh) && IsValid(Montages.EquipMontage))
		{
			Mesh->GetAnimInstance()->Montage_Play(Montages.EquipMontage);
		}
	}
}

void UCombatComponent::Server_FireWeapon_Implementation(const FHitResult& HitResult)
{
	if (!IsValid(CurrentWeapon)) return;
	if (CurrentWeapon->GetMagazine() <= 0) return;
	
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
		CurrentWeapon->SetWeaponStatus(EFPSWeaponStatus::Firing);
		
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
	CurrentWeapon->SetWeaponStatus(EFPSWeaponStatus::Idle);
	
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

int32 UCombatComponent::AdvanceWeaponIndex()
{
	if (InventoryWeapons.Num() >= 2)
	{
		Local_WeaponIndex = (Local_WeaponIndex + 1) % InventoryWeapons.Num();
	}
	
	return Local_WeaponIndex;
}
