// Fill out your copyright notice in the Description page of Project Settings.


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

void UFireflyAttributeManagerComponent::ApplyPlusModifier(EFireflyAttributeType AttributeType, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->PlusMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemovePlusModifier(EFireflyAttributeType AttributeType, float ModifierToRemove)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	if (!AttributeToMod->PlusMods.Contains(ModifierToRemove))
	{
		return;
	}

	AttributeToMod->PlusMods.RemoveSingle(ModifierToRemove);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::ApplyMinusModifier(EFireflyAttributeType AttributeType, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->MinusMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemoveMinusModifier(EFireflyAttributeType AttributeType,
	float ModifierToRemove)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	if (!AttributeToMod->MinusMods.Contains(ModifierToRemove))
	{
		return;
	}

	AttributeToMod->MinusMods.RemoveSingle(ModifierToRemove);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::ApplyMultiplyModifier(EFireflyAttributeType AttributeType, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->MultiplyMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemoveMultiplyModifier(EFireflyAttributeType AttributeType,
	float ModifierToRemove)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	if (!AttributeToMod->MultiplyMods.Contains(ModifierToRemove))
	{
		return;
	}

	AttributeToMod->MultiplyMods.RemoveSingle(ModifierToRemove);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::ApplyDivideModifier(EFireflyAttributeType AttributeType, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->DivideMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemoveDivideModifier(EFireflyAttributeType AttributeType,
	float ModifierToRemove)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	if (!AttributeToMod->DivideMods.Contains(ModifierToRemove))
	{
		return;
	}

	AttributeToMod->DivideMods.RemoveSingle(ModifierToRemove);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::ApplyInnerOverrideModifier(EFireflyAttributeType AttributeType,
	float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->InnerOverrideMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemoveInnerOverrideModifier(EFireflyAttributeType AttributeType,
	float ModifierToRemove)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	if (!AttributeToMod->InnerOverrideMods.Contains(ModifierToRemove))
	{
		return;
	}

	AttributeToMod->InnerOverrideMods.RemoveSingle(ModifierToRemove);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::ApplyOuterOverrideModifier(EFireflyAttributeType AttributeType,
	float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->OuterOverrideMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemoveOuterOverrideModifier(EFireflyAttributeType AttributeType,
	float ModifierToRemove)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	if (!AttributeToMod->OuterOverrideMods.Contains(ModifierToRemove))
	{
		return;
	}

	AttributeToMod->OuterOverrideMods.RemoveSingle(ModifierToRemove);
	AttributeToMod->UpdateCurrentValue();
}
