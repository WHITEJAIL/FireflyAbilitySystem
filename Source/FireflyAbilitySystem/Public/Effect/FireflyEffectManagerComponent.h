﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FireflyEffectManagerComponent.generated.h"

class UFireflyEffect;

/** 效果管理器组件 */
UCLASS( ClassGroup=(FireflyAbilitySystem), meta=(BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyEffectManagerComponent : public UActorComponent
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


#pragma region Application

protected:
	/** 在该管理器中获取特定类型的被应用的激活中的所有效果 */
	UFUNCTION()
	TArray<UFireflyEffect*> GetActiveEffectByClass(TSubclassOf<UFireflyEffect> EffectType) const;	

public:
	/** 为自身应用效果或应用效果的固定堆叠数 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToSelf(AActor* Instigator, TSubclassOf<UFireflyEffect> EffectType, int32 StackToApply = 1);

	/** 为目标应用效果或应用效果的固定堆叠数 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToTarget(AActor* Target, TSubclassOf<UFireflyEffect> EffectType, int32 StackToApply = 1);

	/** 移除自身特定的所有效果的固定堆叠树, StackToRemove = -1时，移除所有效果的所有堆叠 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void RemoveActiveEffectFromSelf(TSubclassOf<UFireflyEffect> EffectType, int32 StackToRemove = -1);

	/** 将某个效果从ActiveEffects中添加或删除 */
	UFUNCTION()
	void AddOrRemoveActiveEffect(UFireflyEffect* InEffect, bool bIsAdd);

protected:
	/** 所有激活中的执行策略不是Instant的效果 */
	UPROPERTY()
	TArray<UFireflyEffect*> ActiveEffects;

#pragma endregion
		
};
