﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Attribute/FireflyAttribute.h"

#include "FireflyAbilitySystemTypes.h"
#include "Attribute/FireflyAttributeManagerComponent.h"

UFireflyAttribute::UFireflyAttribute(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AttributeType = EFireflyAttributeType::AttributeType_Default;
}

UWorld* UFireflyAttribute::GetWorld() const
{
	if (AActor* OwnerActor = GetOwnerActor())
	{
		return OwnerActor->GetWorld();
	}

	return nullptr;
}

float UFireflyAttribute::GetCurrentValue() const
{
	return CurrentValue;
}

float UFireflyAttribute::GetBaseValueToUse() const
{
	return InnerOverrideMods.IsValidIndex(0) ? InnerOverrideMods[0] : BaseValue;
}

AActor* UFireflyAttribute::GetOwnerActor() const
{
	if (!IsValid(GetOwnerManager()))
	{
		return nullptr;
	}

	return GetOwnerManager()->GetOwner();
}

UFireflyAttributeManagerComponent* UFireflyAttribute::GetOwnerManager() const
{
	if (!IsValid(GetOuter()))
	{
		return nullptr;
	}

	return Cast<UFireflyAttributeManagerComponent>(GetOuter());
}

void UFireflyAttribute::Initialize(float InitValue)
{
	float OldValue = 0.f;

	OldValue = BaseValue;
	BaseValue = InitValue;
	OnAttributeValueChanged.Broadcast(AttributeType, OldValue, CurrentValue);

	OldValue = CurrentValue;
	CurrentValue = InitValue;
	OnAttributeBaseValueChanged.Broadcast(AttributeType, OldValue, CurrentValue);
}

void UFireflyAttribute::UpdateCurrentValue_Implementation()
{
	float ResultValue = 0.f;
	const float OldValue = CurrentValue;

	if (GetNewestOuterOverrideModifier(ResultValue))
	{
		CurrentValue = ResultValue;
		OnAttributeValueChanged.Broadcast(AttributeType, OldValue, CurrentValue);
		return;
	}

	float TotalPlusMod = GetTotalPlusModifier();
	float TotalMinusMod = GetTotalMinusModifier();
	float TotalMultiplyMod = GetTotalMultiplyModifier();
	float TotalDivideMod = GetTotalDivideModifier();

	float BaseValueToUse = GetBaseValueToUse();

	CurrentValue = (BaseValueToUse + TotalPlusMod - TotalMinusMod) * (1.f + TotalMultiplyMod) / TotalDivideMod;
	OnAttributeValueChanged.Broadcast(AttributeType, OldValue, CurrentValue);
}

float UFireflyAttribute::GetTotalPlusModifier() const
{
	float TotalPlusMod = 0.f;
	for (const float PlusMod : PlusMods)
	{
		TotalPlusMod += PlusMod;
	}

	return TotalPlusMod;
}

float UFireflyAttribute::GetTotalMinusModifier() const
{
	float TotalMinusMod = 0.f;
	for (const float MinusMod : MinusMods)
	{
		TotalMinusMod += MinusMod;
	}

	return TotalMinusMod;
}

float UFireflyAttribute::GetTotalMultiplyModifier() const
{
	float TotalMultiplyMod = 0.f;
	for (const float MultiplyMod : MultiplyMods)
	{
		TotalMultiplyMod += MultiplyMod;
	}

	return TotalMultiplyMod;
}

float UFireflyAttribute::GetTotalDivideModifier() const
{
	float TotalDivideMod = 0.f;
	for (const float DivideMod : DivideMods)
	{
		TotalDivideMod += DivideMod;
	}
	TotalDivideMod = TotalDivideMod == 0.f ? 1.f : TotalDivideMod;

	return TotalDivideMod;
}

bool UFireflyAttribute::GetNewestOuterOverrideModifier(float& NewestValue) const
{
	if (OuterOverrideMods.IsValidIndex(0))
	{
		NewestValue = OuterOverrideMods[0];
		return true;
	}

	return false;
}