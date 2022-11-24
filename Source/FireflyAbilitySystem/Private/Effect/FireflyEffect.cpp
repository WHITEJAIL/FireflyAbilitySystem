// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/FireflyEffect.h"

#include "Attribute/FireflyAttributeManagerComponent.h"
#include "Effect/FireflyEffectManagerComponent.h"

UWorld* UFireflyEffect::GetWorld() const
{
	if (AActor* OwnerActor = GetOwnerActor())
	{
		return OwnerActor->GetWorld();
	}

	return nullptr;
}

AActor* UFireflyEffect::GetOwnerActor() const
{
	if(!IsValid(GetOwnerManager()))
	{
		return nullptr;
	}

	return GetOwnerManager()->GetOwner();
}

UFireflyEffectManagerComponent* UFireflyEffect::GetOwnerManager() const
{
	if (!IsValid(GetOuter()))
	{
		return nullptr;
	}

	return Cast<UFireflyEffectManagerComponent>(GetOuter());
}

void UFireflyEffect::SetTimeRemainingOfDuration(float NewDuration)
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(DurationTimer))
	{
		TimerManager.ClearTimer(DurationTimer);
	}
	TimerManager.SetTimer(DurationTimer, this, &UFireflyEffect::ExecuteEffectExpiration, NewDuration);
}

float UFireflyEffect::GetTimeRemainingOfDuration() const
{
	return GetWorld()->GetTimerManager().GetTimerRemaining(DurationTimer);
}

void UFireflyEffect::TryRefreshDurationOnStacking()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (!TimerManager.IsTimerActive(DurationTimer))
	{
		TimerManager.SetTimer(DurationTimer, this, &UFireflyEffect::ExecuteEffectExpiration, Duration);
		return;
	}

	if (!bShouldRefreshDurationOnStacking || StackingExpirationPolicy == EFireflyEffectDurationPolicyOnStackingExpired::ClearEntireStack)
	{
		return;
	}

	TimerManager.ClearTimer(DurationTimer);
	TimerManager.SetTimer(DurationTimer, this, &UFireflyEffect::ExecuteEffectExpiration, Duration);
}

void UFireflyEffect::ExecuteStackingExpirationPolicy()
{
	GetWorld()->GetTimerManager().ClearTimer(DurationTimer);

	switch (StackingExpirationPolicy)
	{
	case EFireflyEffectDurationPolicyOnStackingExpired::ClearEntireStack:
		{
			RemoveEffect();
			break;
		}
	case EFireflyEffectDurationPolicyOnStackingExpired::RemoveSingleStackAndRefreshDuration:
		{
			ReduceEffectStack(1);
			break;
		}
	case EFireflyEffectDurationPolicyOnStackingExpired::RefreshDuration:
		{
			TryRefreshDurationOnStacking();
			break;
		}
	}
}

void UFireflyEffect::TryResetPeriodicityOnStacking()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (!TimerManager.IsTimerActive(PeriodicityTimer))
	{
		TimerManager.SetTimer(PeriodicityTimer, this, &UFireflyEffect::ExecuteEffect, PeriodicInterval, true);
		return;
	}

	if (!bShouldResetPeriodicityOnStacking || StackingExpirationPolicy == EFireflyEffectDurationPolicyOnStackingExpired::ClearEntireStack)
	{
		return;
	}

	TimerManager.ClearTimer(PeriodicityTimer);
	TimerManager.SetTimer(PeriodicityTimer, this, &UFireflyEffect::ExecuteEffect, PeriodicInterval, true);
}

void UFireflyEffect::AddEffectStack(int32 StackCountToAdd)
{
	if (StackCount == StackingLimitation && bDenyNewStackingOnOverflow)
	{
		return;
	}

	if (StackingPolicy == EFireflyEffectStackingPolicy::StackHasLimit)
	{
		StackCount += StackCountToAdd;
		StackCount = FMath::Clamp<int32>(StackCount, 0, StackingLimitation);
	}
	if (StackingPolicy == EFireflyEffectStackingPolicy::StackNoLimit)
	{
		StackCount += StackCountToAdd;
	}

	if (StackCount == StackingLimitation)
	{
		ExecuteEffectStackOverflow();
		if (IsGarbageCollecting())
		{
			return;
		}
	}

	TryRefreshDurationOnStacking();

	if (bIsEffectExecutionPeriodic)
	{
		TryResetPeriodicityOnStacking();
	}
	ExecuteEffect();

	ReceiveAddEffectStack(StackCountToAdd);
}

