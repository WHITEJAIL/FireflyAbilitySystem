// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "FireflyAttributeManagerComponent.generated.h"

// CurrentValue = NewestOuterOverrideMod || (((BaseValue || NewestInnerOverrideMod) + PlusMods - MinusMods) * (1 + MultiplyMods)) / (TotalDivideMod == 0.f ? 1.f : TotalDivideMod))

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

	UFireflyAttribute* GetAttributeByType(FGameplayTag AttributeType) const;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 为属性管理器添加一个属性 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void AddNewAttribute(FGameplayTag AttributeType, float InitValue);

	/** 通过属性标签获取一个属性的当前值 */
	UFUNCTION(BlueprintPure, Category=FireflyAttributeManager)
	float GetAttributeValue(FGameplayTag AttributeType) const;

	/** 通过属性标签获取一个属性的基础值 */
	UFUNCTION(BlueprintPure, Category = FireflyAttributeManager)
	float GetAttributeBaseValue(FGameplayTag AttributeType) const;

	/** 添加加法修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void ApplyPlusModifierToAttribute(FGameplayTag AttributeType, float ModValue);

	/** 移除加法修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void RemovePlusModifierFromAttribute(FGameplayTag AttributeType, float ModifierToRemove);

	/** 添加减法修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void ApplyMinusModifierToAttribute(FGameplayTag AttributeType, float ModValue);

	/** 移除减法修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void RemoveMinusModifierFromAttribute(FGameplayTag AttributeType, float ModifierToRemove);

	/** 添加乘法修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void ApplyMultiplyModifierToAttribute(FGameplayTag AttributeType, float ModValue);

	/** 移除乘法修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void RemoveMultiplyModifierFromAttribute(FGameplayTag AttributeType, float ModifierToRemove);

	/** 添加除法修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void ApplyDivideModifierToAttribute(FGameplayTag AttributeType, float ModValue);

	/** 移除除法修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void RemoveDivideModifierFromAttribute(FGameplayTag AttributeType, float ModifierToRemove);

	/** 添加内部覆盖修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void ApplyInnerOverrideModifierToAttribute(FGameplayTag AttributeType, float ModValue);

	/** 移除内部覆盖修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void RemoveInnerOverrideModifierFromAttribute(FGameplayTag AttributeType, float ModifierToRemove);

	/** 添加外部覆盖修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void ApplyOuterOverrideModifierToAttribute(FGameplayTag AttributeType, float ModValue);

	/** 移除外部覆盖修改器 */
	UFUNCTION(BlueprintCallable, Category = FireflyAttributeManager)
	void RemoveOuterOverrideModifierFromAttribute(FGameplayTag AttributeType, float ModifierToRemove);

protected:
	/** 属性容器 */
	UPROPERTY()
	TArray<UFireflyAttribute*> AttributeContainer;
};
