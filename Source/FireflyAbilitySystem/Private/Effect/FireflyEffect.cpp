﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/FireflyEffect.h"

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

void UFireflyEffect::ApplyEffect()
{
	ReceiveApplyEffect();
}

void UFireflyEffect::ExecuteEffect()
{
	ReceiveExecuteEffect();
}

void UFireflyEffect::AddEffectStack(int32 StackCountToAdd)
{
	ReceiveAddEffectStack(StackCountToAdd);
}

void UFireflyEffect::ReduceEffectStack(int32 StackCountToReduce)
{
	ReceiveReduceEffectStack(StackCountToReduce);
}

void UFireflyEffect::RemoveEffect()
{
	ReceiveRemoveEffect();
}
