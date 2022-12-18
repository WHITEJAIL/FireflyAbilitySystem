// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyEffectModifierCalculator.h"

#include "FireflyEffect.h"

UFireflyEffectModifierCalculator::UFireflyEffectModifierCalculator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* UFireflyEffectModifierCalculator::GetWorld() const
{
	if (!IsValid(GetEffect()))
	{
		return nullptr;
	}

	return GetEffect()->GetWorld();
}

UFireflyEffect* UFireflyEffectModifierCalculator::GetEffect() const
{
	return Cast<UFireflyEffect>(GetOuter());
}

float UFireflyEffectModifierCalculator::CalculateModifierValue_Implementation(UFireflyEffect* EffectInstance,
                                                                              EFireflyAttributeType AttributeType, float OldModValue)
{
	return 0.f;
}
