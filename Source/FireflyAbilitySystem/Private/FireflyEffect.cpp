// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyEffect.h"

#include "FireflyAbilitySystemComponent.h"

UFireflyEffect::UFireflyEffect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* UFireflyEffect::GetWorld() const
{
	if (AActor* OwnerActor = GetOwnerActor())
	{
		return OwnerActor->GetWorld();
	}

	return nullptr;
}

bool UFireflyEffect::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	if (!IsValid(GetOwnerActor()))
	{
		return false;
	}

	UNetDriver* NetDriver = GetOwnerActor()->GetNetDriver();
	if (!NetDriver)
	{
		return false;
	}

	NetDriver->ProcessRemoteFunction(GetOwnerActor(), Function, Parms, OutParms, Stack, this);

	return true;
}

int32 UFireflyEffect::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	return (GetOuter() ? GetOuter()->GetFunctionCallspace(Function, Stack) : FunctionCallspace::Local);
}

AActor* UFireflyEffect::GetOwnerActor() const
{
	if(!IsValid(GetOwnerManager()))
	{
		return nullptr;
	}

	return GetOwnerManager()->GetOwner();
}

UFireflyAbilitySystemComponent* UFireflyEffect::GetOwnerManager() const
{
	if (!IsValid(GetOuter()))
	{
		return nullptr;
	}

	return Cast<UFireflyAbilitySystemComponent>(GetOuter());
}

void UFireflyEffect::SetTimeRemainingOfDuration(float NewDuration)
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (!TimerManager.IsTimerActive(DurationTimer))
	{
		return;
	}

	TimerManager.ClearTimer(DurationTimer);
	TimerManager.SetTimer(DurationTimer, this, 
		&UFireflyEffect::ExecuteEffectExpiration, NewDuration);
}

float UFireflyEffect::GetTimeRemainingOfDuration() const
{
	return GetWorld()->GetTimerManager().GetTimerRemaining(DurationTimer);
}

void UFireflyEffect::TryExecuteOrRefreshDuration()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	/** 如果持续时间尚未开始计时，则执行计时器 */
	if (!TimerManager.IsTimerActive(DurationTimer))
	{
		TimerManager.SetTimer(DurationTimer, this, &UFireflyEffect::ExecuteEffectExpiration, Duration);
		return;
	}

	/** 如果堆叠不会刷新执行时间，或者堆叠到期策略为清理所有堆叠数，则不会刷新持续时间 */
	if (!bShouldRefreshDurationOnStacking || StackingExpirationPolicy == EFireflyEffectDurationPolicyOnStackingExpired::ClearEntireStack)
	{
		return;
	}

	/** 刷新持续时间 */
	TimerManager.ClearTimer(DurationTimer);
	TimerManager.SetTimer(DurationTimer, this, &UFireflyEffect::ExecuteEffectExpiration, Duration);
}

