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
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UFireflyEffectManagerComponent::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void UFireflyEffectManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UFireflyEffectManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UFireflyEffectManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

TArray<UFireflyEffect*> UFireflyEffectManagerComponent::GetActiveEffectsByClass(TSubclassOf<UFireflyEffect> EffectType) const
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

	const TArray<UFireflyEffect*> ActiveSpecEffects = GetActiveEffectsByClass(EffectType);

	/** 管理器中目前如果不存在被应用的指定效果，则创建一个新效果，应用该效果 */
	if (ActiveSpecEffects.Num() == 0)
	{
		UFireflyEffect* NewEffect = NewObject<UFireflyEffect>(this, EffectType);
		NewEffect->ApplyEffect(Instigator, GetOwner(), StackToApply);

		return;
	}

	/** 如果指定效果的默认发起者应用策略为每个发起者应用各自的实例 */
	UFireflyEffect* EffectCDO = Cast<UFireflyEffect>(EffectType->GetDefaultObject());
	if (EffectCDO->InstigatorApplicationPolicy == EFireflyEffectInstigatorApplicationPolicy::InstigatorsApplyTheirOwn)
	{
		bool bContainsInstigator = false;
		for (auto Effect : ActiveSpecEffects)
		{
			if (Effect->Instigators.Contains(Instigator))
			{
				bContainsInstigator = true;
				break;
			}			
		}

		// 如果指定效果在该管理器中目前不存在已经存在的，和InInstigator相同的发起者，则创建一个新效果，应用该效果
		if (!bContainsInstigator)
		{
			UFireflyEffect* NewEffect = NewObject<UFireflyEffect>(this, EffectType);
			NewEffect->ApplyEffect(Instigator, GetOwner(), StackToApply);

			return;
		}
	}

	/** 不考虑特殊情况，将所有当前存在的执行效果重新应用，具体的判定逻辑在效果内部执行 */
	for (auto Effect : ActiveSpecEffects)
	{
		Effect->ApplyEffect(Instigator, GetOwner(), StackToApply);
	}
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

	TArray<UFireflyEffect*> EffectsToRemove = TArray<UFireflyEffect*>{};
	for (auto Effect : ActiveEffects)
	{
		if (Effect->GetClass() == EffectType)
		{
			EffectsToRemove.Add(Effect);
		}
	}

	for (auto Effect : EffectsToRemove)
	{
		if (Effect->ReduceEffectStack(StackToRemove))
		{
			EffectsToRemove.RemoveSingle(Effect);
			Effect->RemoveEffect();
		}
	}
}

void UFireflyEffectManagerComponent::AddOrRemoveActiveEffect(UFireflyEffect* InEffect, bool bIsAdd)
{
	if (bIsAdd)
	{
		ActiveEffects.Emplace(InEffect);
	}
	else
	{
		ActiveEffects.RemoveSingle(InEffect);
	}
}

bool UFireflyEffectManagerComponent::GetSingleActiveEffectTimeDuration(TSubclassOf<UFireflyEffect> EffectType,
	float& TimeRemaining, float& TotalDuration) const
{
	if (!IsValid(EffectType))
	{
		return false;
	}

	const TArray<UFireflyEffect*> Effects = GetActiveEffectsByClass(EffectType);
	if (!Effects.IsValidIndex(0))
	{
		return false;
	}

	TimeRemaining = Effects[0]->GetTimeRemainingOfDuration();
	TotalDuration = Effects[0]->Duration;

	return true;
}

bool UFireflyEffectManagerComponent::GetSingleActiveEffectStackingCount(TSubclassOf<UFireflyEffect> EffectType,
	int32& StackingCount) const
{
	if (!IsValid(EffectType))
	{
		return false;
	}

	const TArray<UFireflyEffect*> Effects = GetActiveEffectsByClass(EffectType);
	if (!Effects.IsValidIndex(0))
	{
		return false;
	}

	StackingCount = Effects[0]->StackCount;

	return true;
}
