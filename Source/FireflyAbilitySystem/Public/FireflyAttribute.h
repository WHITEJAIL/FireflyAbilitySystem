// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FireflyAbilitySystemTypes.h"
#include "FireflyAttribute.generated.h"

// CurrentValue = NewestOuterOverrideMod || (((BaseValue || NewestInnerOverrideMod) + PlusMods - MinusMods) * (1 + MultiplyMods)) / (TotalDivideMod == 0.f ? 1.f : TotalDivideMod))

class UFireflyAbilitySystemComponent;

/** 属性修改器 */
USTRUCT()
struct FFireflyAttributeModifier
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	UObject* ModSource = nullptr;

	UPROPERTY()
	float ModValue = 0.f;

	UPROPERTY()
	int32 StackCount = 0;

	FFireflyAttributeModifier() {}

	FFireflyAttributeModifier(UObject* InSource, float InValue) : ModSource(InSource), ModValue(InValue) {}

	FFireflyAttributeModifier(UObject* InSource, float InValue, int32 InStack) : ModSource(InSource), ModValue(InValue), StackCount(InStack) {}

	FORCEINLINE bool operator==(const FFireflyAttributeModifier& Other) const
	{
		return ModSource == Other.ModSource && ModValue == Other.ModValue;
	}
};

/** 属性 */
UCLASS(Blueprintable)
class FIREFLYABILITYSYSTEM_API UFireflyAttribute : public UObject
{
	GENERATED_UCLASS_BODY()

#pragma region Override

public:
	virtual UWorld* GetWorld() const override;

	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;

	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

#pragma endregion


#pragma region Basic

protected:
	/** 初始化属性实例，当属性被构建并添加到技能管理器时触发 */
	UFUNCTION()
	virtual void InitializeAttributeInstance();

	/** 蓝图端实现的初始化属性实例，当属性被构建并添加到技能管理器时触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Attribute", Meta = (DisplayName = "InitializeAttributeInstance"))
	void ReceiveInitializeAttributeInstance();

	/** 获取属性的类型 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE TEnumAsByte<EFireflyAttributeType> GetAttributeType() const;

	/** 获取属性的范围最大值属性类型，基于另一个属性的当前值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE TEnumAsByte<EFireflyAttributeType> GetRangeMaxValueType() const;

	/** 获取属性的当前值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetCurrentValue() const;

	/** 获取属性的基础值或内部覆盖修改器的最新值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetBaseValueToUse() const;	

	/** 获取属性所属的管理器的拥有者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE AActor* GetOwnerActor() const;

	/** 获取属性所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE UFireflyAbilitySystemComponent* GetOwnerManager() const;

	/** 初始化属性 */
	void InitializeAttributeValue(float InitValue);

protected:
	/** 属性名 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Attribute")
	TEnumAsByte<EFireflyAttributeType> AttributeType = AttributeType_Default;

	/** 属性的基础值 */
	UPROPERTY()
	float BaseValue = 0.f;

	/** 属性的当前值 */
	UPROPERTY()
	float CurrentValue = 0.f;

	/** 属性不需要夹值，但属性不能小于给定的值 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Attribute")
	bool bAttributeMustNotLessThanSelection = true;

	/** bAttributeMustMoreNotLessThanSelection为true时，属性不会小于该值 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Attribute", Meta = (EditCondition = "bAttributeMustNotLessThanSelection"))
	float LessBaseValue = 0.f;

	/** 属性是否需要夹值 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Attribute")
	bool bAttributeHasRange = false;

	/** 属性的范围最小值 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Attribute", Meta = (EditCondition = "bAttributeHasRange"))
	float RangeMinValue = 0.f;

	/** 属性的范围最大值 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Attribute", Meta = (EditCondition = "bAttributeHasRange"))
	float RangeMaxValue = 0.f;

	/** 属性的范围最大值属性类型，基于另一个属性的当前值 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Attribute", Meta = (EditCondition = "bAttributeHasRange"))
	TEnumAsByte<EFireflyAttributeType> RangeMaxValueType = AttributeType_Default;

	friend UFireflyAbilitySystemComponent;

#pragma endregion


#pragma region Modifier

protected:
	/** 更新属性的当前值 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Attribute")
	void UpdateCurrentValue();

	/** 更新属性的当前值 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Attribute")
	void UpdateBaseValue(EFireflyAttributeModOperator ModOperator, float ModValue);

	/** 检测某个值是否在该属性的夹值范围中 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	bool IsValueInAttributeRange(float InValue);

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
	
	/** 属性的所有加法修改器 */
	UPROPERTY()
	TArray<FFireflyAttributeModifier> PlusMods = TArray<FFireflyAttributeModifier>{};

	/** 属性的所有减法修改器 */
	UPROPERTY()
	TArray<FFireflyAttributeModifier> MinusMods = TArray<FFireflyAttributeModifier>{};

	/** 属性的所有乘法修改器 */
	UPROPERTY()
	TArray<FFireflyAttributeModifier> MultiplyMods = TArray<FFireflyAttributeModifier>{};

	/** 属性的所有除法修改器 */
	UPROPERTY()
	TArray<FFireflyAttributeModifier> DivideMods = TArray<FFireflyAttributeModifier>{};

	/** 属性的所有内部覆盖修改器 */
	UPROPERTY()
	TArray<FFireflyAttributeModifier> InnerOverrideMods = TArray<FFireflyAttributeModifier>{};

	/** 属性的所有外部覆盖修改器 */
	UPROPERTY()
	TArray<FFireflyAttributeModifier> OuterOverrideMods = TArray<FFireflyAttributeModifier>{};

#pragma endregion
};
