// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FireflyAttribute.generated.h"

// CurrentValue = NewestOuterOverrideMod || (((BaseValue || NewestInnerOverrideMod) + PlusMods - MinusMods) * (1 + MultiplyMods)) / (TotalDivideMod == 0.f ? 1.f : TotalDivideMod))

enum class EFireflyAttributeModOperator : uint8;
enum EFireflyAttributeType;
class UFireflyAttributeManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAttributeValueChangeDelegate, TEnumAsByte<EFireflyAttributeType>, AttributeType, const float, OldValue, const float, Newvalue);

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

	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;

	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

#pragma endregion


#pragma region Basic

protected:
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
	FORCEINLINE UFireflyAttributeManagerComponent* GetOwnerManager() const;

	/** 初始化属性 */
	void Initialize(float InitValue);

public:
	/** 属性的当前值更新时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Attribute")
	FAttributeValueChangeDelegate OnAttributeValueChanged;

	/** 属性的基础值更新时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Attribute")
	FAttributeValueChangeDelegate OnAttributeBaseValueChanged;

protected:
	/** 属性名 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Attribute")
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

	/** 更新属性的当前值 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Attribute")
	void UpdateBaseValue(EFireflyAttributeModOperator ModOperator, float ModValue);

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
