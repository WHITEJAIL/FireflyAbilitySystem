// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireflyAbilitySystemTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FireflyAbilitySystemLibrary.generated.h"

/** 技能系统的通用函数库 */
UCLASS()
class FIREFLYABILITYSYSTEM_API UFireflyAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#pragma region Basic // 基础

public:
	/** 获取某个Actor的Firefly技能系统 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|DataDriven")
	static class UFireflyAbilitySystemComponent* GetFireflyAbilitySystem(AActor* Actor);

#pragma endregion


#pragma region DataDriven // 数据驱动

public:
	/** 根据ID从全局技能数据表里获取一个技能的类型 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|DataDriven")
	static TSubclassOf<UFireflyAbility> GetAbilityClassFromCache(FName AbilityID);

	/** 根据ID从全局效果数据表里获取一个效果的类型 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|DataDriven")
	static TSubclassOf<UFireflyEffect> GetEffectClassFromCache(FName EffectID);

#pragma endregion


#pragma region Attribute // 属性

public:
	/** 返回一个属性实例的名称 */
	static FString GetAttributeTypeName(EFireflyAttributeType AttributeType);

#pragma endregion


#pragma region Ability // 技能

#pragma endregion


#pragma region Effect // 效果

#pragma endregion


#pragma region MessageEvent // 消息事件

public:
	/** 向Actor发送一个技能系统的消息事件 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|NotifyEvent")
	static void SendNotifyEventToActor(AActor* TargetActor, FGameplayTag EventTag, FFireflyMessageEventData EventData);

#pragma endregion
};
