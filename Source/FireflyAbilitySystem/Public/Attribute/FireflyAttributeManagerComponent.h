// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FireflyAttributeManagerComponent.generated.h"

// CurrentValue = NewestOuterOverrideMod || (((BaseValue || NewestInnerOverrideMod) + PlusMods - MinusMods) * (1 + MultiplyMods)) / (TotalDivideMod == 0.f ? 1.f : TotalDivideMod))

enum class EFireflyAttributeModOperator : uint8;
enum EFireflyAttributeType;
class UFireflyAttribute;

/** 属性管理器组件 */
UCLASS( ClassGroup = (FireflyAbilitySystem), meta = (BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyAttributeManagerComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

#pragma region Override

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion


#pragma region Basic

public:
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	UFireflyAttribute* GetAttributeByType(EFireflyAttributeType AttributeType) const;

	FString GetAttributeTypeName(EFireflyAttributeType AttributeType) const;

protected:
	/** 属性容器 */
	UPROPERTY()
	TArray<UFireflyAttribute*> AttributeContainer;

public:
	/** 通过属性标签获取一个属性的当前值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	float GetAttributeValue(EFireflyAttributeType AttributeType) const;

	/** 通过属性标签获取一个属性的基础值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	float GetAttributeBaseValue(EFireflyAttributeType AttributeType) const;

	/** 通过类构造属性并添加到属性修改器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ConstructAttributeByClass(TSubclassOf<UFireflyAttribute> AttributeToConstruct, EFireflyAttributeType AttributeType);

	/** 通过属性类型构造属性并添加到属性修改器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ConstructAttributeByType(EFireflyAttributeType AttributeType);

	/** 初始化属性值 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void InitializeAttribute(EFireflyAttributeType AttributeType, float NewInitValue);

#pragma endregion


#pragma region Modifier

public:
	/** 应用一个修改器到某个属性的当前值中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyModifierToAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue);

	/** 应用一个修改器永久修改某个属性的基础值 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyModifierToAttributeBase(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue);

	/** 移除某个作用于某个属性的当前值的修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemoveModifierFromAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource);
		
#pragma endregion
};
