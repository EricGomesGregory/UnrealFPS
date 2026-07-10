// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "FPS/FPS.h"
#include "KismetTraceUtils.h"
#include "GameFramework/Pawn.h"
#include "FPS/FPSGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "FPS/Interfaces/PlayerInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"


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
	AimFieldOfView = 65.0f;
	TraceRadius = 5.0f;
}

void AWeapon::SetFirstPersonMeshHiddenInGame(bool NewHidden)
{
	FirstPersonMesh->SetHiddenInGame(NewHidden);
}

void AWeapon::SetThirdPersonMeshHiddenInGame(bool NewHidden)
{
	ThirdPersonMesh->SetHiddenInGame(NewHidden);
}

void AWeapon::AttachToOwningPawn() const
{
	const auto* OwningPawn = GetInstigator();
	
	if (IsValid(OwningPawn) && OwningPawn->Implements<UPlayerInterface>())
	{
		SetMeshVisibilities(OwningPawn);
		
		const FName AttachmentSocketName = IPlayerInterface::Execute_GetWeaponAttachPointSocketName(OwningPawn, WeaponTypeTag);
		USkeletalMeshComponent* PawnFirstPersonMesh = IPlayerInterface::Execute_GetFirstPersonSkeletalMeshComponent(OwningPawn);
		USkeletalMeshComponent* PawnThirdPersonMesh = IPlayerInterface::Execute_GetThirdPersonSkeletalMeshComponent(OwningPawn);
	
		FirstPersonMesh->AttachToComponent(PawnFirstPersonMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachmentSocketName);
		ThirdPersonMesh->AttachToComponent(PawnThirdPersonMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachmentSocketName);
	}
}

void AWeapon::WeaponTrace(FHitResult& OutHitResult, float TraceLength) const
{
	ensure(GetInstigator());
	
	if (auto* PC = Cast<APlayerController>(GetInstigator()->GetController()))
	{
		FCollisionQueryParams TraceParameters;
		TraceParameters.bReturnPhysicalMaterial = true;
		TraceParameters.AddIgnoredActor(GetOwner());
	
		FCollisionResponseParams ResponseParams;
		ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
		ResponseParams.CollisionResponse.SetResponse(ECC_WorldStatic, ECR_Block);
		ResponseParams.CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Block);
		ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
		ResponseParams.CollisionResponse.SetResponse(FPSTraceChannels::ECC_Weapon, ECR_Ignore);
		
		FVector EyesWorldLocation;
		FRotator EyesWorldRotation;
		PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
		const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
		
		const FVector Start = EyesWorldLocation;
		const FVector End = Start + (EyesWorldDirection * TraceLength);
		
		const bool bHit = GetWorld()->SweepSingleByChannel(
			OutHitResult, 
			Start, 
			End, 
			FQuat::Identity, 
			FPSTraceChannels::ECC_Weapon,
			FCollisionShape::MakeSphere(TraceRadius), 
			TraceParameters,
			ResponseParams);
	
		if (bHit == false)
		{
			OutHitResult.ImpactPoint = End;
		}
		
#if !UE_BUILD_SHIPPING
		if (bDebugWeapon)
		{
			DrawDebugSphereTraceSingle(
			GetWorld(),
			Start,
			End,
			TraceRadius,
			EDrawDebugTrace::ForDuration,
			bHit,
			OutHitResult,
			FColor::Green,
			FColor::Red,
			5.0f);
		}
#endif
	}
}

void AWeapon::Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson)
{	
	FireEffects(ImpactPoint, ImpactNormal, ImpactSurfaceType, bIsFirstPerson);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::OnRep_Instigator()
{
	Super::OnRep_Instigator();
	
	AttachToOwningPawn();
}

void AWeapon::SetMeshVisibilities(const APawn* OwningPawn) const
{
	check(OwningPawn);
	
	const bool bVisibleInFirstPerson = !OwningPawn->IsLocallyControlled();
	const bool bVisibleInThirdPerson = OwningPawn->IsLocallyControlled();
	
	FirstPersonMesh->SetHiddenInGame(bVisibleInFirstPerson);
	ThirdPersonMesh->SetHiddenInGame(bVisibleInThirdPerson);
}


