﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Attribute/FireflyAttributeManagerComponent.h"

#include "FireflyAbilitySystemTypes.h"
#include "Attribute/FireflyAttribute.h"

// Sets default values for this component's properties
UFireflyAttributeManagerComponent::UFireflyAttributeManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UFireflyAttributeManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UFireflyAttributeManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UFireflyAttribute* UFireflyAttributeManagerComponent::GetAttributeByType(EFireflyAttributeType AttributeType) const
{
	UFireflyAttribute* OutAttribute = nullptr;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->AttributeType == AttributeType)
		{
			OutAttribute = Attribute;
			break;
		}
	}

	return OutAttribute;
}

FString UFireflyAttributeManagerComponent::GetAttributeTypeName(EFireflyAttributeType AttributeType) const
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFireflyAttributeType"), true);
	if (!EnumPtr) return FString("Invalid");

	return EnumPtr != nullptr ? EnumPtr->GetDisplayNameTextByValue(AttributeType).ToString() : FString("Invalid");
}

float UFireflyAttributeManagerComponent::GetAttributeValue(EFireflyAttributeType AttributeType) const
{
	float OutValue = 0.f;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->AttributeType == AttributeType)
		{
			OutValue = Attribute->GetCurrentValue();
			break;
		}
	}

	return OutValue;
}

float UFireflyAttributeManagerComponent::GetAttributeBaseValue(EFireflyAttributeType AttributeType) const
{
	float OutBaseValue = 0.f;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->AttributeType == AttributeType)
		{
			OutBaseValue = Attribute->GetBaseValueToUse();
			break;
		}
	}

	return OutBaseValue;
}

void UFireflyAttributeManagerComponent::ConstructAttributeByClass(TSubclassOf<UFireflyAttribute> AttributeToConstruct, EFireflyAttributeType AttributeType)
{
	FString NewAttributeName = *GetAttributeTypeName(AttributeType) + FString("_") + (TEXT("%s"), GetOwner()->GetName());
	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, AttributeToConstruct, *NewAttributeName);
	NewAttribute->AttributeType = AttributeType;
	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAttributeManagerComponent::ConstructAttributeByType(EFireflyAttributeType AttributeType)
{
	FString NewAttributeName = *GetAttributeTypeName(AttributeType) + FString("_") + (TEXT("%s"), GetOwner()->GetName());
	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, *NewAttributeName);
	NewAttribute->AttributeType = AttributeType;
	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAttributeManagerComponent::InitializeAttribute(EFireflyAttributeType AttributeType, float NewInitValue)
{
	UFireflyAttribute* AttributeToInit = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToInit))
	{
		return;
	}

	AttributeToInit->BaseValue = NewInitValue;
	AttributeToInit->CurrentValue = NewInitValue;
}

void UFireflyAttributeManagerComponent::ApplyModifierToAttribute(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
		{
			break;
		}
	case EFireflyAttributeModOperator::Plus:
		{
			AttributeToMod->PlusMods.Push(FFireflyAttributeModifier(ModSource, ModValue));
			break;
		}
	case EFireflyAttributeModOperator::Minus:
		{
			AttributeToMod->MinusMods.Push(FFireflyAttributeModifier(ModSource, ModValue));
			break;
		}
	case EFireflyAttributeModOperator::Multiply:
		{
			AttributeToMod->MultiplyMods.Push(FFireflyAttributeModifier(ModSource, ModValue));
			break;
		}
	case EFireflyAttributeModOperator::Divide:
		{
			AttributeToMod->DivideMods.Push(FFireflyAttributeModifier(ModSource, ModValue));
			break;
		}
	case EFireflyAttributeModOperator::InnerOverride:
		{
			AttributeToMod->InnerOverrideMods.Push(FFireflyAttributeModifier(ModSource, ModValue));
			break;
		}
	case EFireflyAttributeModOperator::OuterOverride:
		{
			AttributeToMod->OuterOverrideMods.Push(FFireflyAttributeModifier(ModSource, ModValue));
			break;
		}	
	}

	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::ApplyModifierToAttributeBase(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue)
{
	UFireflyAttribute* AttributeToModBase = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToModBase))
	{
		return;
	}

	AttributeToModBase->UpdateBaseValueToUse(ModOperator, ModValue);
}

void UFireflyAttributeManagerComponent::RemoveModifierFromAttribute(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	FFireflyAttributeModifier ModifierToRemove = FFireflyAttributeModifier(ModSource);

#define FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(ModOperatorName) \
	if (AttributeToMod->##ModOperatorName##Mods.Contains(ModifierToRemove)) \
	{ \
		AttributeToMod->##ModOperatorName##Mods.RemoveSingle(ModifierToRemove); \
	}

	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
		{
			break;
		}
	case EFireflyAttributeModOperator::Plus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Plus);
			break;
		}
	case EFireflyAttributeModOperator::Minus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Minus);
			break;
		}
	case EFireflyAttributeModOperator::Multiply:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Multiply);
			break;
		}
	case EFireflyAttributeModOperator::Divide:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Divide);
			break;
		}
	case EFireflyAttributeModOperator::InnerOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(InnerOverride);
			break;
		}
	case EFireflyAttributeModOperator::OuterOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(OuterOverride);
			break;
		}
	}

	AttributeToMod->UpdateCurrentValue();
}