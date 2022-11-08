// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FireflyAttribute.generated.h"

// CurrentValue = NewestOuterOverrideMod || (((BaseValue || NewestInnerOverrideMod) + PlusMods - MinusMods) * (1 + MultiplyMods)) / (TotalDivideMod == 0.f ? 1.f : TotalDivideMod))

enum EFireflyAttributeType;
class UFireflyAttributeManagerComponent;

/** 属性 */
UCLASS()
class FIREFLYABILITYSYSTEM_API UFireflyAttribute : public UObject
{
	GENERATED_UCLASS_BODY()

#pragma region Basic

protected:
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetCurrentValue() const;

	/** 获取属性的内部覆盖修改器的最新值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetBaseValueToUse() const;

	/** 获取属性的加法修改器的合值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetTotalPlusModifier() const;

	/** 获取属性的减法修改器的合值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetTotalMinusModifier() const;

	/** 获取属性的乘法修改器的合值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetTotalMultiplyModifier() const;

	/** 获取属性的除法修改器的合值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetTotalDivideModifier() const;	

	/** 获取属性的内部覆盖修改器的最新值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE bool GetNewestOuterOverrideModifier(float& NewestValue) const;

	/** 获取属性所属的管理器的拥有者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE AActor* GetOwnerActor() const;

	/** 获取属性所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE UFireflyAttributeManagerComponent* GetOwnerManager() const;

	/** 初始化属性 */
	void Initialize(float InitValue);

protected:
	/** 属性名 */
	UPROPERTY(EditDefaultsOnly, Category = Attribute)
	TEnumAsByte<EFireflyAttributeType> AttributeType;

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
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Attribute")
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