void UFireflyEffect::TryExecuteOrResetPeriodicity()
{
	/** 如果该效果不是周期性执行，直接返回 */
	if (!bIsEffectExecutionPeriodic)
	{
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	/** 如果效果的周期性执行尚未开始，则开始计时器，执行周期性逻辑 */
	if (!TimerManager.IsTimerActive(PeriodicityTimer))
	{
		TimerManager.SetTimer(PeriodicityTimer, this, &UFireflyEffect::ExecuteEffect, PeriodicInterval, true);
		return;
	}

	/** 如果堆叠不会重置周期性执行，或者堆叠到期策略为清理所有堆叠数，则不会重置周期性执行 */
	if (!bShouldResetPeriodicityOnStacking || StackingExpirationPolicy == EFireflyEffectDurationPolicyOnStackingExpired::ClearEntireStack)
	{
		return;
	}

	/** 重置周期性执行 */
	TimerManager.ClearTimer(PeriodicityTimer);
	TimerManager.SetTimer(PeriodicityTimer, this, &UFireflyEffect::ExecuteEffect, PeriodicInterval, true);
}

void UFireflyEffect::AddEffectStack(int32 StackCountToAdd)
{
	if (StackCountToAdd <= 0)
	{
		return;
	}

	/** 如果已经开始堆叠 && 满堆叠 && 满堆叠时拒绝新的堆叠实例，直接返回 */
	if (StackCount != 0 && StackCount == StackingLimitation && bDenyNewStackingOnOverflow)
	{
		return;
	}

	/** 添加堆叠数，如果堆叠有上限，执行夹值 */
	float OldStackCount = StackCount;
	StackCount += StackCountToAdd;
	if (StackingPolicy == EFireflyEffectStackingPolicy::StackHasLimit)
	{		
		StackCount = FMath::Clamp<int32>(StackCount, 0, StackingLimitation);
	}
	
	ReceiveAddEffectStack(StackCountToAdd);
}

bool UFireflyEffect::ReduceEffectStack(int32 StackCountToReduce)
{
	if (StackCountToReduce >= 0)
	{
		return false;
	}

	/** 减少堆叠数 */
	float OldStackCount = StackCount;
	StackCount = FMath::Clamp<int32>(StackCount - StackCountToReduce, 0, OldStackCount);

	ReceiveReduceEffectStack(StackCountToReduce);

	if (StackCount == 0)
	{
		return true;
	}

	return false;
}

bool UFireflyEffect::TryExecuteEffectStackOverflow()
{
	/** 如果管理器不存在 || 堆叠数未达到最大值 || 效果的堆叠不受限制，直接返回 */
	if (!IsValid(GetOwnerManager()) || StackCount != StackingLimitation
		|| StackingPolicy != EFireflyEffectStackingPolicy::StackHasLimit)
	{
		return false;
	}

	/** 应用堆叠数达到上限时触发的额外效果 */
	for (auto EffectType : OverflowEffects)
	{
		GetOwnerManager()->ApplyEffectToSelf(GetOwnerActor(), EffectType);
	}

	ReceiveExecuteEffectStackOverflow();

	return true;
}

void UFireflyEffect::ApplyEffect(AActor* InInstigator, AActor* InTarget, int32 StackToApply)
{
	/** 如果持续时间策略为Instant，直接按照申请的堆叠次数执行逻辑，并结束效果的应用 */
	if (DurationPolicy == EFireflyEffectDurationPolicy::Instant)
	{
		Instigators.Contains(InInstigator);
		Target = InTarget;

		for (int i = 0; i < StackToApply; ++i)
		{
			ExecuteEffect();
		}
		
		RemoveEffect();

		return;
	}

	/** 如果发起者应用策略为InstigatorsShareOne，尝试将新的发起者并入效果发起者数组中 */
	if (InstigatorApplicationPolicy == EFireflyEffectInstigatorApplicationPolicy::InstigatorsShareOne
		&& !Instigators.Contains(InInstigator))
	{
		Instigators.AddUnique(InInstigator);
	}

	/** 如果该效果不可堆叠 */
	if (StackingPolicy == EFireflyEffectStackingPolicy::None)
	{
		/** 第一次执行效果逻辑 */
		if (!GetWorld()->GetTimerManager().IsTimerActive(DurationTimer))
		{
			GetOwnerManager()->AddOrRemoveActiveEffect(this, true);
			ExecuteEffect();
		}

		/** 有新的应用被申请，尝试刷新持续时间，尝试重置周期性执行执行 */
		TryExecuteOrRefreshDuration();
		TryExecuteOrResetPeriodicity();

		return;
	}

	/** 先添加堆叠次数 */
	AddEffectStack(StackToApply);

	/** 第一次执行有堆叠的效果逻辑 */
	if (!GetWorld()->GetTimerManager().IsTimerActive(DurationTimer))
	{
		GetOwnerManager()->AddOrRemoveActiveEffect(this, true);
		ExecuteEffect();
	}

	/** 尝试执行满堆叠时的逻辑，如果满堆叠时清理堆叠并结束执行，直接结束效果 */
	if (TryExecuteEffectStackOverflow() && bClearStackingOnOverflow)
	{
		RemoveEffect();		
	}

	/** 尝试刷新持续时间，尝试重置周期性执行执行 */
	TryExecuteOrRefreshDuration();
	TryExecuteOrResetPeriodicity();
}

void UFireflyEffect::ExecuteEffect()
{
	UFireflyAbilitySystemComponent* AbilitySystemManager = GetOwnerManager();
	if (!IsValid(AbilitySystemManager))
	{
		return;
	}

	if (DurationPolicy == EFireflyEffectDurationPolicy::Instant || bIsEffectExecutionPeriodic)
	{
		// 如果效果的持续时间策略为Instant，或者效果在持续期间周期性执行
		for (auto Modifier : Modifiers)
		{
			AbilitySystemManager->ApplyModifierToAttributeInstant(Modifier.AttributeType,
				Modifier.ModOperator, this, Modifier.ModValue);
		}
	}
	else
	{
		// 如果效果在持续期间不周期性执行
		for (auto Modifier : Modifiers)
		{
			AbilitySystemManager->ApplyModifierToAttribute(Modifier.AttributeType, Modifier.ModOperator,
				this, Modifier.ModValue, StackCount);
		}
	}	

	ReceiveExecuteEffect();
}

void UFireflyEffect::ExecuteEffectExpiration()
{
	/** 清理持续时间计时器 */
	GetWorld()->GetTimerManager().ClearTimer(DurationTimer);

	/** 如果该效果不会堆叠，或者堆叠到期策略为 ClearEntireStack，结束效果 */
	if (StackingExpirationPolicy == EFireflyEffectDurationPolicyOnStackingExpired::ClearEntireStack
		|| StackingPolicy == EFireflyEffectStackingPolicy::None)
	{
		ReceiveExecuteEffectExpiration();
		RemoveEffect();		

		return;
	}

	/** 如果堆叠到期策略为 RemoveSingleStackAndRefreshDuration，减少一个堆叠数 */
	if (StackingExpirationPolicy == EFireflyEffectDurationPolicyOnStackingExpired::RemoveSingleStackAndRefreshDuration)
	{
		/** 如果堆叠数被清到0，结束效果 */
		if (ReduceEffectStack(1))
		{
			ReceiveExecuteEffectExpiration();
			RemoveEffect();

			return;
		}
	}

	/** 尝试刷新持续时间 */
	TryExecuteOrRefreshDuration();

	ReceiveExecuteEffectExpiration();	
}

void UFireflyEffect::RemoveEffect()
{
	UFireflyAbilitySystemComponent* AbilitySystemManager = GetOwnerManager();
	if (!IsValid(AbilitySystemManager))
	{
		return;
	}

	/** 清理该效果携带的所有属性修改器 */
	for (auto Modifier : Modifiers)
	{
		AbilitySystemManager->RemoveModifierFromAttribute(Modifier.AttributeType, Modifier.ModOperator, this, Modifier.ModValue);
	}

	/** 堆叠数重置为0 */
	if (StackingPolicy != EFireflyEffectStackingPolicy::None && StackCount > 0)
	{
		ReduceEffectStack(StackCount);
	}

	ReceiveRemoveEffect();

	MarkAsGarbage();
}
