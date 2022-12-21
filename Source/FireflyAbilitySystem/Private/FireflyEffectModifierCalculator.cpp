// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyEffectModifierCalculator.h"

UFireflyEffectModifierCalculator::UFireflyEffectModifierCalculator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* UFireflyEffectModifierCalculator::GetWorld() const
{
	if (!IsValid(GetOuter()))
	{
		return nullptr;
	}

	return GetOuter()->GetWorld();
}

float UFireflyEffectModifierCalculator::CalculateModifierValue_Implementation(UFireflyEffect* EffectInstance)
{
	return 0.f;
}
