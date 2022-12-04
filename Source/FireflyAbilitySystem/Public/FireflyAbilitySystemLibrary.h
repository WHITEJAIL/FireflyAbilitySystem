// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireflyAbilitySystemTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FireflyAbilitySystemLibrary.generated.h"

/**
 * 技能系统的通用函数库
 */
UCLASS()
class FIREFLYABILITYSYSTEM_API UFireflyAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#pragma region DataDriven

public:
	/** 获取全局的技能数据表 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|DataDriven")
	static UDataTable* GetAbilityDataTable();

	/** 根据ID从全局技能数据表里获取一个技能的类型 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|DataDriven")
	static TSubclassOf<UFireflyAbility> GetAbilityClassFromDataTable(FName AbilityID);

	/** 获取全局的效果数据表 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|DataDriven")
	static UDataTable* GetEffectDataTable();

	/** 根据ID从全局效果数据表里获取一个效果的类型 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|DataDriven")
	static TSubclassOf<UFireflyEffect> GetEffectClassFromDataTable(FName EffectID);

#pragma endregion


#pragma region Attribute

public:
	/** 构造并返回一个属性实例的名称 */
	static FString GetAttributeTypeName(EFireflyAttributeType AttributeType);

#pragma endregion


#pragma region Ability

#pragma endregion


#pragma region Effect

#pragma endregion
};
