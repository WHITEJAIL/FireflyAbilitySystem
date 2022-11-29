﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAttribute.h"

#include "FireflyAbilitySystemTypes.h"
#include "FireflyAbilitySystemComponent.h"

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

bool UFireflyAttribute::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
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

int32 UFireflyAttribute::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	return (GetOuter() ? GetOuter()->GetFunctionCallspace(Function, Stack) : FunctionCallspace::Local);
}

float UFireflyAttribute::GetCurrentValue() const
{
	return CurrentValue;
}

float UFireflyAttribute::GetBaseValueToUse() const
{
	return InnerOverrideMods.IsValidIndex(0) ? InnerOverrideMods[0].ModValue : BaseValue;
}

AActor* UFireflyAttribute::GetOwnerActor() const
{
	if (!IsValid(GetOwnerManager()))
	{
		return nullptr;
	}

	return GetOwnerManager()->GetOwner();
}

UFireflyAbilitySystemComponent* UFireflyAttribute::GetOwnerManager() const
{
	if (!IsValid(GetOuter()))
	{
		return nullptr;
	}

	return Cast<UFireflyAbilitySystemComponent>(GetOuter());
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
	const float OldValue = CurrentValue;

	if (OuterOverrideMods.IsValidIndex(0))
	{
		CurrentValue = OuterOverrideMods[0].ModValue;
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

void UFireflyAttribute::UpdateBaseValue_Implementation(EFireflyAttributeModOperator ModOperator, float ModValue)
{
	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
	{
		break;
	}
	case EFireflyAttributeModOperator::Plus:
	{
		BaseValue += ModValue;
		break;
	}
	case EFireflyAttributeModOperator::Minus:
	{
		BaseValue -= ModValue;
		break;
	}
	case EFireflyAttributeModOperator::Multiply:
	{
		BaseValue *= ModValue;
		break;
	}
	case EFireflyAttributeModOperator::Divide:
	{
		BaseValue /= (ModValue == 0.f ? 1.f : ModValue);
		break;
	}
	case EFireflyAttributeModOperator::InnerOverride:
	case EFireflyAttributeModOperator::OuterOverride:
	{
		BaseValue = ModValue;
		break;
	}
	}
}

float UFireflyAttribute::GetTotalPlusModifier() const
{
	float TotalPlusMod = 0.f;
	for (const auto PlusMod : PlusMods)
	{
		TotalPlusMod += PlusMod.ModValue * PlusMod.StackCount;
	}

	return TotalPlusMod;
}

float UFireflyAttribute::GetTotalMinusModifier() const
{
	float TotalMinusMod = 0.f;
	for (const auto MinusMod : MinusMods)
	{
		TotalMinusMod += MinusMod.ModValue * MinusMod.StackCount;
	}

	return TotalMinusMod;
}

float UFireflyAttribute::GetTotalMultiplyModifier() const
{
	float TotalMultiplyMod = 0.f;
	for (const auto MultiplyMod : MultiplyMods)
	{
		TotalMultiplyMod += MultiplyMod.ModValue * MultiplyMod.StackCount;
	}

	return TotalMultiplyMod;
}

float UFireflyAttribute::GetTotalDivideModifier() const
{
	float TotalDivideMod = 0.f;
	for (const auto DivideMod : DivideMods)
	{
		TotalDivideMod += DivideMod.ModValue * DivideMod.StackCount;
	}
	TotalDivideMod = TotalDivideMod == 0.f ? 1.f : TotalDivideMod;

	return TotalDivideMod;
}

bool UFireflyAttribute::GetNewestOuterOverrideModifier(float& NewestValue) const
{
	if (OuterOverrideMods.IsValidIndex(0))
	{
		NewestValue = OuterOverrideMods[0].ModValue;
		return true;
	}

	return false;
}