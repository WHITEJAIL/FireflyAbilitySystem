// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAttributeManagerComponent.h"

#include "FireflyAttribute.h"

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

void UFireflyAttributeManagerComponent::OnRegister()
{
	Super::OnRegister();

	/** 将所有的用户自定义属性存储到属性集数组中 */
	/*for (TFieldIterator<FProperty> It(GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FProperty* Property = *It;
		FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property);
		if (ObjectProperty && ObjectProperty->GetName() != TEXT("AttributeContainer"))
		{
			UFireflyAttribute* Attribute = ObjectProperty->ContainerPtrToValuePtr<UFireflyAttribute>(this);
			if (Attribute)
			{
				AttributeContainer.Emplace(Attribute);
			}
		}
	}*/
}

UFireflyAttribute* UFireflyAttributeManagerComponent::GetAttributeByType(FGameplayTag AttributeType) const
{
	UFireflyAttribute* OutAttribute = nullptr;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->GetAttributeName().MatchesTag(AttributeType))
		{
			OutAttribute = Attribute;
			break;
		}
	}

	return OutAttribute;
}

// Called every frame
void UFireflyAttributeManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UFireflyAttributeManagerComponent::AddNewAttribute(FGameplayTag AttributeType, float InitValue)
{
	if (!AttributeType.IsValid())
	{
		return;
	}

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, AttributeType.GetTagName());
	if (!IsValid(NewAttribute))
	{
		return;
	}

	NewAttribute->AttributeName = AttributeType;
	NewAttribute->Initialize(InitValue);
	AttributeContainer.Emplace(NewAttribute);
}

float UFireflyAttributeManagerComponent::GetAttributeValue(FGameplayTag AttributeType) const
{
	float OutValue = 0.f;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->GetAttributeName().MatchesTag(AttributeType))
		{
			OutValue = Attribute->GetCurrentValue();
			break;
		}
	}

	return OutValue;
}

float UFireflyAttributeManagerComponent::GetAttributeBaseValue(FGameplayTag AttributeType) const
{
	float OutBaseValue = 0.f;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->GetAttributeName().MatchesTag(AttributeType))
		{
			OutBaseValue = Attribute->GetBaseValue();
			break;
		}
	}

	return OutBaseValue;
}

void UFireflyAttributeManagerComponent::ApplyPlusModifierToAttribute(FGameplayTag AttributeType, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->PlusMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemovePlusModifierFromAttribute(FGameplayTag AttributeType, float ModifierToRemove)
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

void UFireflyAttributeManagerComponent::ApplyMinusModifierToAttribute(FGameplayTag AttributeType, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->MinusMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemoveMinusModifierFromAttribute(FGameplayTag AttributeType,
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

void UFireflyAttributeManagerComponent::ApplyMultiplyModifierToAttribute(FGameplayTag AttributeType, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->MultiplyMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemoveMultiplyModifierFromAttribute(FGameplayTag AttributeType,
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

void UFireflyAttributeManagerComponent::ApplyDivideModifierToAttribute(FGameplayTag AttributeType, float ModValue)
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->DivideMods.Push(ModValue);
	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAttributeManagerComponent::RemoveDivideModifierFromAttribute(FGameplayTag AttributeType,
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

void UFireflyAttributeManagerComponent::ApplyInnerOverrideModifierToAttribute(FGameplayTag AttributeType,
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

void UFireflyAttributeManagerComponent::RemoveInnerOverrideModifierFromAttribute(FGameplayTag AttributeType,
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

void UFireflyAttributeManagerComponent::ApplyOuterOverrideModifierToAttribute(FGameplayTag AttributeType,
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

void UFireflyAttributeManagerComponent::RemoveOuterOverrideModifierFromAttribute(FGameplayTag AttributeType,
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