void UFireflyEffect::ReduceEffectStack(int32 StackCountToReduce)
{
	if (StackCountToReduce == -1)
	{
		RemoveEffect();
		return;
	}

	if (StackingPolicy == EFireflyEffectStackingPolicy::StackHasLimit)
	{
		StackCount -= StackCountToReduce;
		StackCount = FMath::Clamp<int32>(StackCount, 0, StackingLimitation);
	}
	if (StackingPolicy == EFireflyEffectStackingPolicy::StackNoLimit)
	{
		StackCount -= StackCountToReduce;
	}

	if (StackCount == 0)
	{
		RemoveEffect();
		return;
	}

	TryRefreshDurationOnStacking();

	if (bIsEffectExecutionPeriodic)
	{
		TryResetPeriodicityOnStacking();
	}
	ExecuteEffect();

	ReceiveReduceEffectStack(StackCountToReduce);
}

void UFireflyEffect::ExecuteEffectStackOverflow()
{
	if (!IsValid(GetOwnerManager()))
	{
		return;
	}

	for (auto EffectType : OverflowEffects)
	{
		GetOwnerManager()->ApplyEffectToSelf(GetOwnerActor(), EffectType);
	}

	ReceiveExecuteEffectStackOverflow();

	if (bClearStackingOnOverflow)
	{
		ReduceEffectStack(-1);
	}
}

void UFireflyEffect::ApplyEffect(AActor* InInstigator, AActor* InTarget, int32 StackToApply)
{
	if (IsValid(InInstigator) && (Instigators.Contains(InInstigator)))
	{
		Instigators.Add(InInstigator);
	}

	if (IsValid(InTarget) && (Target != InTarget))
	{
		Target = InTarget;
	}

	if (DurationPolicy == EFireflyEffectDurationPolicy::Instant)
	{
		ExecuteEffect();
		RemoveEffect();
		return;
	}

	AddEffectStack(StackToApply);
}

void UFireflyEffect::ExecuteEffect()
{
	AActor* OwnerActor = GetOwnerActor();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	UFireflyAttributeManagerComponent* AttributeManager = nullptr;
	if (!IsValid(OwnerActor->GetComponentByClass(UFireflyAttributeManagerComponent::StaticClass())))
	{
		return;
	}
	AttributeManager = Cast<UFireflyAttributeManagerComponent>(OwnerActor->GetComponentByClass(UFireflyAttributeManagerComponent::StaticClass()));

	if (DurationPolicy == EFireflyEffectDurationPolicy::Instant || bIsEffectExecutionPeriodic)
	{
		for (auto Modifier : Modifiers)
		{
			AttributeManager->ApplyModifierToAttributeInstant(Modifier.AttributeType, Modifier.ModOperator, this, Modifier.ModValue);
		}
	}
	else
	{
		for (auto Modifier : Modifiers)
		{
			AttributeManager->ApplyModifierToAttribute(Modifier.AttributeType, Modifier.ModOperator, this, Modifier.ModValue, StackCount);
		}
	}

	ReceiveExecuteEffect();
}

void UFireflyEffect::ExecuteEffectExpiration()
{
	if (StackingPolicy == EFireflyEffectStackingPolicy::None)
	{
		RemoveEffect();
		ReceiveExecuteEffectExpiration();

		return;
	}

	ReceiveExecuteEffectExpiration();

	ExecuteStackingExpirationPolicy();
}

void UFireflyEffect::RemoveEffect()
{
	AActor* OwnerActor = GetOwnerActor();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	UFireflyAttributeManagerComponent* AttributeManager = nullptr;
	if (!IsValid(OwnerActor->GetComponentByClass(UFireflyAttributeManagerComponent::StaticClass())))
	{
		return;
	}
	AttributeManager = Cast<UFireflyAttributeManagerComponent>(OwnerActor->GetComponentByClass(UFireflyAttributeManagerComponent::StaticClass()));

	for (auto Modifier : Modifiers)
	{
		AttributeManager->RemoveModifierFromAttribute(Modifier.AttributeType, Modifier.ModOperator, this, Modifier.ModValue);
	}

	StackCount = 0;
	ReceiveRemoveEffect();

	MarkAsGarbage();
}
