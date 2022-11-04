// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAttribute.h"

UFireflyAttribute::UFireflyAttribute(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFireflyAttribute::Initialize(float InitValue)
{
	BaseValue = InitValue;
	CurrentValue = InitValue;
}

void UFireflyAttribute::UpdateCurrentValue()
{
	float TotalPlusMod = 0.f;
	for(const float PlusMod : PlusMods)
	{
		TotalPlusMod += PlusMod;
	}

	float TotalMinusMod = 0.f;
	for (const float MinusMod : MinusMods)
	{
		TotalMinusMod += MinusMod;
	}

	float TotalMultiplyMod = 0.f;
	for (const float MultiplyMod : MultiplyMods)
	{
		TotalMultiplyMod += MultiplyMod;
	}

	float TotalDivideMod = 0.f;
	for (const float DivideMod : DivideMods)
	{
		TotalDivideMod += DivideMod;
	}
	TotalDivideMod = TotalDivideMod == 0.f ? 1.f : TotalDivideMod;

	if (OuterOverrideMods.IsValidIndex(0))
	{
		CurrentValue = OuterOverrideMods[0];
		return;
	}

	float TmpBaseValue = InnerOverrideMods.IsValidIndex(0) ? InnerOverrideMods[0] : BaseValue;

	CurrentValue = (TmpBaseValue + TotalPlusMod - TotalMinusMod) * (1 + TotalMultiplyMod) / TotalDivideMod;
}