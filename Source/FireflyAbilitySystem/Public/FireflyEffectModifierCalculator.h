// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FireflyEffectModifierCalculator.generated.h"

class UFireflyEffect;

/** 效果修改器的数值计算器 */
UCLASS( Blueprintable )
class FIREFLYABILITYSYSTEM_API UFireflyEffectModifierCalculator : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual UWorld* GetWorld() const override;

public:
	/** 执行该计算器 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem")
	float CalculateModifierValue(UFireflyEffect* EffectInstance);

protected:
	/** 是否每次计算都获取Instigator和Target的最新数值 */
	UPROPERTY(EditDefaultsOnly)
	bool bUpdateUsingAttribute = true;
	
};
