// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FireflyAttributeManagerComponent.generated.h"

// CurrentValue = NewestOuterOverrideMod || (((BaseValue || NewestInnerOverrideMod) + PlusMods - MinusMods) * (1 + MultiplyMods)) / (TotalDivideMod == 0.f ? 1.f : TotalDivideMod))

enum EFireflyAttributeType;
class UFireflyAttribute;

/** 属性管理器组件 */
UCLASS( ClassGroup=(FireflyAbility), meta=(BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyAttributeManagerComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnRegister() override;

	UFireflyAttribute* GetAttributeByType(EFireflyAttributeType AttributeType) const;

	FString GetAttributeTypeName(EFireflyAttributeType AttributeType) const;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	/** 添加加法修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyPlusModifier(EFireflyAttributeType AttributeType, float ModValue);

	/** 移除加法修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemovePlusModifier(EFireflyAttributeType AttributeType, float ModifierToRemove);

	/** 添加减法修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyMinusModifier(EFireflyAttributeType AttributeType, float ModValue);

	/** 移除减法修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemoveMinusModifier(EFireflyAttributeType AttributeType, float ModifierToRemove);

	/** 添加乘法修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyMultiplyModifier(EFireflyAttributeType AttributeType, float ModValue);

	/** 移除乘法修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemoveMultiplyModifier(EFireflyAttributeType AttributeType, float ModifierToRemove);

	/** 添加除法修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyDivideModifier(EFireflyAttributeType AttributeType, float ModValue);

	/** 移除除法修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemoveDivideModifier(EFireflyAttributeType AttributeType, float ModifierToRemove);

	/** 添加内部覆盖修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyInnerOverrideModifier(EFireflyAttributeType AttributeType, float ModValue);

	/** 移除内部覆盖修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemoveInnerOverrideModifier(EFireflyAttributeType AttributeType, float ModifierToRemove);

	/** 添加外部覆盖修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyOuterOverrideModifier(EFireflyAttributeType AttributeType, float ModValue);

	/** 移除外部覆盖修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemoveOuterOverrideModifier(EFireflyAttributeType AttributeType, float ModifierToRemove);

protected:
	/** 属性容器 */
	UPROPERTY()
	TArray<UFireflyAttribute*> AttributeContainer;
};
