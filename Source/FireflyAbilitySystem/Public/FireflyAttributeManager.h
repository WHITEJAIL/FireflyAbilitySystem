// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FireflyAttributeManager.generated.h"

USTRUCT()
struct FIREFLYABILITYSYSTEM_API FFireflyAttributeModifier
{
	GENERATED_BODY()

public:
	FFireflyAttributeModifier() {}

	FFireflyAttributeModifier(UObject* InModSource, float InModValue) : ModSource(InModSource), ModValue(InModValue) {}

	FORCEINLINE bool operator==(const FFireflyAttributeModifier& Other) const
	{
		return ModSource == Other.ModSource
			&& ModValue == Other.ModValue;
	}

protected:
	UPROPERTY()
	UObject* ModSource = nullptr;

	UPROPERTY()
	float ModValue = 0.f;
};

USTRUCT()
struct FIREFLYABILITYSYSTEM_API FFireflyAttribute
{
	GENERATED_BODY()

public:
	FFireflyAttribute() {}

	FFireflyAttribute(FGameplayTag InAttributeName) : AttributeName(InAttributeName) {}

	FFireflyAttribute(FGameplayTag InAttributeName, float InAttributeValue)
	{
		AttributeName = InAttributeName;
		BaseValue = InAttributeValue;
		CurrentValue = InAttributeValue;
	}

	FORCEINLINE bool operator==(const FFireflyAttribute& Other) const
	{
		return AttributeName == Other.AttributeName
			&& BaseValue == Other.BaseValue
			&& CurrentValue == Other.CurrentValue
			&& AdditiveMods == Other.AdditiveMods
			&& MultiplyMods == Other.MultiplyMods
			&& DivideMods == Other.DivideMods
			&& OverrideMods == Other.OverrideMods;
	}

	FORCEINLINE FGameplayTag GetAttributeName() const { return AttributeName; }

	FORCEINLINE float GetBaseValue() const { return BaseValue; }

	FORCEINLINE float GetCurrentValue() const { return CurrentValue; }

protected:
	UPROPERTY()
	FGameplayTag AttributeName = FGameplayTag::EmptyTag;

	UPROPERTY()
	float BaseValue = 0.f;

	UPROPERTY()
	float CurrentValue = 0.f;

	UPROPERTY()
	TArray<FFireflyAttributeModifier> AdditiveMods = TArray<FFireflyAttributeModifier>{};

	UPROPERTY()
	TArray<FFireflyAttributeModifier> MultiplyMods = TArray<FFireflyAttributeModifier>{};

	UPROPERTY()
	TArray<FFireflyAttributeModifier> DivideMods = TArray<FFireflyAttributeModifier>{};

	UPROPERTY()
	TArray<FFireflyAttributeModifier> OverrideMods = TArray<FFireflyAttributeModifier>{};
};

/**
 * 
 */
UCLASS(DefaultToInstanced, NotBlueprintType, Blueprintable)
class FIREFLYABILITYSYSTEM_API UFireflyAttributeManager : public UObject
{
	GENERATED_BODY()

public:
	UFireflyAttributeManager(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE float GetAttributeValue(FGameplayTag InAttribute) const;

	FORCEINLINE float GetAttributeBaseValue(FGameplayTag InAttribute) const;

protected:
	UPROPERTY()
	TArray<FFireflyAttribute> Attributes;
};
