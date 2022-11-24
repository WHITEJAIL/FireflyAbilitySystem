// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/FireflyEffectManagerComponent.h"

#include "Effect/FireflyEffect.h"

// Sets default values for this component's properties
UFireflyEffectManagerComponent::UFireflyEffectManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UFireflyEffectManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UFireflyEffectManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

TArray<UFireflyEffect*> UFireflyEffectManagerComponent::GetActiveEffectByClass(TSubclassOf<UFireflyEffect> EffectType) const
{
	TArray<UFireflyEffect*> OutEffects = TArray<UFireflyEffect*>{};
	for (auto Effect : ActiveEffects)
	{
		if (Effect->GetClass() == EffectType)
		{
			OutEffects.Emplace(Effect);
		}
	}

	return OutEffects;
}

void UFireflyEffectManagerComponent::ApplyEffectToSelf(AActor* Instigator, TSubclassOf<UFireflyEffect> EffectType, int32 StackToApply)
{
	if (!IsValid(EffectType) || StackToApply <= 0)
	{
		return;
	}

	if (!IsValid(Instigator))
	{
		Instigator = GetOwner();
	}

	const TArray<UFireflyEffect*> ActiveSpecEffects = GetActiveEffectByClass(EffectType);
	if (ActiveSpecEffects.Num() == 0)
	{
		UFireflyEffect* NewEffect = NewObject<UFireflyEffect>(this, EffectType);
		NewEffect->ApplyEffect(Instigator, GetOwner(), StackToApply);

		return;
	}

	UFireflyEffect* EffectCDO = Cast<UFireflyEffect>(EffectType->GetDefaultObject());
}

void UFireflyEffectManagerComponent::ApplyEffectToTarget(AActor* Target, TSubclassOf<UFireflyEffect> EffectType,
	int32 StackToApply)
{
	if (!IsValid(Target) || !IsValid(EffectType) || StackToApply <= 0)
	{
		return;
	}

	UFireflyEffectManagerComponent* TargetEffectMgr = nullptr;
	if (!IsValid(Target->GetComponentByClass(UFireflyEffectManagerComponent::StaticClass())))
	{
		return;
	}

	TargetEffectMgr = Cast<UFireflyEffectManagerComponent>(Target->GetComponentByClass(UFireflyEffectManagerComponent::StaticClass()));
	TargetEffectMgr->ApplyEffectToSelf(GetOwner(), EffectType, StackToApply);
}

void UFireflyEffectManagerComponent::RemoveActiveEffectFromSelf(TSubclassOf<UFireflyEffect> EffectType,
	int32 StackToRemove)
{
	if (!IsValid(EffectType))
	{
		return;
	}	
}
