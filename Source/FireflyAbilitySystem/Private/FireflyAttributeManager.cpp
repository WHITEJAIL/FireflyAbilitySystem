// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAttributeManager.h"

UFireflyAttributeManager::UFireflyAttributeManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float UFireflyAttributeManager::GetAttributeValue(FGameplayTag InAttribute) const
{
	float OutValue = 0.f;
	for (auto Attribute : Attributes)
	{
		if (Attribute.GetAttributeName() == InAttribute)
		{
			OutValue = Attribute.GetCurrentValue();
			break;
		}
	}

	return OutValue;
}

float UFireflyAttributeManager::GetAttributeBaseValue(FGameplayTag InAttribute) const
{
	float OutBaseValue = 0.f;
	for (auto Attribute : Attributes)
	{
		if (Attribute.GetAttributeName() == InAttribute)
		{
			OutBaseValue = Attribute.GetBaseValue();
			break;
		}
	}

	return OutBaseValue;
}
