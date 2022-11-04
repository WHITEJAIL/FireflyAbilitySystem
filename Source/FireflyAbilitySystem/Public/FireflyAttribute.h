// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "FireflyAttribute.generated.h"

// CurrentValue = NewestOuterOverrideMod || (((BaseValue || NewestInnerOverrideMod) + PlusMods - MinusMods) * (1 + MultiplyMods)) / (TotalDivideMod == 0.f ? 1.f : TotalDivideMod))

class UFireflyAttributeManagerComponent;

/** 属性 */
UCLASS()
class FIREFLYABILITYSYSTEM_API UFireflyAttribute : public UObject
{
	GENERATED_UCLASS_BODY()

#pragma region Basic

public:
	/** 获取属性名 */
	FORCEINLINE FGameplayTag GetAttributeName() const { return AttributeName; }

	/** 获取属性的基础值 */
	FORCEINLINE float GetBaseValue() const { return BaseValue; }

	/** 获取属性的固定值 */
	FORCEINLINE float GetCurrentValue() const { return CurrentValue; }

	/** 初始化属性 */
	void Initialize(float InitValue);

protected:
	/** 属性名 */
	UPROPERTY()
	FGameplayTag AttributeName = FGameplayTag::EmptyTag;

	/** 属性的基础值 */
	UPROPERTY()
	float BaseValue = 0.f;

	/** 属性的当前值 */
	UPROPERTY()
	float CurrentValue = 0.f;

	friend UFireflyAttributeManagerComponent;

#pragma endregion


#pragma region Modifier

protected:
	/** 更新属性的当前值 */
	void UpdateCurrentValue();

	/** 属性的所有加法修改器 */
	UPROPERTY()
	TArray<float> PlusMods = TArray<float>{};

	/** 属性的所有减法修改器 */
	UPROPERTY()
	TArray<float> MinusMods = TArray<float>{};

	/** 属性的所有乘法修改器 */
	UPROPERTY()
	TArray<float> MultiplyMods = TArray<float>{};

	/** 属性的所有除法修改器 */
	UPROPERTY()
	TArray<float> DivideMods = TArray<float>{};

	/** 属性的所有内部覆盖修改器 */
	UPROPERTY()
	TArray<float> InnerOverrideMods = TArray<float>{};

	/** 属性的所有外部覆盖修改器 */
	UPROPERTY()
	TArray<float> OuterOverrideMods = TArray<float>{};

#pragma endregion
};
