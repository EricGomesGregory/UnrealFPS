// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "Net/UnrealNetwork.h"


UHealthComponent::UHealthComponent()
: Health(100.0f), MaxHealth(100.0f)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
	
	DeathState = EDeathState::NotDead;
}

float UHealthComponent::GetHealthNormalized() const
{
	return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

bool UHealthComponent::ChangeHealthByAmount(const float Amount, AActor* Instigator)
{
	check(Instigator);
	
	const float OldHealth = Health;
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(this, OldHealth, Health, Instigator);
	if (Health <= 0.0f && OldHealth > 0.0f)
	{
		StartDeath();
	}
	return false;
}

void UHealthComponent::ChangeMaxHealthByAmount(const float Amount, AActor* Instigator)
{
	check(Instigator);
	
	const float OldMaxHealth = MaxHealth;
	MaxHealth = FMath::Max(MaxHealth + Amount, 1.0f);
	OnMaxHealthChanged.Broadcast(this, OldMaxHealth, MaxHealth, Instigator);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, DeathState);
	
	DOREPLIFETIME(ThisClass, MaxHealth);
	DOREPLIFETIME(ThisClass, Health);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHealthComponent::StartDeath()
{
	if (DeathState != EDeathState::NotDead) return;
	
	DeathState = EDeathState::DeathStarted;
	OnDeathStarted.Broadcast(this);
	GetOwner()->ForceNetUpdate();
}

void UHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	if (DeathState == EDeathState::DeathStarted)
	{
		OnDeathStarted.Broadcast(this);
	}
}

void UHealthComponent::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(this, OldHealth, Health, nullptr);
}

void UHealthComponent::OnRep_MaxHealth(float OldMaxHealth)
{
	OnMaxHealthChanged.Broadcast(this, OldMaxHealth, MaxHealth, nullptr);
}
