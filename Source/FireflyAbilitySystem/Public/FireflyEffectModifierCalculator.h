// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FireflyEffectModifierCalculator.generated.h"

class UFireflyEffect;

/** Ч���޸�������ֵ������ */
UCLASS( Blueprintable )
class FIREFLYABILITYSYSTEM_API UFireflyEffectModifierCalculator : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** ִ�иü����� */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem")
	float CalculateModifierValue(UFireflyEffect* EffectInstance);

protected:
	/** �Ƿ�ÿ�μ��㶼��ȡInstigator��Target��������ֵ */
	UPROPERTY()
	bool bUpdateUsingAttribute = true;
	
};
